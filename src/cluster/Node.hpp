#ifndef NODE_HPP
#define NODE_HPP

// vta
#include <cluster/Edge.hpp>

// cpp
#include <vector>
#include <string>

// wikidb
#include <wikidb.h>


namespace vta
{

  class Edge;
  class ArticleEdge;
  class CategoryEdge;
  class Cluster;


  struct Node
  {
    // Class constructor
    Node(uint32_t, std::string);

    // Class destructors
    ~Node();

    // Variables
    bool _visited;

    // New index
    uint32_t _index;

    // Position
    double _x;
    double _y;

    // Label (Article Name)
    std::string _label;

    // Color
    float _color[3];

  };


  struct ArticleNode : Node
  {
    // Class constructor
    ArticleNode(Article article)
    : Node(article.index, article.title), _article(article), _outgoingEdges(), _incomingEdges(), _related_cluster()
    {
      _related_cluster = nullptr;
    }

    unsigned getDegree() const;

    // wikidb article
    Article _article;

    // Outgoing and incoming edges
    std::vector<ArticleEdge*> _outgoingEdges;
    std::vector<ArticleEdge*> _incomingEdges;

    // Pointer to the cluster the node belongs to
    Cluster* _related_cluster;
  };


  struct CategoryNode : Node
  {
    CategoryNode(Category category)
    : Node(category.index, category.title), _category(category), _outgoingEdges(), _incomingEdges()
    {}

    unsigned getDegree() const;

    // wikidb category
    Category _category;

    // Outgoing and incoming edges
    std::vector<CategoryEdge*> _outgoingEdges;
    std::vector<CategoryEdge*> _incomingEdges;
  };


} // Namespace vta

#endif // NODE
