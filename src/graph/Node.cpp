#include "Node.hpp"

#include <random>


namespace vta
{


Node::Node(long index, std::string label, Article article)
 : _visited(false),

   outgoingEdges(),
   incomingEdges(),

   _index(index),

   _x(0.0),
   _y(0.0),

   _label(label),

   _color(),

   _article(article)
 {
   _color[0] = ((double) rand() / (RAND_MAX));
   _color[1] = ((double) rand() / (RAND_MAX));
   _color[2] = ((double) rand() / (RAND_MAX));
 }


Node::Node(long index, double x, double y, std::string label)
 : _visited(false),

   outgoingEdges(),
   incomingEdges(),

   _index(index),

   _x(x),
   _y(y),

   _label(label),

   _color(),

   _article()
 {
   _color[0] = 0.0;
   _color[1] = 1.0;
   _color[2] = 0.0;
 }


 Node::~Node()
 {}


unsigned
Node::getDegree() const
{
  return outgoingEdges.size() + incomingEdges.size();
}

}
