#include "Isis.h"
#include "IsisDebug.h"

#include <iostream>
#include <sstream>
#include <set>

#include <QList>
#include <QMap>
#include <QStack>
#include <QString>
#include <QVector>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

#include "Camera.h"
#include "CameraFactory.h"
#include "ControlMeasure.h"
#include "ControlNet.h"
#include "ControlPoint.h"
#include "ControlPointList.h"
#include "CubeManager.h"
#include "FileList.h"
#include "FileName.h"
#include "IString.h"
#include "Progress.h"
#include "ProjectionFactory.h"
#include "PvlGroup.h"
#include "PvlKeyword.h"
#include "SerialNumber.h"
#include "SerialNumberList.h"
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

  ControlNet net(ui.GetFileName("CNET"));

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

  std::vector<int> component(boost::num_vertices(controlGraph));
  size_t numComponents = boost::connected_components(controlGraph, &component[0]);

  std::cout << "Number of vertices: " << boost::num_vertices(controlGraph) << std::endl;
  std::cout << "Number of edges: " << boost::num_edges(controlGraph) << std::endl;
  std::cout << "Number of components: " << numComponents << std::endl;

}
