#include "Visualization.hpp"




namespace vta
{

////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Class constructor
  \remarks ...
*/

Visualization::Visualization()
:
  _detail_view_cluster_index(0),

  _nodes(),
  _edges(),

  _index2node(),

  _clusters(),
  _category_tree(),

  _clusters_per_row(20),
  _cluster_size(0)
{
  _category_tree = new Cluster();
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Class destructor
  \remarks ...
*/

/*virtual*/
Visualization::~Visualization()
{}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Create a NODE and push pointer to node vector
  \remarks ...
*/

Node*
Visualization::create_node(long index, std::string const label, Article article)
{
  Node* newNode = new Node(index, label, article);

  _nodes.push_back(newNode);

  return newNode;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Create a EDGE and push pointer to edge vector
  \remarks ...
*/

Edge*
Visualization::create_edge(Node* source, Node* target, double weight)
{
  Edge* newEdge = new Edge(_edges.size(), source, target, weight);

  _edges.push_back(newEdge);

  source->outgoingEdges.push_back(newEdge);
  target->incomingEdges.push_back(newEdge);

  return newEdge;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Get pointer to EDGE by vector index
  \remarks ...
*/

Edge*
Visualization::get_edge_by_index(long index)
{
  return _edges[index];
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Get pointer to NODE by vector index
  \remarks ...
*/

Node*
Visualization::get_node_by_index(long index)
{
  return _nodes[index];
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Get number of NODES
  \remarks ...
*/

int
Visualization::get_node_num() const
{
  return _nodes.size();
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Get number of EDGES
  \remarks ...
*/

int
Visualization::get_edge_num() const
{
  return _edges.size();
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Get number of CLUSTERS
  \remarks ...
*/

unsigned
Visualization::get_cluster_num() const
{
  return _clusters.size();
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Get CLUSTER by vector index
  \remarks ...
*/

Cluster*
Visualization::get_cluster_by_index(unsigned index)
{
  return _clusters[index];
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Check if id-node map contains id
  \remarks id that replaced rev-id
*/

bool
Visualization::node_map_contains_id(int id)
{
  std::map<uint32_t,Node*>::iterator it = _index2node.find(id);

  if(it != _index2node.end())
  {
   return true;
  }
  return false;
}


// Compare two clusters according to their size
bool cluster_compare_func (Cluster* i, Cluster* j) { return (i->get_node_num() > j->get_node_num()); }


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Reads complete binary file and creates nodes and edges
  \remarks node --> article
  \remarks edge --> comparison
*/

void
Visualization::create_graph_from_db(const char g_bin_data_filename[], const char offset_file_name[])
{
    std::cout << "in create_Visualization_from_db`" << std::endl;
    WikiDB wikidb("/media/HDD/testdb/pages");
    std::vector<uint32_t> articleVec = wikidb.getArticles();
    std::vector<uint32_t> categoryVec = wikidb.getCategories();

    std::cout << "article vec size " << articleVec.size() << std::endl;
    std::cout << "category vec size " << categoryVec.size() << std::endl;

    Article article = wikidb.getArticle(1);

    for (auto k : categoryVec)
    {
      Category category = wikidb.getCategory(k);

      if(!node_map_contains_id(k))
      {
        _index2node[k] = create_node(k,category.title, article);
        _category_tree->add_node(_index2node[k]);
      }

      std::vector<uint32_t> parents = category.getParents();

      for(unsigned j = 0; j < parents.size(); ++j)
      {
        uint32_t index = parents[j];

        Category parent = wikidb.getCategory(index);
        if (!node_map_contains_id(index))
        {
          _index2node[index] = create_node(index, parent.title, article);
          _category_tree->add_node(_index2node[index]);
        }


        _category_tree->add_edge(create_edge(_index2node[k], _index2node[index], 0.9));
      }
    }

    _index2node.clear();

    // All cluster have the size and radius of the biggest cluster
    double width_height = 5000.0 * sqrt(_category_tree->get_node_num());
    double radius = 5000.0 * _category_tree->get_node_num() + width_height;

    _category_tree->set_position(radius/2 + radius/4, radius/2 + radius/4);

    _category_tree->set_radius(radius);
//    _category_tree->make_radial_layout();
    _category_tree->make_category_tree_layout();

    std::cout << "Created category tree with " << _category_tree->get_node_num() << " nodes." << std::endl;



//    for (auto i : articleVec) {
//      Article article = wikidb.getArticle(i);
//      std::vector<SimPair> compVector = article.getComparisons();
//
//      if(!node_map_contains_id(i))
//        _index2node[article.index] = create_node(article.index,article.title, article);
//
//
//      for(unsigned k = 0; k < compVector.size(); ++k)
//      {
//        if (k == 100)
//          break;
//
//        SimPair current_sim_pair = compVector[k];
//
//        uint32_t index = current_sim_pair.getIndex();
//
//        Article article2 = wikidb.getArticle(index);
//
//        if (!node_map_contains_id(index))
//          _index2node[index] = create_node(index, article2.title, article2);
//
//        uint32_t sim = current_sim_pair.getSim();
//
//        double similarity = sim;
//        similarity = similarity / 1000;
//
//        Edge* new_edge = create_edge(_index2node[article.index], _index2node[index], similarity);
//
//        new_edge->_color[0] = 0.0f;
//        new_edge->_color[1] = 0.0f;
//        new_edge->_color[2] = similarity;
//      }
//    }
//
//      search_clusters();
//      std::sort (_clusters.begin(), _clusters.end(), cluster_compare_func);
//
//      set_cluster_positions();
//
//      _index2node.clear();

      std::cout << "created from db" << std::endl;
  }


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Returns the size of every cluster
  \remarks All clusters currently have the same size
*/

double
Visualization::get_cluster_size() const
{
  return _cluster_size;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Returns the amount of clusters per row
  \remarks Default value is 20
*/

unsigned
Visualization::get_clusters_per_row_num() const
{
  return _clusters_per_row;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Visit the node and recursive visit all adjacent nodes that have not been visited yet
  \remarks Auskommentiert ist die Möglichkeit ein Cluster nur über einem bestimmten similarity threshold zu bilden
*/

void
Visualization::visit_node(Node* v1, Cluster* current_cluster)
{
  if(!v1->_visited)
  {
    v1->_visited = true;
    current_cluster->add_node(v1);

    for (unsigned i = 0; i != v1->incomingEdges.size(); i++)
    {
      Edge* current_edge = v1->incomingEdges[i];

      current_cluster->add_edge(current_edge);

      if (current_edge->getWeight() >= 0.63)
        visit_node(current_edge->getSource(), current_cluster);
    }

    for (unsigned i = 0; i != v1->outgoingEdges.size(); i++)
    {
      Edge* current_edge = v1->outgoingEdges[i];

      if (current_edge->getWeight() >= 0.63)
        visit_node(current_edge->getTarget(), current_cluster);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Search for clusers and store them in the vector _clusters
  \remarks
*/

void
Visualization::search_clusters()
{
  std::cout << "Searching for clusters..." << std::endl;

  for (unsigned i = 0; i != _nodes.size(); i++)
  {
    Node* source = _nodes[i];

    Cluster* current_cluster = new Cluster();

    visit_node(source, current_cluster);

    if(current_cluster->get_node_num() != 0)
      _clusters.push_back(current_cluster);

  }

  std::cout << "Finished searching for clusters" << std::endl;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Set all cluster positions according to the radius of the biggest cluster
  \remarks
*/

void
Visualization::set_cluster_positions()
{
  std::cout << _clusters.size() << std::endl;

  std::sort (_clusters.begin(), _clusters.end(), cluster_compare_func);

  double node_size = 100.0;

  // All cluster have the size and radius of the biggest cluster
  double width_height = node_size * sqrt(_clusters[0]->get_node_num());
  double radius = node_size * _clusters[0]->get_node_num() + width_height;

  // Set Cluster size
  _cluster_size = (radius * 2) + radius/2;

  // Das erste Cluster soll von 0 bis "_cluster_size" reichen
  double position_x = radius;
  double position_y = radius;

  for (unsigned i_cluster = 0; i_cluster != _clusters.size(); ++i_cluster)
  {
    Cluster* current_cluster = _clusters[i_cluster];

    // Set position and radius
    current_cluster->set_position(position_x, position_y);
    current_cluster->set_radius(radius);

    // Set position for the next cluster
    position_x += (radius * 2) + radius/2;
    if (i_cluster % _clusters_per_row == _clusters_per_row-1) // _cluster_per_row inital 20
    {
      position_x = radius;
      position_y += (radius * 2) + radius/2;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Return category tree
  \remarks
*/

Cluster*
Visualization::get_category_tree() const
{
  return _category_tree;
}


} // namespace vtas
