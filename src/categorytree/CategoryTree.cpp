#include "CategoryTree.hpp"

namespace vta
{

CategoryTree::CategoryTree()
: _highlight_mode(false), _nodes(),_edges(),_highlighted_nodes()
{}


CategoryTree::~CategoryTree()
{}


void
CategoryTree::add_node(CategoryNode* node)
{
  _nodes.push_back(node);
}


void
CategoryTree::add_edge(CategoryEdge* edge)
{
  _edges.push_back(edge);
}


CategoryNode*
CategoryTree::get_node(unsigned index)
{
  return _nodes[index];
}


CategoryEdge*
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


std::vector<CategoryNode*>
CategoryTree::get_all_nodes() const
{
  return _nodes;
}


std::vector<CategoryEdge*>
CategoryTree::get_all_edges() const
{
  return _edges;
}


std::vector<CategoryNode*>
CategoryTree::get_highlighted_nodes() const
{
  return _highlighted_nodes;
}


std::vector<CategoryEdge*>
CategoryTree::get_highlighted_edges() const
{
  return _highlighted_edges;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Creates the layout for the category tree
  \remarks
  \remarks
*/


// Compare two nodes according to their weigth
bool nodes_compare_func (CategoryNode* i, CategoryNode* j) { return (i->getDegree() > j->getDegree()); }


std::vector<CategoryNode*>
CategoryTree::create_vertical_node_line(std::vector<CategoryNode*>& nodes, double node_pos_x)
{
  double node_size = 100.0;

  double width = 1920.0;
  double height = 4320.0;

  // Sort node vector according to their node weights
  std::sort(nodes.begin(), nodes.end(), nodes_compare_func);

  // Starting positions for nodes
  double node_pos_y = 100.0;

  double step_width_y = height / nodes.size();

  std::vector<CategoryNode*> children_line;
  for (unsigned i_node = 0; i_node != nodes.size(); ++i_node)
  {
    CategoryNode* current_node = nodes[i_node];

    current_node->_x = node_pos_x;
    current_node->_y = node_pos_y;

    current_node->_visited = true;

    node_pos_y += step_width_y;

    for (unsigned i_edge = 0; i_edge != current_node->_outgoingEdges.size(); ++i_edge)
    {
      CategoryNode* out_node = current_node->_outgoingEdges[i_edge]->getTarget();
      children_line.push_back(out_node);
    }
  }

  return children_line;
}



void
CategoryTree::make_category_tree_layout()
{
  std::cout << "make category tree layout" << std::endl;

  std::vector<CategoryNode*> line;
  for (unsigned i_node = 0; i_node != _nodes.size(); ++i_node)
  {
    CategoryNode* current_node = _nodes[i_node];

    if (current_node->_incomingEdges.size() == 0)
    {
      line.push_back(current_node);
    }
  }

  // position first line and create second line
  std::vector<CategoryNode*> second_line = create_vertical_node_line(line, 100.0);

  // position second line
  std::vector<CategoryNode*> third_line = create_vertical_node_line(second_line, 1920 / 2);

  std::cout << "Third line size: " << third_line.size() << std::endl;


  // alle ubrig gebliebeneden
  std::vector<CategoryNode*> not_positioned;
  for (unsigned i_node = 0; i_node != _nodes.size(); ++i_node)
  {
    CategoryNode* current_node = _nodes[i_node];

    if(!current_node->_visited)
    {
      not_positioned.push_back(current_node);
    }
  }

  std::vector<CategoryNode*> no_pos = create_vertical_node_line(not_positioned, 1700);

  std::cout << "no pos size: " << no_pos.size() << std::endl;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Set the highlighted nodes and edges
  \remarks
  \remarks
*/

void
CategoryTree::add_highlighted_node(CategoryNode* cat_node)
{
  _highlighted_nodes.push_back(cat_node);

  std::cout << "added node: " << cat_node->_category.title << std::endl;
}


void
CategoryTree::add_highlighted_edge(CategoryEdge* cat_edge)
{
  _highlighted_edges.push_back(cat_edge);
}


void
CategoryTree::clear()
{
  _nodes.clear();
  _edges.clear();
}

void
CategoryTree::clear_highlighting()
{
  _highlighted_nodes.clear();
  _highlighted_edges.clear();
}

}
