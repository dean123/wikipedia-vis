#ifndef CATEGORY_TREE
#define CATEGORY_TREE

// vta
#include <cluster/Visualization.hpp>

namespace vta
{

  class CategoryTree
  {
    public:
      CategoryTree();
      ~CategoryTree();

      // Add node to CategoryTree
      void add_node(Node*);
      void add_edge(Edge*);

      // Get node with index
      Node* get_node(unsigned);
      Edge* get_edge(unsigned);

      // Get Number of nodes and edges
      unsigned get_node_num();
      unsigned get_edge_num();

      // Layout functions
      void make_category_tree_layout();

      // Clear nodes and edges
      void clear();

    private:
      // Nodes and edges of CategoryTree
      std::vector<Node*> _nodes;
      std::vector<Edge*> _edges;

      // Draw these nodes
      std::vector<Node*> _drawable_nodes;

  };


}


#endif // CATEGORY_TREE
