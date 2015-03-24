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
  _scaleVector(0.005, 0.005, 1.0),

  _mouse_pos(0.0, 0.0, 0.0),

  _zoomingPanningStack(),

  _graph(graph),

  _max_similarity(1.0),
  _min_similarity(0.64),

  _blacklist(),

  _uniformSet(),
  _edgeShader(nullptr),
  _nodeShader(nullptr),
  _typeWriter(nullptr),

  _vboNodes(nullptr),
  _vboEdges(nullptr),

  _highlight_at_mouse_over(false),
  _cluster_node_vbo(),
  _cluster_edge_vbo(),

  _num_clusters_on_screen(0),
  _cluster_detail_view(false)
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

  for (unsigned i = 0; i != _graph->get_cluster_num(); ++i)
  {
    Cluster* current_cluster = _graph->get_cluster_by_index(i);

    // Initial layout
    current_cluster->make_radial_layout();

//    create_cluster_node_vbo(current_cluster);
//    create_cluster_edge_vbo(current_cluster);
  }

  std::cout << "Number of cluster vbos: " << _cluster_node_vbo.size() << std::endl;

  // create shader program and attach all components
  _edgeShader = gloost::gl::ShaderProgram::create();
  _edgeShader->attachShader(GLOOST_SHADERPROGRAM_VERTEX_SHADER,   "../../shaders/edge.vs");
  _edgeShader->attachShader(GLOOST_SHADERPROGRAM_FRAGMENT_SHADER, "../../shaders/edge.fs");

  // create shader program and attach all components
  _nodeShader = gloost::gl::ShaderProgram::create();
  _nodeShader->attachShader(GLOOST_SHADERPROGRAM_VERTEX_SHADER,   "../../shaders/node.vs");
  _nodeShader->attachShader(GLOOST_SHADERPROGRAM_GEOMETRY_SHADER, "../../shaders/node.gs");
  _nodeShader->attachShader(GLOOST_SHADERPROGRAM_FRAGMENT_SHADER, "../../shaders/node.fs");

  // Fill vbos with new positions
  fill_vbo_nodes();
  fill_vbo_edges();

  std::cout << "Initialized Visualization" << std::endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Add new node-vbo for new cluster
  \remarks ...
*/

