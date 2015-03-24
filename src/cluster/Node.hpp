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


  struct Node
  {
    // Class constructor
    Node(uint32_t, std::string);

    // Class destructors
    ~Node();

    // Variables
    bool _visited;

    // Outgoing and incoming edges
    std::vector<Edge*> outgoingEdges;
    std::vector<Edge*> incomingEdges;

    // New index
    uint32_t _index;

    // Position
    double _x;
    double _y;

    // Label (Article Name)
    std::string _label;

    // Color
    float _color[3];


    // Get the amount of edges at this node
    unsigned getDegree() const;

  };


  struct ArticleNode : Node
  {
    // Class constructor
    ArticleNode(long index, std::string title, Article article)
    : Node(index, title), _article(article)
    {}

    // wikidb article
    Article _article;
  };


  struct CategoryNode : Node
  {
    CategoryNode(long index, std::string title, Category category)
    : Node(index, title), _category(category)
    {}

    // wikidb category
    Category _category;
  };


} // Namespace vta

#endif // NODE
