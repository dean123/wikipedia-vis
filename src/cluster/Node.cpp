#include "Node.hpp"

#include <random>


namespace vta
{


Node::Node(uint32_t index, std::string label)
 : _visited(false),

   outgoingEdges(),
   incomingEdges(),

   _index(index),

   _x(0.0),
   _y(0.0),

   _label(label),

   _color()
 {
   _color[0] = ((double) rand() / (RAND_MAX));
   _color[1] = ((double) rand() / (RAND_MAX));
   _color[2] = ((double) rand() / (RAND_MAX));
 }


 Node::~Node()
 {}


unsigned
Node::getDegree() const
{
  return outgoingEdges.size() + incomingEdges.size();
}

}
