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
typedef boost::adjacency_list<boost::setS, boost::vecS, boost::undirectedS, Image, Connection> network;
typedef network::vertex_descriptor imageVertex;
typedef network::edge_descriptor imageConnection;

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

  network controlGraph;

  foreach (QString imageSerial, imageList) {
    vertexMap.insert(imageSerial, boost::add_vertex(controlGraph));
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
  std::vector<int> component(boost::num_vertices(controlGraph));
  size_t numComponents = boost::connected_components(controlGraph, &component[0]);
  auto islandFinish = std::chrono::high_resolution_clock::now();

  auto loadNetTime  = std::chrono::duration_cast<std::chrono::duration<double>>(loadNetFinish - loadNetStart);
  auto createGraphTime = std::chrono::duration_cast<std::chrono::duration<double>>(createGraphFinish - createGraphStart);
  auto islandTime = std::chrono::duration_cast<std::chrono::duration<double>>(islandFinish - islandStart);

  std::cout << "Number of vertices: " << boost::num_vertices(controlGraph) << std::endl;
  std::cout << "Number of edges: " << boost::num_edges(controlGraph) << std::endl;
  std::cout << "Number of components: " << numComponents << std::endl;
  std::cout << "Control network load time: " << loadNetTime.count() << std::endl;
  std::cout << "Create graph time: " << createGraphTime.count() << std::endl;;
  std::cout << "Count islands time: " << islandTime.count() << std::endl;

}
