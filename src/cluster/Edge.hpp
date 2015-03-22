#ifndef EDGE_HPP
#define EDGE_HPP

// vta
#include <cluster/Node.hpp>


namespace vta
{

  struct Node;

  class Edge
  {
    public:
      // Class constructor
      Edge(long, Node*, Node*, double);

      // Class destructor
      ~Edge();

      // Get Edge index
      long getIndex() const;

      // Get source and target node
      Node* getSource() const;
      Node* getTarget() const;

      // Get edge weight
      double getWeight() const;

      // Color
      float _color[3];

    private:
      long _index;
      // Nodes
      Node* _source;
      Node* _target;
      // Weight
      double _weight;
  };


} // Namespace vta

#endif // EDGE

