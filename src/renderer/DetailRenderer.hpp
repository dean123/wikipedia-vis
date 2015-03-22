#ifndef VTA_DETAIL_RENDERER
#define VTA_DETAIL_RENDERER

// gloost
#include <gloost/gl/ShaderProgram.h>
#include <gloost/gl/UniformSet.h>
#include <gloost/gl/Texture.h>
#include <gloost/gl/Vbo4.h>

#include <gloost/TextureManager.h>
#include <gloost/FreeTypeWriter.h>
#include <gloost/Matrix.h>
#include <gloost/MatrixStack.h>

#include <gloost/human_input/MouseState.h>

// cpp
#include <memory>
#include <vector>

// gl
#include <GL/glew.h>
#include <GL/gl.h>

// Graph
#include <cluster/Edge.hpp>
// Cluster
#include <cluster/Cluster.hpp>


namespace vta
{

  class DetailRenderer
  {

    public:

      // class constructor
      DetailRenderer(Visualization*);

      // class destructor
      virtual ~DetailRenderer();

      // initialize
      bool initialize();

      // Draw nodes and edges
      void fill_vbo_nodes();
      void fill_vbo_edges();

      // display
      void display();

      // resize
      void resize(int width, int height);


    protected:

      // size
      int _width;
      int _height;

      // Model View Projection
      gloost::Matrix _projectionMatrix;
      gloost::Matrix _viewMatrix;
      gloost::MatrixStack _modelMatrixStack;

      // data
      Visualization* _graph;

      // gl ressources
      gloost::gl::UniformSet _uniformSet;
      std::shared_ptr<gloost::gl::ShaderProgram> _edgeShader;
      std::shared_ptr<gloost::gl::ShaderProgram> _nodeShader;
      std::shared_ptr<gloost::gl::Vbo4> _vboNodes;
      std::shared_ptr<gloost::gl::Vbo4> _vboEdges;

      std::shared_ptr<gloost::FreeTypeWriter> _typeWriter;


  };



} // namespace vta


#endif // VTA_DETAIL_RENDERER
