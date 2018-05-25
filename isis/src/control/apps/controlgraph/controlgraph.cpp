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
typedef boost::adjacency_list<boost::setS, boost::listS, boost::undirectedS, Image, Connection> network;
typedef network::vertex_descriptor imageVertex;
typedef network::edge_descriptor imageConnection;
typedef std::map<imageVertex, size_t> vertexIndexMap;
typedef network::out_edge_iterator connectionIterator;

// Main program
void IsisMain() {
  UserInterface &ui = Application::GetUserInterface();

  auto loadNetStart = std::chrono::high_resolution_clock::now();
  ControlNet net(ui.GetFileName("CNET"));
  auto loadNetFinish = std::chrono::high_resolution_clock::now();

  auto createGraphStart = std::chrono::high_resolution_clock::now();
  QList<QString> imageList = net.GetCubeSerials();
  QList<ControlPoint *> pointList = net.GetPoints();
  QHash<QString, imageVertex> vertexMap;
  vertexIndexMap indexMap;
  boost::associative_property_map<vertexIndexMap> indexMapAdaptor(indexMap);

  network controlGraph;

  for (int i = 0; i < imageList.size(); i++) {
    imageVertex newVertex = boost::add_vertex(controlGraph);
    vertexMap.insert(imageList[i], newVertex);
    boost::put(indexMapAdaptor, newVertex, i);
  }

  foreach (ControlPoint *point, pointList) {
    QList<QString> pointMeasures = point->getCubeSerialNumbers();
    for (int i = 0; i < pointMeasures.size()-1; i++) {
      for (int j = i+1; j < pointMeasures.size(); j++) {
        imageConnection connection = boost::add_edge(vertexMap[pointMeasures[i]],
                                                     vertexMap[pointMeasures[j]],
                                                     controlGraph).first;
        controlGraph[connection].strength++;
      }
    }
  }
  auto createGraphFinish = std::chrono::high_resolution_clock::now();

  auto islandStart = std::chrono::high_resolution_clock::now();
  vertexIndexMap componentMap;
  boost::associative_property_map<vertexIndexMap> componentAdaptor(componentMap);
  size_t numComponents = boost::connected_components(controlGraph, componentAdaptor,
                                                     boost::vertex_index_map(indexMapAdaptor));
  auto islandFinish = std::chrono::high_resolution_clock::now();

  QString testSerial = ui.GetString("serial");

  std::vector<std::pair<imageVertex, int>> adjacent_edges;
  std::pair<connectionIterator, connectionIterator> adjacentIterators;
  adjacentIterators = boost::out_edges(vertexMap[testSerial], controlGraph);
  connectionIterator edgeIt = adjacentIterators.first;
  while(edgeIt != adjacentIterators.second) {
    std::pair<imageVertex, int> edge(boost::target(*edgeIt, controlGraph), controlGraph[*edgeIt].strength);
    adjacent_edges.push_back(edge);
    ++edgeIt;
  }

  auto removeImageStart = std::chrono::high_resolution_clock::now();
  boost::clear_vertex(vertexMap[testSerial], controlGraph);
  boost::remove_vertex(vertexMap[testSerial], controlGraph);
  vertexMap.remove(testSerial);
  auto removeImageFinish = std::chrono::high_resolution_clock::now();

  auto addImageStart = std::chrono::high_resolution_clock::now();
  imageVertex newVertex = boost::add_vertex(controlGraph);
  vertexMap.insert(testSerial, newVertex);
  for(size_t i = 0; i < adjacent_edges.size(); i++) {
    imageConnection connection = boost::add_edge(newVertex, adjacent_edges[i].first, controlGraph).first;
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
