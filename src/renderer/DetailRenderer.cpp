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

  _projectionMatrix(),
  _viewMatrix(),
  _modelMatrixStack(),

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

  std::cout << "Initialized Cluster Visualization" << std::endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Fills the nodes-vbo with all nodes of the cluster
  \remarks ...
*/

void
DetailRenderer::fill_vbo_nodes()
{
  Cluster* cluster = _graph->get_cluster_by_index(_graph->_detail_view_cluster_index);

  // init nodes
  int numNodes = cluster->get_node_num();

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
    Node* current_node = cluster->get_node(i);

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
  Cluster* cluster = _graph->get_cluster_by_index(_graph->_detail_view_cluster_index);

  int numEdges = cluster->get_edge_num();

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
    Edge* current_edge = cluster->get_edge(i);

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
  fill_vbo_nodes();
  fill_vbo_edges();

  Cluster* cluster = _graph->get_cluster_by_index(_graph->_detail_view_cluster_index);

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
    // Translate Cluster to the middle of the window
    _modelMatrixStack.translate(_width/2, _height/2, 0.0);

    // Scale Cluster to window size
    float scale = _height/(cluster->get_radius() * 2);
    _modelMatrixStack.scale(scale, scale, 1.0);

    // Translate cluster to the window
    double cluster_x = cluster->get_position_x();
    double cluster_y = cluster->get_position_y();
    _modelMatrixStack.translate(-cluster_x, -cluster_y, 0.0);

    // set current model view matrix
    _uniformSet.set_mat4("Model", gloost::mat4(_modelMatrixStack.top()));

    _edgeShader->use();
    {
      // Set Edge Color
      _uniformSet.set_vec4("Color", gloost::vec4(0.3, 0.3f, 0.3f, 1.0f));

      _uniformSet.applyToShader(_edgeShader.get());

      _vboEdges->bind();
      _vboEdges->draw(GL_LINES);
      _vboEdges->unbind();
    }
    _edgeShader->disable();


    _nodeShader->use();
    {
      // Set Node Color
      _uniformSet.set_vec4("Color", gloost::vec4(1.0, 0.0f, 0.0f, 1.0f));

      _uniformSet.applyToShader(_nodeShader.get());

      _vboNodes->bind();
      _vboNodes->draw(GL_POINTS);
      _vboNodes->unbind();
    }
    _nodeShader->disable();
  }
  _modelMatrixStack.pop();


  // LABEL NODES
  for (unsigned i = 0; i != cluster->get_node_num(); ++i)
  {
    Node* current_node = cluster->get_node(i);

    gloost::Vector3 text_position(current_node->_x, current_node->_y, 0.0);

    // Scale Cluster to window size
    gloost::Matrix model_matrix;
    float scale = _height/(cluster->get_radius() * 2);

    model_matrix.setScale(gloost::Vector3(scale, scale,0.0));

    text_position = model_matrix * text_position;

    // Translate from original cluster position to 0,0
    text_position[0] -= cluster->get_position_x() * scale;
    text_position[1] -= cluster->get_position_y() * scale;

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

      /// _typewriter functions
      /*
      // cuts a line, so it is not longer as maxPixelLength with the current font
      std::string cutLineToLength(const std::string& text, unsigned maxPixelLength) const;

      // sets the scaling
      void setScale(float scale);
      */

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
} // namespace vta

