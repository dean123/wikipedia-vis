#include "OverviewRenderer.h"

// gloost
#include <gloost/BinaryFile.h>
#include <gloost/gloostHelper.h>
#include <gloost/gl/gloostGlUtil.h>
#include <gloost/InterleavedAttributes.h>
#include <gloost/Mesh.h>

// boost
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

// cpp
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <algorithm>
#include <cstdlib>
#include <ctime>


////////////////////////////////////////////////////////////////////////////////

namespace vta
{

/**
  \class   OverviewRenderer

  \brief   ...

  \author  Dean Juerges,
           Giuliano Castiglia,
           Bagrat Ter-Akopyan
  \remarks ...
*/

////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Class constructor
  \remarks ...
*/

OverviewRenderer::OverviewRenderer(Visualization* graph):
  _width(0),
  _height(0),
  _mouseState(),

  _projectionMatrix(),
  _viewMatrix(),
  _modelMatrixStack(),

  _translateVector(0.0, 0.0, 0.0),
  _scaleVector(0.001, 0.001, 1.0),

  _mouse_pos(0.0, 0.0, 0.0),

  _zoomingPanningStack(),

  _graph(graph),

  _max_similarity(1.0),
  _min_similarity(0.82),

  _blacklist(),

  _uniformSet(),
  _edgeShader(nullptr),
  _nodeShader(nullptr),
  _typeWriter(nullptr),

  _vboNodes(nullptr),
  _vboEdges(nullptr),

  _highlight_mode(false),
  _cluster_node_vbo(),
  _cluster_edge_vbo(),

  _num_clusters_on_screen(0),
  _cluster_detail_view(false),

