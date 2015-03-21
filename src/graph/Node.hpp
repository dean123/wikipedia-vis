#ifndef NODE_HPP
#define NODE_HPP

// vta
#include <graph/Edge.hpp>

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
    // Class constructors
    Node(long, std::string, Article);
    Node(long, double, double, std::string);

    // Class destructors
    ~Node();

    // Variables
    bool _visited;

    // Position
    double _x;
    double _y;

    // Color
    float _color[3];

    // wikidb article
    Article _article;


    // Article Member

    // Get the amount of edges at this node
    unsigned getDegree() const;

    // Outgoing and incoming edges
    std::vector<Edge*> outgoingEdges;   // comparisons
    std::vector<Edge*> incomingEdges;

    // New index
    long _index;    //TODO uint32_t !!!

    // Label (Article Name)
    std::string _label;

  };


} // Namespace vta

#endif // NODE
