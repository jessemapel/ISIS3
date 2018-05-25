#include "Isis.h"

#include <iostream>
#include <chrono>

#include <QList>
#include <QHash>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

#include "ControlNet.h"
#include "ControlPoint.h"
#include "UserInterface.h"

using namespace Isis;
using namespace std;

struct Image {
  QString serial;
};

struct Connection {
  int strength = 0;
};

// typedefs to help cut down on templated type bloat
typedef boost::adjacency_list<boost::setS, boost::listS, boost::undirectedS, Image, Connection> Network;
typedef Network::vertex_descriptor ImageVertex;
typedef Network::edge_descriptor ImageConnection;
typedef std::map<ImageVertex, size_t> VertexIndexMap;
typedef Network::out_edge_iterator ConnectionIterator;

// Main program
void IsisMain() {
  UserInterface &ui = Application::GetUserInterface();

  auto loadNetStart = std::chrono::high_resolution_clock::now();
  ControlNet net(ui.GetFileName("CNET"));
  auto loadNetFinish = std::chrono::high_resolution_clock::now();

  auto createGraphStart = std::chrono::high_resolution_clock::now();
  QList<QString> imageList = net.GetCubeSerials();
  QList<ControlPoint *> pointList = net.GetPoints();
  QHash<QString, ImageVertex> vertexMap;
  VertexIndexMap indexMap;
  boost::associative_property_map<VertexIndexMap> indexMapAdaptor(indexMap);

  Network controlGraph;

  for (int i = 0; i < imageList.size(); i++) {
    ImageVertex newVertex = boost::add_vertex(controlGraph);
    vertexMap.insert(imageList[i], newVertex);
    boost::put(indexMapAdaptor, newVertex, i);
  }

  foreach (ControlPoint *point, pointList) {
    QList<QString> pointMeasures = point->getCubeSerialNumbers();
    for (int i = 0; i < pointMeasures.size()-1; i++) {
      for (int j = i+1; j < pointMeasures.size(); j++) {
        ImageConnection connection = boost::add_edge(vertexMap[pointMeasures[i]],
                                                     vertexMap[pointMeasures[j]],
                                                     controlGraph).first;
        controlGraph[connection].strength++;
      }
    }
  }
  auto createGraphFinish = std::chrono::high_resolution_clock::now();

  auto islandStart = std::chrono::high_resolution_clock::now();
  VertexIndexMap componentMap;
  boost::associative_property_map<VertexIndexMap> componentAdaptor(componentMap);
  size_t numComponents = boost::connected_components(controlGraph, componentAdaptor,
                                                     boost::vertex_index_map(indexMapAdaptor));
  auto islandFinish = std::chrono::high_resolution_clock::now();

  QString testSerial = ui.GetString("serial");

  std::vector<std::pair<ImageVertex, int>> adjacent_edges;
  std::pair<ConnectionIterator, ConnectionIterator> adjacentIterators;
  adjacentIterators = boost::out_edges(vertexMap[testSerial], controlGraph);
  ConnectionIterator edgeIt = adjacentIterators.first;
  while(edgeIt != adjacentIterators.second) {
    std::pair<ImageVertex, int> edge(boost::target(*edgeIt, controlGraph), controlGraph[*edgeIt].strength);
    adjacent_edges.push_back(edge);
    ++edgeIt;
  }

  auto removeImageStart = std::chrono::high_resolution_clock::now();
  boost::clear_vertex(vertexMap[testSerial], controlGraph);
  boost::remove_vertex(vertexMap[testSerial], controlGraph);
  vertexMap.remove(testSerial);
  auto removeImageFinish = std::chrono::high_resolution_clock::now();

  auto addImageStart = std::chrono::high_resolution_clock::now();
  ImageVertex newVertex = boost::add_vertex(controlGraph);
  vertexMap.insert(testSerial, newVertex);
  for(size_t i = 0; i < adjacent_edges.size(); i++) {
    ImageConnection connection = boost::add_edge(newVertex, adjacent_edges[i].first, controlGraph).first;
    controlGraph[connection].strength = adjacent_edges[i].second;
  }
  auto addImageFinish = std::chrono::high_resolution_clock::now();

  auto loadNetTime  = std::chrono::duration_cast<std::chrono::duration<double>>(loadNetFinish - loadNetStart);
  auto createGraphTime = std::chrono::duration_cast<std::chrono::duration<double>>(createGraphFinish - createGraphStart);
  auto islandTime = std::chrono::duration_cast<std::chrono::duration<double>>(islandFinish - islandStart);
  auto removeImageTime = std::chrono::duration_cast<std::chrono::duration<double>>(removeImageFinish - removeImageStart);
  auto addImageTime = std::chrono::duration_cast<std::chrono::duration<double>>(addImageFinish - addImageStart);

  std::cout << "Number of vertices: " << boost::num_vertices(controlGraph) << std::endl;
  std::cout << "Number of edges: " << boost::num_edges(controlGraph) << std::endl;
  std::cout << "Number of components: " << numComponents << std::endl;
  std::cout << "Control network load time: " << loadNetTime.count() << std::endl;
  std::cout << "Create graph time: " << createGraphTime.count() << std::endl;;
  std::cout << "Count islands time: " << islandTime.count() << std::endl;
  std::cout << "Removed image with " << adjacent_edges.size() << " edges" << std::endl;
  std::cout << "Remove image time: " << removeImageTime.count() << std::endl;
  std::cout << "Add image time: " << addImageTime.count() << std::endl;

}
