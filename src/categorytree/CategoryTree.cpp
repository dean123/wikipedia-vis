#include "CategoryTree.hpp"

namespace vta
{

CategoryTree::CategoryTree()
: _nodes(),_edges(),_drawable_nodes()
{}


CategoryTree::~CategoryTree()
{}


void
CategoryTree::add_node(Node* node)
{
  _nodes.push_back(node);
}


void
CategoryTree::add_edge(Edge* edge)
{
  _edges.push_back(edge);
}


Node*
CategoryTree::get_node(unsigned index)
{
  return _nodes[index];
}


Edge*
CategoryTree::get_edge(unsigned index)
{
  return _edges[index];
}


unsigned
CategoryTree::get_node_num()
{
  return _nodes.size();
}

unsigned
CategoryTree::get_edge_num()
{
  return _edges.size();
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Creates the layout for the category tree
  \remarks
  \remarks
*/

// Compare two nodes according to their weigth
bool nodes_compare_func (Node* i, Node* j) { return (i->getDegree() > j->getDegree()); }

void
CategoryTree::make_category_tree_layout()
{
  std::cout << "make category tree layout" << std::endl;

  double node_size = 100.0;

  double width = 1920.0;
  double height = 1080.0;

  // Sort node vector according to their node weights
  std::sort(_nodes.begin(), _nodes.end(), nodes_compare_func);

  // Starting positions for nodes
  double node_pos_x = 10.0;
  double node_pos_y = 10.0;

  for (unsigned i_node = 0; i_node != _nodes.size(); ++i_node)
  {
    Node* current_node = _nodes[i_node];

    if (current_node->incomingEdges.size() == 0)
    {
      current_node->_x = node_pos_x;
      current_node->_y = node_pos_y;
    }

    else
    {
      current_node->_x = node_pos_x;
      current_node->_y = height/2;
    }


    node_pos_x += 10.0;

//    Node* current_node = _nodes[i_node];
//
//    current_node->_x = node_pos_x;
//    current_node->_y = node_pos_y;
//
//    unsigned node_num = current_node->outgoingEdges.size();
//
//    double node_size = 100.0;
//    double width_height = node_size * sqrt(node_num);
//    double radius = node_size * node_num + width_height;
//
//    double angle = (M_PI * 2.0)/node_num;
//
//    for (unsigned i = 0; i != node_num; ++i)
//    {
//      Edge* in_edge = current_node->outgoingEdges[i];
//      Node* target = in_edge->getTarget();
//
//      target->_x = (cos(angle * i) * radius) + node_pos_x;
//      target->_y = (sin(angle * i) * radius) + node_pos_y;
//    }
//
//    node_pos_x += radius * 2;
//    if (node_pos_x > 10000000)
//    {
//      node_pos_x = 0.0;
//      node_pos_y += radius * 2;
//    }
  }
}


void
CategoryTree::clear()
{
  _nodes.clear();
  _edges.clear();
}

}
