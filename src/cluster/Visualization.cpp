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

  _index2articleNode(),
  _index2categoryNode(),

  _clusters(),
  _category_tree(),

  _clusters_per_row(20),
  _cluster_size(0),

  _article_index(1),
  _wikidb("/media/HDD/wikipedia-db/pages")
{
  _category_tree = new CategoryTree();
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
  \brief   Create a NODE for the article visualizaton and push pointer to node vector
  \remarks ...
*/

ArticleNode*
Visualization::create_article_node(long index, std::string const label, Article article)
{
  ArticleNode* newNode = new ArticleNode(index, label, article);

  _nodes.push_back(newNode);

  return newNode;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Create a NODE and push pointer to node vector
  \remarks ...
*/

CategoryNode*
Visualization::create_category_node(long index, std::string const label, Category category)
{
  CategoryNode* newNode = new CategoryNode(index, label, category);

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

ArticleNode*
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
Visualization::article_map_contains_id(int id)
{
  std::map<uint32_t,ArticleNode*>::iterator it = _index2articleNode.find(id);

  if(it != _index2articleNode.end())
  {
   return true;
  }
  return false;
}

bool
Visualization::cat_map_contains_id(int id)
{
  std::map<uint32_t,CategoryNode*>::iterator it = _index2categoryNode.find(id);

  if(it != _index2categoryNode.end())
  {
   return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Clusters through data base and returns first found cluster
  \remarks
*/


void
Visualization::visit_article(Article article, Cluster* current_cluster)
{
  if(!article_map_contains_id(article.index))
  {
    _index2articleNode[article.index] = create_article_node(article.index,article.title, article);
    current_cluster->add_node(_index2articleNode[article.index]);
  }

  std::vector<SimPair> comparisons = article.getComparisons();

  for(unsigned k = 0; k < comparisons.size(); ++k)
  {
    SimPair current_sim_pair = comparisons[k];

    uint32_t index = current_sim_pair.getIndex();

    Article article2 = _wikidb.getArticle(index);

    uint32_t sim = current_sim_pair.getSim();

    double similarity = sim;
    similarity = similarity / 1000;


    if (!article_map_contains_id(index))
    {
      _index2articleNode[index] = create_article_node(index, article2.title, article2);

//      if (similarity > 0.8)
        current_cluster->add_node(_index2articleNode[index]);
    }


    Edge* new_edge = create_edge(_index2articleNode[article.index], _index2articleNode[index], similarity);

//    if (similarity > 0.8)
      current_cluster->add_edge(new_edge);

    new_edge->_color[0] = 0.0f;
    new_edge->_color[1] = 0.0f;
    new_edge->_color[2] = similarity;

    if (!_index2articleNode[index]->_visited)
    {
      _index2articleNode[index]->_visited = true;
      visit_article(article2, current_cluster);
    }
  }
}



void
Visualization::get_next_cluster()
{
  Cluster* current_cluster = new Cluster();

  Article article = _wikidb.getArticle(_article_index);

  if(!article_map_contains_id(article.index))
  {
    _index2articleNode[article.index] = create_article_node(article.index,article.title, article);
    current_cluster->add_node(_index2articleNode[article.index]);
  }

  if (!_index2articleNode[article.index]->_visited)
  {
    _index2articleNode[article.index]->_visited = true;
    visit_article(article, current_cluster);
  }

  current_cluster->set_radius(2000.0);
  current_cluster->set_position(_article_index * 200.0, 0.0);
  current_cluster->make_radial_layout();

  _clusters.push_back(current_cluster);

  std::cout << "found cluster with " << current_cluster->get_node_num() << " nodes." << std::endl;

  _article_index++;
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
  \brief   Set all cluster positions according to the radius of the biggest cluster
  \remarks
*/

// Compare two clusters according to their size
bool cluster_compare_func (Cluster* i, Cluster* j) { return (i->get_node_num() > j->get_node_num()); }


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

CategoryTree*
Visualization::get_category_tree() const
{
  return _category_tree;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Build category tree for a specific article
  \remarks
*/

void
Visualization::build_category_tree(Cluster* cluster)
{
  std::cout << "Build category tree for " << cluster->get_node_num() << " nodes." << std::endl;

  // Get all parents of all articles in this cluster
  for (unsigned i_node = 0; i_node != cluster->get_node_num(); ++i_node)
  {
    Article article = cluster->get_node(i_node)->_article;

    std::vector<uint32_t> parents = article.getParents();

    for (unsigned i = 0; i != parents.size(); ++i)
    {
      Category parent = _wikidb.getCategory(parents[i]);

      if(!cat_map_contains_id(parent.index))
      {
        _index2categoryNode[parent.index] = create_category_node(parent.index,parent.title, parent);
        _category_tree->add_node(_index2categoryNode[parent.index]);
      }

      std::vector<uint32_t> parent_parents = parent.getParents();

      for (unsigned j = 0; j != parent_parents.size(); ++j)
      {
        Category parent_parent = _wikidb.getCategory(parent_parents[j]);

//        if(!cat_map_contains_id(parent_parent.index))
        if(cat_map_contains_id(parent_parent.index))
        {
//          _index2categoryNode[parent_parent.index] = create_node(parent_parent.index,parent_parent.title, article);
//          _category_tree->add_node(_index2categoryNode[parent_parent.index]);

          Edge* new_edge = create_edge(_index2categoryNode[parent_parent.index],
                                       _index2categoryNode[parent.index], 0.9);
          _category_tree->add_edge(new_edge);
        }
      }
    }
  }

  _category_tree->make_category_tree_layout();
}


} // namespace vtas
