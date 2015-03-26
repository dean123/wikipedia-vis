#ifndef CLUSTER_HPP
#define CLUSTER_HPP

// vta
#include <cluster/Visualization.hpp>

// gloost
#include <gloost/Matrix.h>
#include <gloost/Mesh.h>

namespace vta
{

  class Cluster
  {
    public:
      Cluster();
      ~Cluster();

      // Add node to Cluster
      void add_node(ArticleNode*);
      void add_edge(ArticleEdge*);

      // Merge two clusters
      void merge_clusters(Cluster*);

      // Get node with index
      ArticleNode* get_node(unsigned);
      ArticleEdge* get_edge(unsigned);

      // Get Number of nodes and edges
      unsigned get_node_num();
      unsigned get_edge_num();

      std::vector<ArticleNode*> get_nodes() const;
      std::vector<ArticleEdge*> get_edges() const;

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

      // Check if point is within bounding box
      bool within(const gloost::Point3&);

    private:
      // Nodes and edges of cluster
      std::vector<ArticleNode*> _nodes;
      std::vector<ArticleEdge*> _edges;

      // Cluster position
      double _position_x;
      double _position_y;
      // Radius
      double _radius;

      // Bounding box
      gloost::BoundingBox _bounding_box;

      // Radial layout weight functions
      double get_max_edge_weight(ArticleNode*);
      double get_min_edge_weight(ArticleNode*);
      double get_average_edge_weight(ArticleNode*);
  };


}


#endif // CLUSTER_HPP
