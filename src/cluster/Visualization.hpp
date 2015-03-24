#ifndef VISUALIZATION_HPP
#define VISUALIZATION_HPP

// vta
#include <cluster/Edge.hpp>
#include <cluster/Cluster.hpp>
#include <categorytree/CategoryTree.hpp>

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
  class CategoryTree;


  class Visualization
  {
    public:

      // class constructor
      Visualization();

      // class destructor
      virtual ~Visualization();

      // Create Nodes and Edges
      ArticleNode* create_article_node (long, std::string const, Article);
      CategoryNode* create_category_node(long, std::string const, Category);
      Edge* create_edge(Node*, Node*, double);

      // Get Nodes and Edges
      ArticleNode* get_node_by_index(long); /// delete
      Edge* get_edge_by_index(long); /// delete

      int get_node_num() const; // replace durch unsigned counter?
      int get_edge_num() const; // replace durch unsigned counter?

      // Cluster getter
      unsigned get_cluster_num() const;
      Cluster* get_cluster_by_index(unsigned);

      // Create Visualization from data base
      void create_graph_from_db(const char input_file_name[], const char offset_file_name[]); // replace durch get cluster
      void visit_article(Article, Cluster*);
      void get_next_cluster();

      // Get cluster size
      double get_cluster_size() const; // Allgemeine Cluster size
      // Get amount of clusters per row
      unsigned get_clusters_per_row_num() const; // wichtig fuer cluster index bei mouse over

      // Index of Cluster that will be displayed in the detail view
      unsigned _detail_view_cluster_index; // wichtig fuer Cluster window

      // Set all cluster position
      void set_cluster_positions();

      // Return the category tree
      CategoryTree* get_category_tree() const;

      // Build category tree for a specific article
      void build_category_tree(Cluster*);

    private:
      std::vector<ArticleNode*> _nodes; /// delete
      std::vector<Edge*> _edges; /// delete

      std::map<uint32_t,ArticleNode*> _index2articleNode;
      std::map<uint32_t,CategoryNode*> _index2categoryNode;

      // Clustering
      std::vector<Cluster*> _clusters;

      // Category tree
      CategoryTree* _category_tree;

      // Amount of clusters per row
      unsigned _clusters_per_row;
      double _cluster_size;

      // wikidb
      uint32_t _article_index;
      WikiDB _wikidb;

      // to check if nodes allready exist
      bool article_map_contains_id(int);
      bool cat_map_contains_id(int);


  };


} // Namespace vta

#endif // VISUALIZATION_HPP
