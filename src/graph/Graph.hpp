#ifndef GRAPH_HPP
#define GRAPH_HPP

// vta
#include <graph/Edge.hpp>
#include <cluster/Cluster.hpp>

// cpp
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

// boost
#include <boost/lexical_cast.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>

//wikidb
#include <contract.h>
#include <record_not_found.h>
#include <SimPair.h>
#include <wikidb.h>


namespace vta
{

  class Cluster;


  class Graph
  {
    public:

      // class constructor
      Graph();

      // class destructor
      virtual ~Graph();

      // Create Nodes and Edges
      Node* create_node(long, std::string const, Article);
      Edge* create_edge(Node*, Node*, double);

      // Get Nodes and Edges
      Node* get_node_by_index(long); /// delete
      Edge* get_edge_by_index(long); /// delete

      int get_node_num() const; // replace durch unsigned counter?
      int get_edge_num() const; // replace durch unsigned counter?

      // Cluster getter
      unsigned get_cluster_num() const;
      Cluster* get_cluster_by_index(unsigned);

      // Create Graph from data base
      void create_graph_from_db(const char input_file_name[], const char offset_file_name[]); // replace durch get cluster

      void print_graph_to_console(); /// delete

      // Get cluster size
      double get_cluster_size() const; // Allgemeine Cluster size
      // Get amount of clusters per row
      unsigned get_clusters_per_row_num() const; // wichtig fuer cluster index bei mouse over

      // Index of Cluster that will be displayed in the detail view
      unsigned _detail_view_cluster_index; // wichtig fuer Cluster window

      // Cluster algorithms
      void visit_node(Node*, Cluster*); /// delete
      void search_clusters(); /// delete

      // Set all cluster position
      void set_cluster_positions();

    private:
      std::vector<Node*> _nodes; /// delete
      std::vector<Edge*> _edges; /// delete

      std::map<uint32_t,Node*> _index2node; /// delete

      // Clustering
      std::vector<Cluster*> _clusters;

      // Amount of clusters per row
      unsigned _clusters_per_row;
      double _cluster_size;

      //
      bool node_map_contains_id(int); /// delete

  };


} // Namespace vta

#endif // GRAPH
