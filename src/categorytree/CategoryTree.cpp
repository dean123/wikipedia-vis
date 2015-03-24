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
  double height = 4320.0;

  // Sort node vector according to their node weights
  std::sort(_nodes.begin(), _nodes.end(), nodes_compare_func);

  // Starting positions for nodes
  double node_pos_x = 100.0;
  double node_pos_y = 100.0;


  std::vector<Node*> line;
  for (unsigned i_node = 0; i_node != _nodes.size(); ++i_node)
  {
    Node* current_node = _nodes[i_node];

    if (current_node->incomingEdges.size() == 0)
    {
      line.push_back(current_node);
    }
  }

  double step_width_y = height / line.size();

  std::vector<Node*> second_line;
  for (unsigned i_node = 0; i_node != line.size(); ++i_node)
  {
    Node* current_node = line[i_node];

    current_node->_x = node_pos_x;
    current_node->_y = node_pos_y;

    node_pos_y += step_width_y;

    for (unsigned i_edge = 0; i_edge != current_node->outgoingEdges.size(); ++i_edge)
    {
      Node* out_node = current_node->outgoingEdges[i_edge]->getTarget();
      second_line.push_back(out_node);
    }
  }


  node_pos_x = width / 2;
  node_pos_y = 100.0;
  step_width_y = height / second_line.size();

  for (unsigned i_node = 0; i_node != second_line.size(); ++i_node)
  {
    Node* current_node = second_line[i_node];

    current_node->_x = node_pos_x;
    current_node->_y = node_pos_y;

    node_pos_y += step_width_y;
  }
}


void
CategoryTree::clear()
{
  _nodes.clear();
  _edges.clear();
}

}
