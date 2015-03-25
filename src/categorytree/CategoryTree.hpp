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
      void add_node(CategoryNode*);
      void add_edge(CategoryEdge*);

      // Get node with index
      CategoryNode* get_node(unsigned);
      CategoryEdge* get_edge(unsigned);

      // Get Number of nodes and edges
      unsigned get_node_num();
      unsigned get_edge_num();

      // Get all nodes and edges
      std::vector<CategoryNode*> get_all_nodes() const;
      std::vector<CategoryEdge*> get_all_edges() const;

      // Get all highlighted nodes and edges
      std::vector<CategoryNode*> get_highlighted_nodes() const;
      std::vector<CategoryEdge*> get_highlighted_edges() const;

      // Layout functions
      void make_category_tree_layout();

      // Set highlighted nodes
      void add_highlighted_node(CategoryNode*);
      void add_highlighted_edge(CategoryEdge*);

      // Clear nodes and edges
      void clear();
      void clear_highlighting();

      bool _highlight_mode;

    private:
      // nodes and edges of CategoryTree
      std::vector<CategoryNode*> _nodes;
      std::vector<CategoryEdge*> _edges;

      // Highlight these nodes
      std::vector<CategoryNode*> _highlighted_nodes;
      std::vector<CategoryEdge*> _highlighted_edges;

  };


}


#endif // CATEGORY_TREE
