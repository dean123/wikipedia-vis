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
    ArticleNode(uint32_t index, std::string title, Article article)
    : Node(index, title), _article(article), _outgoingEdges(), _incomingEdges()
    {}

    unsigned getDegree() const;

    // wikidb article
    Article _article;
    // Outgoing and incoming edges
    std::vector<ArticleEdge*> _outgoingEdges;
    std::vector<ArticleEdge*> _incomingEdges;
  };


  struct CategoryNode : Node
  {
    CategoryNode(uint32_t index, std::string title, Category category)
    : Node(index, title), _category(category), _outgoingEdges(), _incomingEdges()
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
