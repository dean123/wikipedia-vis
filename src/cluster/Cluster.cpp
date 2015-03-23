#include "Cluster.hpp"

namespace vta
{


Cluster::Cluster()
: _nodes(),_edges(),_position_x(0.0),_position_y(0.0),_radius(0.0)
{}

Cluster::~Cluster()
{
  _nodes.clear();
}


void
Cluster::add_node(Node* node)
{
  _nodes.push_back(node);
}

void
Cluster::add_edge(Edge* edge)
{
  _edges.push_back(edge);
}


Node*
Cluster::get_node(unsigned index)
{
  return _nodes[index];
}


Edge*
Cluster::get_edge(unsigned index)
{
  return _edges[index];
}


unsigned
Cluster::get_node_num()
{
  return _nodes.size();
}

unsigned
Cluster::get_edge_num()
{
  return _edges.size();
}

// Set Cluster Position
void
Cluster::set_position(double x, double y)
{
  _position_x = x;
  _position_y = y;
}

// Get Cluster Position
double
Cluster::get_position_x()
{
  return _position_x;
}


double
Cluster::get_position_y()
{
  return _position_y;
}


void
Cluster::set_radius(double radius)
{
  _radius = radius;
}


double
Cluster::get_radius()
{
  return _radius;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
// Layouts
/////////////////////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Creates a radial layout
  \remarks
*/

void
Cluster::make_radial_layout()
{
  double node_size = 100.0;
  unsigned node_num = _nodes.size();

  // Compute angle according to number of nodes
  double angle = (M_PI * 2.0)/node_num;

  for (unsigned i_node = 0; i_node != node_num; ++i_node)
  {
    Node* current_node = _nodes[i_node];

    current_node->_x = (cos(angle * i_node) * _radius) + _position_x;
    current_node->_y = (sin(angle * i_node) * _radius) + _position_y;
  }
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Creates a ring layout
  \remarks
  \remarks
*/

void
Cluster::make_ring_layout()
{
  double node_size = 100.0; // Member variable?

  unsigned number_of_rings = 5; // Member variable?

  // Radius that changes with every ring
  double local_radius = _radius;

  double radius_step = _radius / number_of_rings;

  // Fill inner and outer ring
  std::vector<Node*> inner_ring;
  std::vector<Node*> outer_rings;

  // Fill according to sum of node weights of cluster and inner ring
  unsigned cluster_node_weight_sum = get_node_weight_sum();
  unsigned inner_ring_node_weight_sum = 0;

  for (unsigned i_node = 0; i_node != _nodes.size(); ++i_node)
  {
    Node* current_node = _nodes[i_node];

    if (inner_ring_node_weight_sum >= cluster_node_weight_sum * 0.5)
      outer_rings.push_back(current_node);

    else
      inner_ring.push_back(current_node);

    inner_ring_node_weight_sum += current_node->getDegree();
  }

  // Draw inner ring
  double angle = (M_PI * 2.0)/inner_ring.size();


  local_radius = radius_step * (number_of_rings+1);

  for (unsigned i_node = 0; i_node != inner_ring.size(); ++i_node)
  {
    Node* current_node = inner_ring[i_node];

    current_node->_x = (cos(angle * i_node) * local_radius) + _position_x;
    current_node->_y = (sin(angle * i_node) * local_radius) + _position_y;
  }

  // Draw outer rings
  double min_edge_weigth = 0.9; // Minimum edge weight of subset
  double max_edge_weight = 1.0; // Maximum edge weight of subset

  double step_width = (max_edge_weight - min_edge_weigth)/(number_of_rings-1);

  for (unsigned i_ring = 0; i_ring != number_of_rings-1; ++i_ring)
  {
    double ring_edge_weight_min = min_edge_weigth + (i_ring * step_width);
    double ring_edge_weight_max = min_edge_weigth + ((i_ring+1) * step_width);

    std::vector<Node*> current_ring;

    for (unsigned i_node = 0; i_node != outer_rings.size(); ++i_node)
    {
      Node* current_node = outer_rings[i_node];

      double max_edge_weight = get_max_edge_weight(current_node);

      if (max_edge_weight >= ring_edge_weight_min && max_edge_weight < ring_edge_weight_max)
      {
        current_ring.push_back(current_node);
      }
    }

    local_radius = radius_step * (i_ring+2);

    angle = (M_PI * 2.0)/current_ring.size();

    for (unsigned i_node = 0; i_node != current_ring.size(); ++i_node)
    {
      Node* current_node = current_ring[i_node];

      current_node->_x = (cos(angle * i_node) * local_radius) + _position_x;
      current_node->_y = (sin(angle * i_node) * local_radius) + _position_y;
    }
  }
}



unsigned
Cluster::get_node_weight_sum()
{
  unsigned node_weight_sum = 0;

  for (unsigned i = 0; i != _nodes.size(); ++i)
    node_weight_sum += _nodes[i]->getDegree();

  return node_weight_sum;
}

void
Cluster::create_default_cluster()
{
//  // Create some nodes
//  for (unsigned i = 0; i != 10; ++i)
//  {
//    Node* new_node = new Node(i, "unnamed");
//
//    // Set Position
//    new_node->_x = i;
//    new_node->_y = i;
//
//    // Push to cluster
//    _nodes.push_back(new_node);
//  }
//
//  // Create some edges
//  for (unsigned i = 0; i != 5; ++i)
//  {
//    // Create Edge between:
//    Node* source = _nodes[i];
//    Node* target = _nodes[i+1];
//
//    Edge* new_edge = new Edge(i, source, target, 0.5);
//
//    // Push Edge to outgoing and incoming edges of its nodes
//    source->outgoingEdges.push_back(new_edge);
//    target->incomingEdges.push_back(new_edge);
//  }
}




/////////////////////////////////////////////////////////////////////////////////////////////////
// Allgemeine Layout Funktionen fuer edge-weights
/////////////////////////////////////////////////////////////////////////////////////////////////

double
Cluster::get_max_edge_weight(Node* v1)
{
  double max_edge_weight = 0.0;

  for(unsigned i_edge = 0; i_edge != v1->outgoingEdges.size(); ++i_edge)
    max_edge_weight = std::max(v1->outgoingEdges[i_edge]->getWeight(), max_edge_weight);

  for(unsigned i_edge = 0; i_edge != v1->incomingEdges.size(); ++i_edge)
    max_edge_weight = std::max(v1->incomingEdges[i_edge]->getWeight(), max_edge_weight);

  return max_edge_weight;
}

double
Cluster::get_min_edge_weight(Node* v1)
{
  double min_edge_weight = 1.1;

  for(unsigned i_edge = 0; i_edge != v1->outgoingEdges.size(); ++i_edge)
    min_edge_weight = std::min(v1->outgoingEdges[i_edge]->getWeight(), min_edge_weight);

  for(unsigned i_edge = 0; i_edge != v1->incomingEdges.size(); ++i_edge)
    min_edge_weight = std::min(v1->incomingEdges[i_edge]->getWeight(), min_edge_weight);

  return min_edge_weight;
}

double
Cluster::get_average_edge_weight(Node* v1)
{
  double sum_edge_weight = 0.0;
  unsigned num_of_edges = 0;

  for(unsigned i_edge = 0; i_edge != v1->outgoingEdges.size(); ++i_edge)
  {
    sum_edge_weight += v1->outgoingEdges[i_edge]->getWeight();
    num_of_edges++;
  }

  for(unsigned i_edge = 0; i_edge != v1->incomingEdges.size(); ++i_edge)
  {
    sum_edge_weight += v1->incomingEdges[i_edge]->getWeight();
    num_of_edges++;
  }

  return sum_edge_weight/num_of_edges;
}


}
