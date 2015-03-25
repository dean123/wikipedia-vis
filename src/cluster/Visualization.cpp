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
  _wikidb("/media/HDD/wikipedia-db/pages"),

  _max_x(0.0),
  _max_y(0.0)
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
  \brief   Create a Article Edge
  \remarks ...
*/

ArticleEdge*
Visualization::create_article_edge(ArticleNode* source, ArticleNode* target, double weight)
{
  ArticleEdge* newEdge = new ArticleEdge(source, target, weight);

  _edges.push_back(newEdge);

  source->_outgoingEdges.push_back(newEdge);
  target->_incomingEdges.push_back(newEdge);

  return newEdge;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Create a EDGE and push pointer to edge vector
  \remarks ...
*/

CategoryEdge*
Visualization::create_category_edge(CategoryNode* source, CategoryNode* target, double weight)
{
  CategoryEdge* newEdge = new CategoryEdge(source, target, weight);

  source->_outgoingEdges.push_back(newEdge);
  target->_incomingEdges.push_back(newEdge);

  return newEdge;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Get node vector
  \remarks ...
*/

std::vector<ArticleNode*>
Visualization::get_nodes()
{
  return _nodes;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Get pointer to NODE by vector index
  \remarks ...
*/

std::vector<ArticleEdge*>
Visualization::get_edges()
{
  return _edges;
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
  \brief   Get Visualization boundaries
  \remarks ...
*/

double
Visualization::get_max_x() const
{
  return _max_x;
}


double
Visualization::get_max_y() const
{
  return _max_y;
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

      if (similarity > 0.8)
        current_cluster->add_node(_index2articleNode[index]);
    }

    ArticleEdge* new_edge = create_article_edge(_index2articleNode[article.index], _index2articleNode[index], similarity);

    new_edge->_color[0] = 0.0f;
    new_edge->_color[1] = 0.0f;
    new_edge->_color[2] = similarity;

    if (similarity > 0.8)
    {
      current_cluster->add_edge(new_edge);

      if (!_index2articleNode[index]->_visited)
      {
        _index2articleNode[index]->_visited = true;

        visit_article(article2, current_cluster);
      }
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

    _max_x = std::max(_max_x, position_x);

    if (i_cluster % _clusters_per_row == _clusters_per_row-1) // _cluster_per_row inital 20
    {
      position_x = radius;
      position_y += (radius * 2) + radius/2;

      _max_y = std::max(_max_y, position_y);
    }
  }

  _max_x += radius;
  _max_y += radius;
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

  std::vector<uint32_t> all_parents;
  std::vector<uint32_t> similar_parents;

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

          CategoryEdge* new_edge = create_category_edge(_index2categoryNode[parent_parent.index],
                                                        _index2categoryNode[parent.index], 0.9);
          _category_tree->add_edge(new_edge);
        }
        else
        {
          bool allread_in_vec = false;
          for (unsigned k = 0; k != all_parents.size(); ++k)
          {
            if (all_parents[k] == parent_parents[j])
            {
              // Dieser Parent hat mehrere Children
              std::cout << parent_parent.title << std::endl;

              allread_in_vec = true;

              if(!cat_map_contains_id(parent_parent.index))
              {
                _index2categoryNode[parent_parent.index] = create_category_node(all_parents[k], parent_parent.title, parent_parent);
                similar_parents.push_back(parent_parent.index);
              }

              break;
            }
          }

          if (!allread_in_vec)
          {
            all_parents.push_back(parent_parent.index);
          }
        }
      }
    }
  }

//
//  for (unsigned i = 0; i != similar_parents.size(); ++i)
//  {
//    CategoryNode* cat_node = _index2categoryNode[similar_parents[i]];
//
//  std::map<uint32_t,CategoryNode*>::iterator it = _index2categoryNode.find(id);
//
//  if(it != _index2categoryNode.end())
//  {
//   return true;
//  }
//  return false;
//  }

  _category_tree->make_category_tree_layout();
}

////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Set the hihglighted nodes and edges for the category tree
  \remarks
*/

void
Visualization::set_highlighted_categories(ArticleNode* article_node)
{
  _category_tree->_highlight_mode = true;

  Article article = article_node->_article;

  std::vector<uint32_t> parents = article.getParents();

  for (unsigned i = 0; i != parents.size(); ++i)
  {
    Category parent = _wikidb.getCategory(parents[i]);

    CategoryNode* cat_node = _index2categoryNode[parent.index];

    _category_tree->add_highlighted_node(cat_node);


    for (unsigned j = 0; j != cat_node->_outgoingEdges.size(); ++j)
    {
      CategoryNode* cat_parent_node = cat_node->_outgoingEdges[j]->getTarget();

      _category_tree->add_highlighted_node(cat_parent_node);
      _category_tree->add_highlighted_edge(cat_node->_outgoingEdges[j]);

    }
  }
}

void
Visualization::reset_highlighted_categories()
{
  _category_tree->_highlight_mode = false;
  _category_tree->clear_highlighting();
}


} // namespace vtas
