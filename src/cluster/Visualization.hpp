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

      // Create Nodes and Edges for articles
      ArticleNode* create_article_node (Article);
      ArticleEdge* create_article_edge(ArticleNode*, ArticleNode*, double);
      // Create nodes and edges for categorys
      CategoryNode* create_category_node(Category);
      CategoryEdge* create_category_edge(CategoryNode*, CategoryNode*, double);

      // Cluster getter
      unsigned get_cluster_num() const;
      Cluster* get_cluster_by_index(unsigned);

      // Load edges for new similarity values
      void add_edges_for_sim(double, double);

      // Create Visualization from data base
      void visit_article(Article, Cluster*);
      void get_next_cluster();

      // Set cluster index for nodes
      void set_node_cluster_index();

      // Get cluster size
      double get_cluster_size() const; // Allgemeine Cluster size
      // Get amount of clusters per row
      unsigned get_clusters_per_row_num() const; // wichtig fuer cluster index bei mouse over

      // Index of Cluster that will be displayed in the detail view
      unsigned _detail_view_cluster_index; // wichtig fuer Cluster window

      // Set all cluster position
      void set_cluster_positions();
      // Set cluster positions for a global radial layout
      void make_global_radial_layout();

      // Return the category tree
      CategoryTree* get_category_tree() const;

      // Build category tree for a specific article
      void build_category_tree(Cluster*);
      void set_highlighted_categories(ArticleNode*);
      void reset_highlighted_categories();

      // Create new cluster with all edges and nodes of the current visualization
      void re_cluster_current_vis();

      // Delete cluster from vector
      void clear_cluster(unsigned);

    private:

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
