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

  _translateVector(0.0, 0.0, 0.0),
  _scaleVector(1.0, 1.0, 1.0),

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
  CategoryTree* category_tree = _graph->get_category_tree();

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
    CategoryNode* current_node = category_tree->get_node(i);

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
  CategoryTree* category_tree = _graph->get_category_tree();

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
    CategoryEdge* current_edge = category_tree->get_edge(i);

    CategoryNode* source = current_edge->getSource();
    CategoryNode* target = current_edge->getTarget();

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
  CategoryTree* category_tree = _graph->get_category_tree();

  fill_vbo_nodes();
  fill_vbo_edges();

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
    // Scale and Translate is disabled
//    _modelMatrixStack.translate(_translateVector[0], _translateVector[1], 0.0);
//    _modelMatrixStack.scale(_scaleVector);

    // set current model view matrix
    _uniformSet.set_mat4("Model", _modelMatrixStack.top());

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

  if (_graph->get_category_tree()->_highlight_mode)
  {
    label_nodes(_graph->get_category_tree()->get_highlighted_nodes());
  }
  else
  {
    label_nodes(_graph->get_category_tree()->get_all_nodes());
  }
}


void
DetailRenderer::label_nodes(std::vector<CategoryNode*> const& nodes)
{
  for (unsigned i = 0; i != nodes.size(); ++i)
  {
    CategoryNode* current_node = nodes[i];

    gloost::Vector3 text_position(current_node->_x, current_node->_y, 0.0);

    _modelMatrixStack.clear();
    _modelMatrixStack.push();
    {
      _modelMatrixStack.scale(_scaleVector);

       text_position = text_position + _translateVector;
       text_position = _modelMatrixStack.top() * text_position;
    }

    _modelMatrixStack.pop();

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

//  /// Panning
//  if (_mouseState.getButtonState(GLOOST_MOUSESTATE_BUTTON2))
//  {
//    gloost::Vector3 mouse_drag = _mouseState.getSpeed();
//
//    _translateVector += gloost::Vector3(mouse_drag[0], mouse_drag[1], 0.0);
//  }
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
//  _scaleVector[0] = _scaleVector[0] * 1.1;
//  _scaleVector[1] = _scaleVector[1] * 1.1;
}

void
DetailRenderer::mouseScrollDecrease()
{
  // Zoom out
//  _scaleVector[0] = _scaleVector[0] * 0.8;
//  _scaleVector[1] = _scaleVector[1] * 0.8;
}



////////////////////////////////////////////////////////////////////////////////
} // namespace vta

