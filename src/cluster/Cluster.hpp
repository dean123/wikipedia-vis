#ifndef CLUSTER_HPP
#define CLUSTER_HPP

// vta
#include <cluster/Visualization.hpp>

namespace vta
{

  class Cluster
  {
    public:
      Cluster();
      ~Cluster();

      // Add node to Cluster
      void add_node(Node*);
      void add_edge(Edge*);

      // Get node with index
      Node* get_node(unsigned);
      Edge* get_edge(unsigned);

      // Get Number of nodes and edges
      unsigned get_node_num();
      unsigned get_edge_num();

      // Set cluster global position
      void set_position(double, double);

      // Get global position x and y
      double get_position_x();
      double get_position_y();

      // Set & Get radius
      void set_radius(double);
      double get_radius();

      // Layout functions
      void make_radial_layout();
      void make_ring_layout();

      // Compute sum of all node weights
      unsigned get_node_weight_sum();

      // Create a default cluster
      void create_default_cluster();

    private:
      // Nodes and edges of cluster
      std::vector<Node*> _nodes;
      std::vector<Edge*> _edges;
      // Cluster position
      double _position_x;
      double _position_y;
      // Radius
      double _radius;

      // Radial layout weight functions
      double get_max_edge_weight(Node*);
      double get_min_edge_weight(Node*);
      double get_average_edge_weight(Node*);
  };


}


#endif // CLUSTER_HPP
