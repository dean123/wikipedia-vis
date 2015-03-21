// vta
#include <Visualization.h>
#include <cluster/ClusterVis.hpp>

// gl
#include <GL/glew.h>

// glfw
#include <GLFW/glfw3.h>

// imgui
#include <imgui/imgui.h>
//#include <imgui/imgui.cpp>
#include <imgui/imgui_impl_glfw.h>

// cpp includes
#include <string>
#include <iostream>
#include <math.h>
#include <stdlib.h>

// Graph for tests
#include <graph/Graph.hpp>


vta::Visualization* g_visualization;
vta::ClusterVis* cluster_visualization;

// GRAPH
vta::Graph* graph;


/////////////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Calls the display function of the application
  \remarks ...
*/

void main_window_display()
{
  g_visualization->display();
}

void cluster_window_display()
{
  cluster_visualization->display();
}


/////////////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Resize callback
  \param   width
  \param   height
  \remarks ...
*/

void main_window_resizefun(GLFWwindow* window, int width, int height)
{
  g_visualization->resize(width, height);
}

void cluster_window_resizefun(GLFWwindow* window, int width, int height)
{
  cluster_visualization->resize(width, height);
}


/////////////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Mouse button callback
  \param   window	 The window that received the event.
  \param   button	 The mouse button that was pressed or released.
  \param   action	 One of GLFW_PRESS or GLFW_RELEASE.
  \param   mods	   Bit field describing which modifier keys were held down.
  \remarks ...
*/

void main_window_mousebuttonfun(GLFWwindow* window, int button, int action, int mods)
{
  double xpos = 0;
  double ypos = 0;

  // retrieve latest cursor position
  glfwGetCursorPos(window, &xpos, &ypos);

  if (action)
    g_visualization->mousePress(xpos, ypos, button, mods);
  else
    g_visualization->mouseRelease(xpos, ypos, button, mods);
}

/////////////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Mouse scroll callback
  \param   window	 The window that received the event.
  \param   xoffset ?
  \param   yoffset Scroll wheel offset
  \remarks ...
*/

void main_window_scrollfun(GLFWwindow* window, double xoffset, double yoffset)
{
  if (yoffset > 0)
    g_visualization->mouseScrollEnhance();
  else
    g_visualization->mouseScrollDecrease();
}


/////////////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Mouse position callback
  \param   window	 The window that received the event.
  \param   xpos	   The new x-coordinate, in screen coordinates, of the cursor.
  \param   ypos	   The new y-coordinate, in screen coordinates, of the cursor.
  \remarks ...
*/

void main_window_cursorposfun(GLFWwindow* window, double xpos, double ypos)
{
  g_visualization->mouseMove(xpos, ypos);
}


/////////////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Keyboard callback
  \param   window	  The window that received the event.
  \param   key	    The keyboard key that was pressed or released.
  \param   scancode	The system-specific scancode of the key.
  \param   action	  GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
  \param   mods     Bit field describing which modifier keys were held down.
  \remarks ...
*/

void main_window_keyfun(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  // recognize press of escape key and close application
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
  {
    delete g_visualization;

    glfwSetWindowShouldClose(window, GL_TRUE);
    return;
  }

  // default key function
  if (action == GLFW_PRESS)
    g_visualization->keyPress(key, mods);
  else // GLFW_RELEASE
    g_visualization->keyRelease(key, mods);
}


/////////////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Error callback
  \param   error
  \param   description
  \remarks ...
*/

void glfw_errorfun(int error, const char* description)
{
  fputs(description, stderr);
}


/////////////////////////////////////////////////////////////////////////////////////////


