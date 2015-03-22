#include "Edge.hpp"

#include <random>

namespace vta
{

Edge::Edge(long index, Node* source, Node* target, double weight)
 : _color(), _index(index), _source(source), _target(target), _weight(weight)
 {
   _color[0] = ((double) rand() / (RAND_MAX));
   _color[1] = ((double) rand() / (RAND_MAX));
   _color[2] = ((double) rand() / (RAND_MAX));
 }


Edge::~Edge()
{}


long
Edge::getIndex() const
{
  return _index;
}


Node*
Edge::getSource() const
{
  return _source;
}

Node*
Edge::getTarget() const
{
  return _target;
}

double
Edge::getWeight() const
{
  return _weight;
}

}
