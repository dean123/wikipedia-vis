#include "Edge.hpp"

#include <random>

namespace vta
{

Edge::Edge(double weight)
 : _weight(weight)
 {}


Edge::~Edge()
{}


double
Edge::getWeight() const
{
  return _weight;
}


// Article Edge

ArticleEdge::ArticleEdge(ArticleNode* source, ArticleNode* target, double weight)
  : Edge(weight), _source(source), _target(target), _color()
  {
   _color[0] = ((double) rand() / (RAND_MAX));
   _color[1] = ((double) rand() / (RAND_MAX));
   _color[2] = ((double) rand() / (RAND_MAX));
  }

// virtual
double
ArticleEdge::getWeight() const
{
  return Edge::getWeight();
}

ArticleNode*
ArticleEdge::getSource() const
{
  return _source;
}

ArticleNode*
ArticleEdge::getTarget() const
{
  return _target;
}


// Category Edge

CategoryEdge::CategoryEdge(CategoryNode* source, CategoryNode* target, double weight)
  : Edge(weight), _source(source), _target(target), _color()
  {
   _color[0] = ((double) rand() / (RAND_MAX));
   _color[1] = ((double) rand() / (RAND_MAX));
   _color[2] = ((double) rand() / (RAND_MAX));
  }


// virtual
double
CategoryEdge::getWeight() const
{
  return Edge::getWeight();
}


CategoryNode*
CategoryEdge::getSource() const
{
  return _source;
}

CategoryNode*
CategoryEdge::getTarget() const
{
  return _target;
}

}