int main(int argc, char *argv[])
{
  // Graph init
  graph = new vta::Graph();

  //TODO create DB from tsv files

  graph->create_graph_from_db("/media/HDD/RAM_CORPUS/SUBSETS/subset_sim_900_999.dat", "/media/HDD/RAM_CORPUS/SUBSETS/offset_subset_sim_900_999.dat");

  // GLFW
  glfwSetErrorCallback(glfw_errorfun);

  if (!glfwInit())
      exit(1);

  // new cluster visualization instance
  cluster_visualization = new vta::ClusterVis(graph);

  if (!cluster_visualization->initialize())
  {
    std::cerr << "error initializing cluster vis" << std::endl;
    exit(EXIT_FAILURE);
  }


  // GLFW cluster window init
  GLFWwindow* cluster_window = glfwCreateWindow(1920, 1080, "Cluster Window", NULL, NULL);

  if (!cluster_window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(cluster_window);
  glfwSetFramebufferSizeCallback(cluster_window, cluster_window_resizefun);

  int cluster_window_width = 0;
  int cluster_window_height = 0;

  glfwGetFramebufferSize(cluster_window, &cluster_window_width, &cluster_window_height);

  cluster_visualization->resize(cluster_window_width, cluster_window_height); // initial resize

  // GLFW main window init
  GLFWwindow* main_window = glfwCreateWindow(1920, 1080, "VisualTextAnalytics", NULL, NULL);

  if (!main_window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(main_window);
  glfwSetKeyCallback(main_window, main_window_keyfun);
  glfwSetCursorPosCallback(main_window, main_window_cursorposfun);
  glfwSetMouseButtonCallback(main_window, main_window_mousebuttonfun);
  glfwSetFramebufferSizeCallback(main_window, main_window_resizefun);
  glfwSetScrollCallback(main_window, main_window_scrollfun);

  // new visualization instance
  g_visualization = new vta::Visualization(graph);

  if (!g_visualization->initialize())
  {
    std::cerr << "error initializing main vis" << std::endl;
    exit(EXIT_FAILURE);
  }

  int main_window_width = 0;
  int main_window_height = 0;

  glfwGetFramebufferSize(main_window, &main_window_width, &main_window_height);

  g_visualization->resize(main_window_width, main_window_height); // initial resize

  if (GLEW_OK != glewInit())
	{
	  std::cerr << "'glewInit()' failed." << std::endl;
	  exit(EXIT_FAILURE);
  }

  // Setup ImGui binding
  GLFWwindow* imgui_window = glfwCreateWindow(800, 600, "Imgui", NULL, NULL);
  glfwMakeContextCurrent(imgui_window);

  ImGui_ImplGlfw_Init(imgui_window, true);
  ImGui_ImplGlfw_InitFontsTexture();

  bool show_test_window = true;
  bool show_another_window = false;
  ImVec4 clear_color = ImColor(114, 144, 154);

  // Similairty thresholds
  float min_similarity = 0.82;
  float max_similarity = 1.02;

  // Blacklist
  char* buf = new char [256];

  // Set Cluster Visualization in main vis

  // Main loop
  while (!glfwWindowShouldClose(imgui_window))
  {
    glfwPollEvents();

    // Main Window (Visualization)
    glfwMakeContextCurrent(main_window);
    main_window_display(); // display function
    glfwSwapBuffers(main_window);

    // Cluster Window (/Visualization)
    glfwMakeContextCurrent(cluster_window);
    cluster_window_display(); // display function
    glfwSwapBuffers(cluster_window);

    // Imgui Window (Interface)
    glfwMakeContextCurrent(imgui_window);

    ImGui_ImplGlfw_NewFrame();

    {
      ImGui::Begin("Minimum Similarity");
      ImGui::SliderFloat("sim", &min_similarity, 0.0f, 1.0f);
      ImGui::End();
    }

    {
      ImGui::Begin("Maximum Similarity");
      ImGui::SliderFloat("sim", &max_similarity, 0.0f, 1.0f);
      ImGui::End();
    }

    {
      ImGui::Begin("Blacklist");
      ImGui::InputText("string", buf, 256);

      if (ImGui::Button("add"))
      {
        g_visualization->add_to_blacklist(std::string (buf));
      }

      std::vector<std::string> current_blacklist = g_visualization->get_blacklist();
      for (unsigned i = 0; i != current_blacklist.size(); ++i)
      {
        std::string title = current_blacklist[i];
        const char *cstr = title.c_str();

        ImGui::Text(cstr);

//        delete [] cstr;
      }

      ImGui::End();
    }

    g_visualization->set_minimum_similarity(min_similarity);
    g_visualization->set_maximum_similarity(max_similarity);

    // Rendering
    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    glfwSwapBuffers(imgui_window);
  }

  // Cleanup
  ImGui_ImplGlfw_Shutdown();
  glfwDestroyWindow(main_window);
  glfwTerminate();

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
