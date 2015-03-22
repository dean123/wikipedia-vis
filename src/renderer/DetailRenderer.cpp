#include "DetailRenderer.hpp"

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



namespace vta
{

////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Class constructor
  \remarks ...
*/

DetailRenderer::DetailRenderer(Visualization* graph):

  _width(0),
  _height(0),

  _mouseState(),

  _projectionMatrix(),
  _viewMatrix(),
  _modelMatrixStack(),

  _translateMatrix(),
  _scaleMatrix(),
  _panning_zooming_mat(),

  _graph(graph),

  _uniformSet(),
  _edgeShader(nullptr),
  _nodeShader(nullptr),
  _typeWriter(nullptr),
  _vboNodes(nullptr),
  _vboEdges(nullptr)
{
  // create TypeWriter
  _typeWriter = gloost::FreeTypeWriter::create("../../res/fonts/Verdana.ttf", 12.0f, true);

  // Set Model View Projection Matrices
  _projectionMatrix.setIdentity();
  _viewMatrix.setIdentity();
  _modelMatrixStack.clear();

  _translateMatrix.setIdentity();
  _scaleMatrix.setIdentity();
  _panning_zooming_mat.setIdentity();

  gloost::gl::gloostOrtho(_projectionMatrix,
                          0.0, _width,
                          0.0, _height,
                          0.1, 10.0);


  gloost::gl::gloostLookAt(_viewMatrix,
                           gloost::Vector3(0.0, 0.0, 5.0),   // eye pos
                           gloost::Vector3(0.0, 0.0, 0.0),   // look at
                           gloost::Vector3(0.0, 1.0, 0.0));  // up


  panningZooming(0.0001);

  _uniformSet.set_mat4("Projection", gloost::mat4(_projectionMatrix));
  _uniformSet.set_mat4("View", gloost::mat4(_viewMatrix));
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Class destructor
  \remarks ...
*/

DetailRenderer::~DetailRenderer()
{}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Initialize the cluster visualization
  \remarks ...
*/

bool
DetailRenderer::initialize()
{
  // create shader program and attach all components for edges
  _edgeShader = gloost::gl::ShaderProgram::create();
  _edgeShader->attachShader(GLOOST_SHADERPROGRAM_VERTEX_SHADER,   "../../shaders/edge.vs");
  _edgeShader->attachShader(GLOOST_SHADERPROGRAM_FRAGMENT_SHADER, "../../shaders/edge.fs");

  // create shader program and attach all components for nodes
  _nodeShader = gloost::gl::ShaderProgram::create();
  _nodeShader->attachShader(GLOOST_SHADERPROGRAM_VERTEX_SHADER,   "../../shaders/node.vs");
  _nodeShader->attachShader(GLOOST_SHADERPROGRAM_GEOMETRY_SHADER, "../../shaders/node.gs");
  _nodeShader->attachShader(GLOOST_SHADERPROGRAM_FRAGMENT_SHADER, "../../shaders/node.fs");

  fill_vbo_nodes();
  fill_vbo_edges();

  std::cout << "Initialized Cluster Visualization" << std::endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Fills the nodes-vbo with all nodes of the category_tree
  \remarks ...
*/

void
DetailRenderer::fill_vbo_nodes()
{
  Cluster* category_tree = _graph->get_category_tree();

  // init nodes
  int numNodes = category_tree->get_node_num();

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
    Node* current_node = category_tree->get_node(i);

    container[vboIdx++] = current_node->_x; // node x
    container[vboIdx++] = current_node->_y; // node y

    container[vboIdx++] = current_node->_color[0]; // node color r
    container[vboIdx++] = current_node->_color[1]; // node color g
    container[vboIdx++] = current_node->_color[2]; // node color b
  }
  _vboNodes = gloost::gl::Vbo4::create(interleavedAttributes);
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Fills the edges-vbo with all edges of the cluster
  \remarks ...
*/

void
DetailRenderer::fill_vbo_edges()
{
  Cluster* category_tree = _graph->get_category_tree();

  int numEdges = category_tree->get_edge_num();

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
    Edge* current_edge = category_tree->get_edge(i);

    Node* source = current_edge->getSource();
    Node* target = current_edge->getTarget();

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
  _vboEdges = gloost::gl::Vbo4::create(interleavedAttributes);
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Displays the visualization
  \remarks ...
*/

void
DetailRenderer::display()
{
  Cluster* category_tree = _graph->get_category_tree();

  // setup clear color and clear screen
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // apply viewport
  glViewport(0, 0, _width, _height);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // set modelview matrix
  _modelMatrixStack.clear();
  _modelMatrixStack.push();
  {
//    _modelMatrixStack.translate(_translateVector[0], _translateVector[1], 0.0);
//    _modelMatrixStack.scale(_scaleVector);

    // set current model view matrix
    _uniformSet.set_mat4("Model", _panning_zooming_mat);

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
//
//
//  // LABEL NODES
//  for (unsigned i = 0; i != cluster->get_node_num(); ++i)
//  {
//    Node* current_node = cluster->get_node(i);
//
//    gloost::Vector3 text_position(current_node->_x, current_node->_y, 0.0);
//
//    // Scale Cluster to window size
//    gloost::Matrix model_matrix;
//    float scale = _height/(cluster->get_radius() * 2);
//
//    model_matrix.setScale(gloost::Vector3(scale, scale,0.0));
//
//    text_position = model_matrix * text_position;
//
//    // Translate from original cluster position to 0,0
//    text_position[0] -= cluster->get_position_x() * scale;
//    text_position[1] -= cluster->get_position_y() * scale;
//
//    // translate to the middle of the screen
//    text_position[0] += _width/2;
//    text_position[1] += _height/2;
//
//    std::string label = current_node->_label;
//
//    // draw text
//    glPushMatrix();
//    glPushAttrib(GL_ALL_ATTRIB_BITS);
//    {
//      glDisable(GL_DEPTH_TEST);
//      glDepthMask(GL_FALSE);
//      glEnable(GL_TEXTURE_2D);
//
//      glMatrixMode(GL_PROJECTION);
//      gloostLoadMatrix(_projectionMatrix.data());
//
//      glMatrixMode(GL_MODELVIEW);
//      gloostLoadMatrix(_viewMatrix.data());
//
//      /// _typewriter functions
//      /*
//      // cuts a line, so it is not longer as maxPixelLength with the current font
//      std::string cutLineToLength(const std::string& text, unsigned maxPixelLength) const;
//
//      // sets the scaling
//      void setScale(float scale);
//      */
//
//      _typeWriter->beginText();
//      _typeWriter->setScale(2.0);
//      {
//        glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
//        _typeWriter->writeLine(text_position[0], text_position[1], label);
//      }
//      _typeWriter->endText();
//    }
//    glPopAttrib();
//    glPopMatrix();
//  }
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Resizes the visualization
  \remarks ...
*/

void
DetailRenderer::resize(int width, int height)
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
  \brief   Mouse input
  \param   x     The x-position
  \param   y     The y-position
  \param   btn   The pressed button
  \param   mods  The modification key
  \remarks ...
*/

void
DetailRenderer::mousePress(int x, int y, int btn, int mods)
{
  _mouseState.setButtonState(btn + 1, true);
  _mouseState.setPosition((float)x, (float)(_height - y));
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
DetailRenderer::mouseRelease(int x, int y, int btn, int mods)
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
DetailRenderer::mouseMove(int x, int y)
{
  _mouseState.setPosition((float)x, (float)(_height - y));

  /// Panning
  gloost::Matrix old_trans_matrix = _translateMatrix;

  if (_mouseState.getButtonState(GLOOST_MOUSESTATE_BUTTON2))
  {
    gloost::Vector3 mouse_drag = _mouseState.getSpeed();

    gloost::Vector3 scale_vector = _scaleMatrix.getScale();

    _translateMatrix.setTranslate(mouse_drag[0] / (scale_vector[0] * 2000.0),
                                  mouse_drag[1] / (scale_vector[1] * 2000.0),
                                  0.0);

    _translateMatrix = old_trans_matrix * _translateMatrix;

    panningZooming(1.0);
  }
}

////////////////////////////////////////////////////////////////////////////////

void
DetailRenderer::panningZooming(float scale)
{
  gloost::Matrix new_scale_mat;

  new_scale_mat.setIdentity();
  new_scale_mat.setScale(scale);

  _scaleMatrix = _scaleMatrix * new_scale_mat;

  _panning_zooming_mat = _scaleMatrix * _translateMatrix;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Mouse scroll
  \param   y_offset   Scroll wheel offset
  \remarks ...
*/

void
DetailRenderer::mouseScrollEnhance()
{
  // Zoom in
  panningZooming(1.1);
}

void
DetailRenderer::mouseScrollDecrease()
{
  // Zoom out
  panningZooming(0.8);
}



////////////////////////////////////////////////////////////////////////////////
} // namespace vta

