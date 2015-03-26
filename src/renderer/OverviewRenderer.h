#ifndef H_VTA_OVERVIEW_RENDERER
#define H_VTA_OVERVIEW_RENDERER

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
#include <cluster/Visualization.hpp>


namespace vta
{


class OverviewRenderer
{

  public:

    // class constructor
    OverviewRenderer(Visualization*);

    // class destructor
    virtual ~OverviewRenderer();


    // initialize
    bool initialize();

    void fill_vbos();

    // Create a new vbo for a cluster
    void create_cluster_node_vbo(Cluster*);
    void create_cluster_edge_vbo(Cluster*);

    // Fill vbo's with nodes and edges
    void fill_vbo_nodes(std::vector<ArticleNode*> const&);
    void fill_vbo_edges(std::vector<ArticleEdge*> const&);

    // Draw nodes and edges (node-color, edge-color)
    void draw_nodes_and_edges(gloost::vec4, gloost::vec4);

    // display loop
    void display();

    // Display node labels of detailed cluster
    void display_node_labels();

    // resize window
    void resize(int width, int height);

    // Get Cluster index from mouse position
    unsigned get_cluster_index_from_mouse_pos(gloost::Vector3);

    // Hightlight nodes and all corresponding edges
    void highlight_node(ArticleNode* current_node);

    // mouse input
    void mousePress(int x, int y, int btn, int mods);
    void mouseRelease(int x, int y, int btn, int mods);
    void mouseMove(int x, int y);
    void mouseScrollEnhance();
    void mouseScrollDecrease();

    // Imgui callbacks
    void imgui_mouse_released();

    // key input
    void keyPress(int key, int mods);
    void keyRelease(int key, int mods);

    // Similarity thresholds
    void set_minimum_similarity(double);
    void set_maximum_similarity(double);

    // Blacklist
    void add_to_blacklist(std::string const);
    bool blacklist_constains(std::string const);
    std::vector<std::string> get_blacklist();

    // Display next cluster
    void get_next_cluster(unsigned);


  protected:

    // Window size
    int _width;
    int _height;

    // holding the current mouse state
    gloost::human_input::MouseState _mouseState;

    // MVP
    gloost::Matrix _projectionMatrix;
    gloost::Matrix _viewMatrix;
    gloost::MatrixStack _modelMatrixStack;

    // Panning and Zooming
    gloost::Vector3 _translateVector;
    gloost::Vector3 _oldTranslateVector;
    gloost::Vector3 _scaleVector;
    gloost::Vector3 _oldScaleVector;

    // Mouse Position for scaling
    gloost::Point3 _mouse_pos;

    // Transformation Matrix for detail view
    gloost::MatrixStack _zoomingPanningStack;

    // data
    Visualization* _graph;

    // Similarity thresholds
    double _max_similarity;
    double _min_similarity;

    // Blacklist
    std::vector<std::string> _blacklist;

    // gl ressources
    gloost::gl::UniformSet _uniformSet;
    std::shared_ptr<gloost::gl::ShaderProgram> _edgeShader;
    std::shared_ptr<gloost::gl::ShaderProgram> _nodeShader;

    // vbo for all nodes and edges
    std::shared_ptr<gloost::gl::Vbo4> _vboNodes;
    std::shared_ptr<gloost::gl::Vbo4> _vboEdges;

    // typewriter for text rendering
    std::shared_ptr<gloost::FreeTypeWriter> _typeWriter;

    // Toggle if nodes should be highlighted at mouse over
    bool _highlight_mode;

    // Vector with all cluster vbos
    std::vector < std::shared_ptr<gloost::gl::Vbo4> > _cluster_node_vbo;
    std::vector < std::shared_ptr<gloost::gl::Vbo4> > _cluster_edge_vbo;

    // Number of clusters on screen
    unsigned _num_clusters_on_screen;

    // Toggle if a cluster is currently at detail view
    bool _cluster_detail_view;

    // Number of nodes and edges
    unsigned _num_nodes;
    unsigned _num_edges;

};


} // namespace vta


#endif // H_VTA_OVERVIEW_RENDERER