void
OverviewRenderer::create_cluster_node_vbo(Cluster* current_cluster)
{
  std::shared_ptr<gloost::gl::Vbo4> vboNodes;

  // init nodes
  int numNodes = current_cluster->get_node_num();

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
    Node* current_node = current_cluster->get_node(i);

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
  vboNodes = gloost::gl::Vbo4::create(interleavedAttributes);

  _cluster_node_vbo.push_back(vboNodes);
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Add new edge-vbo for new cluster
  \remarks ...
*/

void
OverviewRenderer::create_cluster_edge_vbo(Cluster* current_cluster)
{
  std::shared_ptr<gloost::gl::Vbo4> vboEdge;

  int numEdges = current_cluster->get_edge_num();

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
    Edge* current_edge = current_cluster->get_edge(i);

    Node* source = current_edge->getSource();
    Node* target = current_edge->getTarget();

    std::string title_source = source->_label;
    std::string title_target = target->_label;

    if (!blacklist_constains(title_source) && !blacklist_constains(title_target))
    {
      double similarity = current_edge->getWeight();

      if (similarity >= _min_similarity && similarity <= _max_similarity)
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
  }

  vboEdge = gloost::gl::Vbo4::create(interleavedAttributes);

  _cluster_edge_vbo.push_back(vboEdge);
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Fills the nodes-vbo with all nodes of _graph
  \remarks ...
*/

void
OverviewRenderer::fill_vbo_nodes()
{
  // init nodes
  int numNodes = _graph->get_node_num();

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
    Node* current_node = _graph->get_node_by_index(i);

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
OverviewRenderer::fill_vbo_edges()
{
  int numEdges = _graph->get_edge_num();

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
    Edge* current_edge = _graph->get_edge_by_index(i);

    Node* source = current_edge->getSource();
    Node* target = current_edge->getTarget();

    std::string title_source = source->_label;
    std::string title_target = target->_label;

    if (!blacklist_constains(title_source) && !blacklist_constains(title_target))
    {
      double similarity = current_edge->getWeight();

      if (similarity >= _min_similarity && similarity <= _max_similarity)
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
      //_modelMatrixStack.translate(_translateVector[0], _translateVector[1], 0.0);
//      _modelMatrixStack.translate(_mouse_pos);
      _modelMatrixStack.scale(_scaleVector);

      _uniformSet.set_mat4("Model", gloost::mat4(_modelMatrixStack.top())); // set current model view matrix
    }

    _edgeShader->use();
    {
      _uniformSet.applyToShader(_edgeShader.get());

//      for (unsigned i = 0; i != _cluster_edge_vbo.size(); ++i)
//      {
//        _cluster_edge_vbo[i]->bind();
//        _cluster_edge_vbo[i]->draw(GL_LINES);
//        _cluster_edge_vbo[i]->unbind();
//      }

      _vboEdges->bind();
      _vboEdges->draw(GL_LINES);
      _vboEdges->unbind();

    }
    _edgeShader->disable();


    _nodeShader->use();
    {
      _uniformSet.applyToShader(_nodeShader.get());

//      for (unsigned i = 0; i != _cluster_node_vbo.size(); ++i)
//      {
//        _cluster_node_vbo[i]->bind();
//        _cluster_node_vbo[i]->draw(GL_POINTS);
//        _cluster_node_vbo[i]->unbind();
//      }

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
  //fill_vbo_edges();

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
        _typeWriter->writeLine(10.0, _height - 20.0, "Maximum similarity: " + boost::lexical_cast<std::string>(_max_similarity));
        _typeWriter->nextLine();
        _typeWriter->writeLine(10.0, _height - 30.0, "Minimum similarity: " + boost::lexical_cast<std::string>(_min_similarity));

        _typeWriter->nextLine();
        _typeWriter->writeLine(10.0, _height - 40.0, "Number of nodes: " + boost::lexical_cast<std::string>(_graph->get_node_num()));
        _typeWriter->nextLine();
        _typeWriter->writeLine(10.0, _height - 50.0, "Number of edges: " + boost::lexical_cast<std::string>(_graph->get_edge_num()));
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

/**
  \brief   Returns the index of the cluster at the current mouse position
  \remarks ...
*/

unsigned
OverviewRenderer::get_cluster_index_from_mouse_pos(gloost::Vector3 mouse_position)
{
  unsigned idx = _graph->get_cluster_num()+1;

  if (mouse_position[0] > 0.0 && mouse_position[1] > 0.0)
  {
    _modelMatrixStack.clear();
    _modelMatrixStack.push();
    {
      _modelMatrixStack.scale(_scaleVector);

       mouse_position = mouse_position - _translateVector;
       mouse_position = _modelMatrixStack.top().inverted() * mouse_position;
    }

    _modelMatrixStack.pop();

    double cluster_size = _graph->get_cluster_size();
    unsigned clusters_per_row = _graph->get_clusters_per_row_num();

    idx = floor( mouse_position[0] / cluster_size ) +
         (floor( mouse_position[1] / cluster_size ) * clusters_per_row);
  }

  return idx;
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

      for (unsigned i = 0; i != detail_cluster->get_node_num(); ++i)
      {
        Node* current_node = detail_cluster->get_node(i);

        auto node_pos = gloost::Point3(current_node->_x, current_node->_y, 0.0);

        node_pos = matrixStack.top() * node_pos;

        // Check if mouse is over this node
        if (mouse_position[0] >= node_pos[0] - node_size && mouse_position[0] <= node_pos[0] + node_size &&
            mouse_position[1] >= node_pos[1] - node_size && mouse_position[1] <= node_pos[1] + node_size)
            {
              std::cout << current_node->_label << std::endl;
            }
      }

    }


    else /// Click on clusters
    {
      unsigned cluster_idx = get_cluster_index_from_mouse_pos(mouse_position);
      if (cluster_idx < _graph->get_cluster_num()) // mouse pos is inside graph bounding
      {
        _graph->_detail_view_cluster_index = cluster_idx;

        _cluster_detail_view = true;

        // Set Category tree
        _graph->build_category_tree(_graph->get_cluster_by_index(cluster_idx));
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
      //gloost::Vector3 mouse_drag = _mouseState.getSpeed();



      //_translateVector += gloost::Vector3(mouse_drag[0], mouse_drag[1], 0.0);

      _translateVector += _mouseState.getPosition() - _mouseState.getLastMouseDownPosition();

//      int imgx = (int)(startx + (deltaX / zoom));
//      int imgy = (int)(starty + (deltaY / zoom));
    }
  }


//  /// Mouse over node functions
//  if (_highlight_at_mouse_over)
//  {
//    _highlighted_nodes.clear();
//    _highlighted_edges.clear();
//
//    double node_size = 15000.0;
//
//    gloost::Vector3 mouse_position = _mouseState.getPosition();
//
//    unsigned cluster_idx = get_cluster_index_from_mouse_pos(mouse_position);
//
//    // mouse pos is inside graph bounding
//    if (cluster_idx < _graph->get_cluster_num())
//    {
//      Cluster* cluster_mouse_inside = _graph->get_cluster_by_index(cluster_idx);
//
//      // For all nodes of this cluster
//      for (unsigned i = 0; i != cluster_mouse_inside->get_node_num(); ++i)
//      {
//        Node* current_node = cluster_mouse_inside->get_node(i);
//
//        // Check if mouse is over this node
//        if (mouse_position[0] >= current_node->_x - node_size && mouse_position[0] <= current_node->_x + node_size &&
//            mouse_position[1] >= current_node->_y - node_size && mouse_position[1] <= current_node->_y + node_size)
//        {
//          _highlighted_nodes.push_back(current_node);
//
//          // For all outgoing edges
//          for (unsigned i = 0; i != current_node->outgoingEdges.size(); i++)
//          {
//            Node* neighboor_node = current_node->outgoingEdges[i]->getTarget();
//
//            // Push back all outgoing edges and their target nodes
//            _highlighted_edges.push_back(current_node->outgoingEdges[i]);
//            _highlighted_nodes.push_back(neighboor_node);
//          }
//
//          // For all incoming edges
//          for (unsigned i = 0; i != current_node->incomingEdges.size(); i++)
//          {
//            Node* neighboor_node = current_node->incomingEdges[i]->getSource();
//
//            // Push back all incoming edges and their source-nodes
//            _highlighted_edges.push_back(current_node->incomingEdges[i]);
//            _highlighted_nodes.push_back(neighboor_node);
//          }
//        }
//      }
//    }
//  }
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
      fill_vbo_nodes();
      fill_vbo_edges();

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
      fill_vbo_nodes();
      fill_vbo_edges();

      break;
    }

    case 50: // 2
    {
      std::cout << "Change layout to ring" << std::endl;

      // Change layout for every cluster
      for (unsigned i = 0; i != _graph->get_cluster_num(); ++i)
        _graph->get_cluster_by_index(i)->make_ring_layout();

      // Fill vbos with new positions
      fill_vbo_nodes();
      fill_vbo_edges();

      break;
    }

    case 72: // H
    {
      _highlight_at_mouse_over = !_highlight_at_mouse_over;

      std::cout << "Toggle highlight mode to: " << _highlight_at_mouse_over << std::endl;

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
      _cluster_detail_view = false;
      _graph->get_category_tree()->clear();

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
  _graph->set_cluster_positions();

  // re-layout all clusters
  for (unsigned i = 0; i != _graph->get_cluster_num(); ++i)
    _graph->get_cluster_by_index(i)->make_radial_layout();

  // Fill vbos with new positions
  fill_vbo_nodes();
  fill_vbo_edges();
}


////////////////////////////////////////////////////////////////////////////////

} // namespace vta