  _num_nodes(0),
  _num_edges(0)
{

  // create TypeWriter
  _typeWriter = gloost::FreeTypeWriter::create("../../res/fonts/Verdana.ttf", 12.0f, true);

  // set projection matrix
  _projectionMatrix.setIdentity();
  _viewMatrix.setIdentity();
  _modelMatrixStack.clear();

  gloost::gl::gloostOrtho(_projectionMatrix,
                          0.0, _width,
                          0.0, _height,
                          0.1, 10.0);


  gloost::gl::gloostLookAt(_viewMatrix,
                           gloost::Vector3(0.0, 0.0, 5.0),   // eye pos
                           gloost::Vector3(0.0, 0.0, 0.0),   // look at
                           gloost::Vector3(0.0, 1.0, 0.0));  // up

  _uniformSet.set_mat4("Projection", gloost::mat4(_projectionMatrix));
  _uniformSet.set_mat4("View", gloost::mat4(_viewMatrix));

  std::srand(std::time(0)); // use current time as seed for random generator
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Class destructor
  \remarks ...
*/

/*virtual*/
OverviewRenderer::~OverviewRenderer()
{
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Initialize the visualization
  \remarks ...
*/

bool
OverviewRenderer::initialize()
{
  std::cout << "Number of clusters: " << _graph->get_cluster_num() << std::endl;

  // create shader program and attach all components
  _edgeShader = gloost::gl::ShaderProgram::create();
  _edgeShader->attachShader(GLOOST_SHADERPROGRAM_VERTEX_SHADER,   "../../shaders/edge.vs");
  _edgeShader->attachShader(GLOOST_SHADERPROGRAM_FRAGMENT_SHADER, "../../shaders/edge.fs");

  // create shader program and attach all components
  _nodeShader = gloost::gl::ShaderProgram::create();
  _nodeShader->attachShader(GLOOST_SHADERPROGRAM_VERTEX_SHADER,   "../../shaders/node.vs");
  _nodeShader->attachShader(GLOOST_SHADERPROGRAM_GEOMETRY_SHADER, "../../shaders/node.gs");
  _nodeShader->attachShader(GLOOST_SHADERPROGRAM_FRAGMENT_SHADER, "../../shaders/node.fs");

  fill_vbos();

  std::cout << "Initialized Visualization" << std::endl;

  return true;
}


void
OverviewRenderer::fill_vbos()
{
  std::vector<ArticleNode*> drawable_nodes;
  std::vector<ArticleEdge*> drawable_edges;

  _num_nodes = 0;
  _num_edges = 0;


  for (unsigned i = 0; i != _graph->get_cluster_num(); ++i)
  {
    // Add all nodes
    std::vector<ArticleNode*> cluster_nodes = _graph->get_cluster_by_index(i)->get_nodes();

    if (cluster_nodes.size() > 1)
      _num_nodes += cluster_nodes.size();

    for (unsigned j = 0; j != cluster_nodes.size(); ++j)
      drawable_nodes.push_back(cluster_nodes[j]);

    // Add all edges
    std::vector<ArticleEdge*> cluster_edges = _graph->get_cluster_by_index(i)->get_edges();

    _num_edges += cluster_edges.size();

    for (unsigned j = 0; j != cluster_edges.size(); ++j)
    {
      double similarity = cluster_edges[j]->getWeight();

      if (similarity >= _min_similarity && similarity <= _max_similarity)
      {
        ArticleNode* source = cluster_edges[j]->getSource();
        ArticleNode* target = cluster_edges[j]->getTarget();

        if (source->_x != 0.0 && source->_y != 0.0)
        {
          if (target->_x != 0.0 && target->_y != 0.0)
          {
            drawable_edges.push_back(cluster_edges[j]);
          }
        }
      }
    }
  }

  fill_vbo_nodes(drawable_nodes);
  fill_vbo_edges(drawable_edges);
}

////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Fills the nodes-vbo with all nodes of _graph
  \remarks ...
*/

void
OverviewRenderer::fill_vbo_nodes(std::vector<ArticleNode*> const& nodes)
{
  // init nodes
  int numNodes = nodes.size();

  auto interleavedAttributes = gloost::InterleavedAttributes::create();

  unsigned containerSize = numNodes * 5; // vec2(position) + vec3(color)
  std::vector<float>& container = interleavedAttributes->getVector();
  container = std::vector<float>(containerSize, 0.0f);

  // tell the vertex attribute container which attributes are contained
  interleavedAttributes->addAttribute(2, 8, "in_position");
  interleavedAttributes->addAttribute(3, 12, "in_color");

  unsigned vboIdx = 0;

  for (unsigned i = 0; i != numNodes; ++i)
  {
    ArticleNode* current_node = nodes[i];

    std::string title = current_node->_label;

    if (!blacklist_constains(title))
    {
      container[vboIdx++] = current_node->_x; // node x
      container[vboIdx++] = current_node->_y; // node y

      container[vboIdx++] = current_node->_color[0]; // node color r
      container[vboIdx++] = current_node->_color[1]; // node color g
      container[vboIdx++] = current_node->_color[2]; // node color b
    }
  }
  _vboNodes = gloost::gl::Vbo4::create(interleavedAttributes);
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Fills the edges-vbo with all edges of _graph
  \remarks ...
*/

void
OverviewRenderer::fill_vbo_edges(std::vector<ArticleEdge*> const& edges)
{
  int numEdges = edges.size();

  auto interleavedAttributes = gloost::InterleavedAttributes::create();

  unsigned containerSize = numEdges * 10; // vec2 * 2 + vec3 * 2
  std::vector<float>& container = interleavedAttributes->getVector();
  container = std::vector<float>(containerSize, 0.0f);

  // tell the vertex attribute container which attributes are contained
  interleavedAttributes->addAttribute(2, 8, "in_position");
  interleavedAttributes->addAttribute(3, 12, "in_color");

  unsigned vboIdx = 0;

  for (unsigned i = 0; i != numEdges; ++i)
  {
    ArticleEdge* current_edge = edges[i];

    Node* source = current_edge->getSource();
    Node* target = current_edge->getTarget();

    std::string title_source = source->_label;
    std::string title_target = target->_label;

    if (!blacklist_constains(title_source) && !blacklist_constains(title_target))
    {
      container[vboIdx++] = source->_x; // source x
      container[vboIdx++] = source->_y; // source y

      container[vboIdx++] = current_edge->_color[0]; // edge color r
      container[vboIdx++] = current_edge->_color[1]; // edge color g
      container[vboIdx++] = current_edge->_color[2]; // edge color b

      container[vboIdx++] = target->_x; // target x
      container[vboIdx++] = target->_y; // target y

      container[vboIdx++] = current_edge->_color[0]; // edge color r
      container[vboIdx++] = current_edge->_color[1]; // edge color g
      container[vboIdx++] = current_edge->_color[2]; // edge color b
    }
  }
  _vboEdges = gloost::gl::Vbo4::create(interleavedAttributes);
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Draw nodes and edges
  \remarks ...
*/

void
OverviewRenderer::draw_nodes_and_edges(gloost::vec4 nodes_color, gloost::vec4 edges_color)
{
  // set modelview matrix
  _modelMatrixStack.clear();
  _modelMatrixStack.push();
  {
    if (_cluster_detail_view) // No panning and zooming
    {
      Cluster* detail_cluster = _graph->get_cluster_by_index(_graph->_detail_view_cluster_index);

      // Translate Cluster to the middle of the window
      _modelMatrixStack.translate(_width/2, _height/2, 0.0);

      // Scale Cluster to window size
      float scale = _height/(detail_cluster->get_radius() * 2);
      _modelMatrixStack.scale(scale, scale, 1.0);

      // Translate cluster to the window
      double cluster_x = detail_cluster->get_position_x();
      double cluster_y = detail_cluster->get_position_y();
      _modelMatrixStack.translate(-cluster_x, -cluster_y, 0.0);

      // set current model view matrix
      _uniformSet.set_mat4("Model", gloost::mat4(_modelMatrixStack.top()));
    }
    else
    {
      // Translate vector to x = 0 and y = 0
      auto translate_to_screen_middle = gloost::Vector3((float)_width/2, (float)_height/2, 0.0);

      // Translate vector according to mouse dragging
      auto translate = gloost::Vector3(_scaleVector[0] * _translateVector[0],
                                       _scaleVector[1] * _translateVector[1],
                                       0.0);

      // Translate to 0,0
      _modelMatrixStack.translate(translate);

      // Panning
      _modelMatrixStack.translate(translate_to_screen_middle);

      // Zooming
      _modelMatrixStack.scale(_scaleVector);


      _uniformSet.set_mat4("Model", gloost::mat4(_modelMatrixStack.top())); // set current model view matrix
    }

    _edgeShader->use();
    {
      _uniformSet.applyToShader(_edgeShader.get());

      _vboEdges->bind();
      _vboEdges->draw(GL_LINES);
      _vboEdges->unbind();

    }
    _edgeShader->disable();


    _nodeShader->use();
    {
      _uniformSet.applyToShader(_nodeShader.get());

      _vboNodes->bind();
      _vboNodes->draw(GL_POINTS);
      _vboNodes->unbind();

    }
    _nodeShader->disable();
  }
  _modelMatrixStack.pop();
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Displays the visualization
  \remarks ...
*/

void
OverviewRenderer::display()
{
  // setup clear color and clear screen
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // apply viewport
  glViewport(0, 0, _width, _height);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Colors
  gloost::vec4 red (1.0f, 0.0f, 0.0f, 1.0f);
  gloost::vec4 blue (0.0f, 0.0f, 1.0f, 1.0f);
  gloost::vec4 grey (0.3f, 0.3f, 0.3f, 1.0f);


  // Draw nodes red and edges grey
  draw_nodes_and_edges(red, grey);


  // reset mouse events
  _mouseState.resetMouseEvents();
  _mouseState.setSpeedToZero();

  // In der detail view werden node labels angezeigt
  if (_cluster_detail_view)
  {
    display_node_labels();
  }

  // Sonst infos ueber die overview
  else
  {
    //  Text:
    //  - maximum and minimum displayed similarity
    //  - number of edges and nodes
    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    {
      glDisable(GL_DEPTH_TEST);
      glDepthMask(GL_FALSE);
      glEnable(GL_TEXTURE_2D);

      glMatrixMode(GL_PROJECTION);
      gloostLoadMatrix(_projectionMatrix.data());

      glMatrixMode(GL_MODELVIEW);
      gloostLoadMatrix(_viewMatrix.data());

      _typeWriter->beginText();
      {
        glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
        _typeWriter->nextLine();
        _typeWriter->writeLine(10.0, _height - 20.0, "Number of nodes: " + boost::lexical_cast<std::string>(_num_nodes));
        _typeWriter->nextLine();
        _typeWriter->writeLine(10.0, _height - 50.0, "Number of edges: " + boost::lexical_cast<std::string>(_num_edges));
        _typeWriter->nextLine();
        _typeWriter->writeLine(10.0, _height - 90.0, "Number of clusters: " + boost::lexical_cast<std::string>(_graph->get_cluster_num()));
      }
      _typeWriter->endText();
    }
    glPopAttrib();
    glPopMatrix();
  }
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Displays the node labels
  \remarks ...
*/

void
OverviewRenderer::display_node_labels()
{
  Cluster* detail_cluster = _graph->get_cluster_by_index(_graph->_detail_view_cluster_index);

  // LABEL NODES
  for (unsigned i = 0; i != detail_cluster->get_node_num(); ++i)
  {
    Node* current_node = detail_cluster->get_node(i);

    gloost::Vector3 text_position(current_node->_x, current_node->_y, 0.0);

    // Scale Cluster to window size
    gloost::Matrix model_matrix;
    float scale = _height/(detail_cluster->get_radius() * 2);

    model_matrix.setScale(gloost::Vector3(scale, scale,0.0));

    text_position = model_matrix * text_position;

    // Translate from original cluster position to 0,0
    text_position[0] -= detail_cluster->get_position_x() * scale;
    text_position[1] -= detail_cluster->get_position_y() * scale;

    // translate to the middle of the screen
    text_position[0] += _width/2;
    text_position[1] += _height/2;

    std::string label = current_node->_label;

    // draw text
    glPushMatrix();
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    {
      glDisable(GL_DEPTH_TEST);
      glDepthMask(GL_FALSE);
      glEnable(GL_TEXTURE_2D);

      glMatrixMode(GL_PROJECTION);
      gloostLoadMatrix(_projectionMatrix.data());

      glMatrixMode(GL_MODELVIEW);
      gloostLoadMatrix(_viewMatrix.data());

      _typeWriter->beginText();
      _typeWriter->setScale(2.0);
      {
        glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
        _typeWriter->writeLine(text_position[0], text_position[1], label);
      }
      _typeWriter->endText();
    }
    glPopAttrib();
    glPopMatrix();
  }
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Resizes the visualization
  \param   width   The width
  \param   height  The height
  \remarks ...
*/

void
OverviewRenderer::resize(int width, int height)
{
  _width  = width;
  _height = height;

  gloost::gl::gloostOrtho(_projectionMatrix,
                          0.0, _width,
                          0.0, _height,
                          0.1, 10.0);

  _uniformSet.set_mat4("Projection", gloost::mat4(_projectionMatrix));
}


////////////////////////////////////////////////////////////////////////////////


void
OverviewRenderer::highlight_node(ArticleNode* current_node)
{
  std::vector<ArticleNode*> highlighted_nodes;
  std::vector<ArticleEdge*> highlighted_edges;

  highlighted_nodes.push_back(current_node);

  // Highlight categories in category-tree
  _graph->set_highlighted_categories(current_node);


  // For all outgoing edges
  for (unsigned i = 0; i != current_node->_outgoingEdges.size(); i++)
  {
    ArticleNode* neighboor_node = current_node->_outgoingEdges[i]->getTarget();

    // Push back all outgoing edges and their target nodes
    highlighted_edges.push_back(current_node->_outgoingEdges[i]);
    highlighted_nodes.push_back(neighboor_node);
  }

  // For all incoming edges
  for (unsigned i = 0; i != current_node->_incomingEdges.size(); i++)
  {
    ArticleNode* neighboor_node = current_node->_incomingEdges[i]->getSource();

    // Push back all incoming edges and their source-nodes
    highlighted_edges.push_back(current_node->_incomingEdges[i]);
    highlighted_nodes.push_back(neighboor_node);
  }

  fill_vbo_nodes(highlighted_nodes);
  fill_vbo_edges(highlighted_edges);
}


////////////////////////////////////////////////////////////////////////////////
/**
  \brief   Mouse input
  \param   x     The x-position
  \param   y     The y-position
  \param   btn   The pressed button
  \param   mods  The modification key
  \remarks ...
*/

void
OverviewRenderer::mousePress(int x, int y, int btn, int mods)
{
  _mouseState.setButtonState(btn + 1, true);
  _mouseState.setPosition((float)x, (float)(_height - y));

  if (_mouseState.getButtonState(GLOOST_MOUSESTATE_BUTTON1))
  {
    gloost::Vector3 mouse_position = _mouseState.getPosition();

    if (_cluster_detail_view) /// Click on nodes
    {
      Cluster* detail_cluster = _graph->get_cluster_by_index(_graph->_detail_view_cluster_index);

      gloost::Matrix model_matrix;

      auto matrixStack = gloost::MatrixStack();

      // Translate Cluster to the middle of the window
      matrixStack.translate(_width/2, _height/2, 0.0);

      // Scale Cluster to window size
      float scale = _height/(detail_cluster->get_radius() * 2);
      matrixStack.scale(scale, scale, 1.0);

      // Translate cluster to the window
      double cluster_x = detail_cluster->get_position_x();
      double cluster_y = detail_cluster->get_position_y();
      matrixStack.translate(-cluster_x, -cluster_y, 0.0);

      double node_size = 40.0;

      bool clicked_on_node = false;

      for (unsigned i = 0; i != detail_cluster->get_node_num(); ++i)
      {
        ArticleNode* current_node = detail_cluster->get_node(i);

        auto node_pos = gloost::Point3(current_node->_x, current_node->_y, 0.0);

        node_pos = matrixStack.top() * node_pos;

        // Check if mouse is over this node
        if (mouse_position[0] >= node_pos[0] - node_size && mouse_position[0] <= node_pos[0] + node_size &&
            mouse_position[1] >= node_pos[1] - node_size && mouse_position[1] <= node_pos[1] + node_size)
            {
              highlight_node(current_node);

              clicked_on_node = true;
              _highlight_mode = true;
            }
      }

      if (!clicked_on_node && _highlight_mode)
      {
        // Fill vbos with new positions
        fill_vbos();

        _highlight_mode = false;
        _graph->reset_highlighted_categories();
      }

    }


    else /// Click on clusters
    {
      gloost::Point3 current_mouse_pos = _mouseState.getPosition();

      gloost::Vector3 translate = gloost::Vector3(_scaleVector[0] * _translateVector[0],
                                                  _scaleVector[1] * _translateVector[1],
                                                  0.0);

      auto translate_to_middle = gloost::Vector3((float)_width/2, (float)_height/2, 0.0);

      auto matrixStack = gloost::MatrixStack();
      matrixStack.push();
      {
        // Inverted translate
        current_mouse_pos = current_mouse_pos - translate_to_middle;

        current_mouse_pos = current_mouse_pos - translate;


        // Inverted scale
        matrixStack.scale(_scaleVector);
        current_mouse_pos = matrixStack.top().inverted() * current_mouse_pos;
      }
      matrixStack.pop();


      for (unsigned i = 0; i != _graph->get_cluster_num(); ++i)
      {
        Cluster* current_cluster = _graph->get_cluster_by_index(i);

        if (current_cluster->within(current_mouse_pos))
        {
          _graph->_detail_view_cluster_index = i;

          _cluster_detail_view = true;

          // Set Category tree
          _graph->build_category_tree(_graph->get_cluster_by_index(i));
        }
      }
    }
  }
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Mouse input
  \param   x     The x-position
  \param   y     The y-position
  \param   btn   The pressed button
  \param   mods  The modification key
  \remarks ...
*/

void
OverviewRenderer::mouseRelease(int x, int y, int btn, int mods)
{
  _mouseState.setButtonState(btn + 1, false);
  _mouseState.setPosition((float)x, (float)(_height - y));
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Mouse input
  \param   x     The x-position
  \param   y     The y-position
  \remarks ...
*/

void
OverviewRenderer::mouseMove(int x, int y)
{
  _mouseState.setPosition((float)x, (float)(_height - y));

  gloost::Vector3 mouse_position = _mouseState.getPosition();

  /// Panning
  if (!_cluster_detail_view)
  {
    if (_mouseState.getButtonState(GLOOST_MOUSESTATE_BUTTON2))
    {
      gloost::Vector3 mouse_drag = _mouseState.getSpeed();

      _translateVector += gloost::Vector3(mouse_drag[0] * 1/_scaleVector[0], mouse_drag[1] * 1/_scaleVector[1], 0.0);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Mouse scroll
  \param   y_offset   Scroll wheel offset
  \remarks ...
*/

void
OverviewRenderer::mouseScrollEnhance()
{
  if (!_cluster_detail_view)
  {
    // Zoom in
    _scaleVector[0] = _scaleVector[0] * 1.1;
    _scaleVector[1] = _scaleVector[1] * 1.1;

    _mouse_pos = _mouseState.getPosition();
  }
}

void
OverviewRenderer::mouseScrollDecrease()
{
  if (!_cluster_detail_view)
  {
    // Zoom out
    _scaleVector[0] = _scaleVector[0] * 0.8;
    _scaleVector[1] = _scaleVector[1] * 0.8;

    _mouse_pos = _mouseState.getPosition();
  }
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   When imgui mouse released, this function is called
  \param
  \remarks ...
*/

void
OverviewRenderer::imgui_mouse_released()
{
  // Get new edges
  _graph->add_edges_for_sim(_min_similarity, _max_similarity);

  fill_vbos();
}

////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Key input
  \param   key   The key code as enum
  \param   mods  The modification key
  \remarks ...
*/

void
OverviewRenderer::keyPress(int key, int mods)
{
  // http://www.glfw.org/docs/latest/group__keys.html

  std::cerr << "Visualization::keyPress( " << key << " )" << std::endl;

  switch (key)
  {
    case 82: //GLFW_KEY_R
    {
      // Fill vbos with new positions
      fill_vbos();

      // Reload shaders
      _edgeShader->reloadShaders();
      _nodeShader->reloadShaders();

      std::cout << "Reload shaders" << std::endl;

      break;
    }

    case 49: // 1
    {
      std::cout << "Change layout to radial" << std::endl;

      // Change layout for every cluster
      for (unsigned i = 0; i != _graph->get_cluster_num(); ++i)
        _graph->get_cluster_by_index(i)->make_radial_layout();

      // Fill vbos with new positions
      fill_vbos();

      break;
    }

    case 50: // 2
    {
      std::cout << "Change layout to ring" << std::endl;

      // Change layout for every cluster
      for (unsigned i = 0; i != _graph->get_cluster_num(); ++i)
        _graph->get_cluster_by_index(i)->make_ring_layout();

      // Fill vbos with new positions
      fill_vbos();

      break;
    }

    case 72: // H
    {
      break;
    }

    case 73: // I
    {
      // Compute the number of clusters on screen
      unsigned per_row = _graph->get_clusters_per_row_num();
      double c_size = _graph->get_cluster_size();

      c_size = c_size * _scaleVector[0];

      unsigned one_row_on_screen = ceil(_width / c_size);
      unsigned one_column_on_screen = ceil(_height / c_size);

      unsigned clusters_on_screen = one_row_on_screen * one_column_on_screen;

      std::cout << "Anzahl cluster on screen: " << clusters_on_screen << std::endl;

      break;
    }

    case 256: // ESC
    {
      if (!_highlight_mode)
      {
        _cluster_detail_view = false;
        _graph->get_category_tree()->clear();
      }

      break;
    }

    case 321: // NUM 1
    {
      std::cout << "Change global layout to ordered squares" << std::endl;

      _graph->set_cluster_positions();

      // re-layout all clusters
      for (unsigned i = 0; i != _graph->get_cluster_num(); ++i)
        _graph->get_cluster_by_index(i)->make_radial_layout();

      // Fill vbos with new positions
      fill_vbos();

      break;
    }

    case 322: // NUM 2
    {
      std::cout << "Change global layout to radial" << std::endl;

      _graph->make_global_radial_layout();

      // re-layout all clusters
      for (unsigned i = 0; i != _graph->get_cluster_num(); ++i)
        _graph->get_cluster_by_index(i)->make_radial_layout();

      // Fill vbos with new positions
      fill_vbos();

      break;
    }

    default:
    {}
  }

}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Key input
  \param   key   The key code as enum
  \param   mods  The modification key
  \remarks ...
*/

void
OverviewRenderer::keyRelease(int key, int mods)
{
}



////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Set minimum and maximum similarity thresholds
  \param   sim similarity
  \remarks ...
*/

void
OverviewRenderer::set_minimum_similarity(double sim)
{
  _min_similarity = sim;
}


void
OverviewRenderer::set_maximum_similarity(double sim)
{
  _max_similarity = sim;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Add string to blacklist to filter article titles
  \param   title
  \remarks ...
*/

void
OverviewRenderer::add_to_blacklist(std::string const title)
{
  if(std::find(_blacklist.begin(), _blacklist.end(), title) != _blacklist.end())
  {
    std::cout << title << " is allready in blacklist" << std::endl;
  }
  else
  {
    _blacklist.push_back(title);
  }
}


bool
OverviewRenderer::blacklist_constains(std::string const title)
{
	for (unsigned i = 0; i != _blacklist.size(); i++)
  {
    std::size_t found = title.find(_blacklist[i]);
    if (found!=std::string::npos)
    {
      return true;
    }
  }

  return false;
}


std::vector<std::string>
OverviewRenderer::get_blacklist()
{
  return _blacklist;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Get next cluster
  \param
  \remarks ...
*/

void
OverviewRenderer::get_next_cluster(unsigned number_of_cluster)
{
  for (unsigned i = 0; i != number_of_cluster; ++i)
    _graph->get_next_cluster();

  // Sort and set new cluster positions
//  _graph->set_cluster_positions();
  _graph->make_global_radial_layout();

  // re-layout all clusters
  for (unsigned i = 0; i != _graph->get_cluster_num(); ++i)
    _graph->get_cluster_by_index(i)->make_radial_layout();

  // Fill vbos with new positions
  fill_vbos();
}


////////////////////////////////////////////////////////////////////////////////

} // namespace vta
