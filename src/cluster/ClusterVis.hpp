#ifndef VTA_CLUSTERVIS
#define VTA_CLUSTERVIS

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
#include <graph/Edge.hpp>
// Cluster
#include <cluster/Cluster.hpp>


namespace vta
{

  class ClusterVis
  {

    public:

      // class constructor
      ClusterVis(Graph*);

      // class destructor
      virtual ~ClusterVis();


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
      Graph* _graph;

      // gl ressources
      gloost::gl::UniformSet _uniformSet;
      std::shared_ptr<gloost::gl::ShaderProgram> _edgeShader;
      std::shared_ptr<gloost::gl::ShaderProgram> _nodeShader;
      std::shared_ptr<gloost::gl::Vbo4> _vboNodes;
      std::shared_ptr<gloost::gl::Vbo4> _vboEdges;

      std::shared_ptr<gloost::FreeTypeWriter> _typeWriter;


  };



} // namespace vta


#endif // VTA_CLUSTERVIS
