#ifndef EDGE_HPP
#define EDGE_HPP

// vta
#include <cluster/Node.hpp>


namespace vta
{

  struct Node;
  struct ArticleNode;
  struct CategoryNode;

  class Edge
  {
    public:
      // Class constructor
      Edge(double);

      // Class destructor
      ~Edge();

      // Get edge weight
      double getWeight() const;

    private:
      // Weight
      double _weight;
  };


  class ArticleEdge : Edge
  {
    public:
      // Class constructor
      ArticleEdge(ArticleNode*, ArticleNode*, double);

      // Get edge weight
      virtual double getWeight() const;

      // Get source and target node
      ArticleNode* getSource() const;
      ArticleNode* getTarget() const;

      // Color
      float _color[3];

    private:
      // Nodes
      ArticleNode* _source;
      ArticleNode* _target;

  };


  class CategoryEdge : Edge
  {
    public:
      // Class constructor
      CategoryEdge(CategoryNode*, CategoryNode*, double);

      // Get edge weight
      virtual double getWeight() const;

      // Get source and target node
      CategoryNode* getSource() const;
      CategoryNode* getTarget() const;

      // Color
      float _color[3];

    private:
      // Nodes
      CategoryNode* _source;
      CategoryNode* _target;

  };


} // Namespace vta

#endif // EDGE

