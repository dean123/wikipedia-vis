// vta
#include <renderer/OverviewRenderer.h>
#include <renderer/DetailRenderer.hpp>

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
#include <cluster/Visualization.hpp>


vta::OverviewRenderer* overview_renderer;
vta::DetailRenderer* detail_renderer;

// GRAPH
vta::Visualization* graph;


/////////////////////////////////////////////////////////////////////////////////////////

/**
  \brief   Calls the display function of the application
  \remarks ...
*/

void main_window_display()
{
  overview_renderer->display();
}

void detail_window_display()
{
  detail_renderer->display();
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
  overview_renderer->resize(width, height);
}

void detail_window_resizefun(GLFWwindow* window, int width, int height)
{
  detail_renderer->resize(width, height);
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
    overview_renderer->mousePress(xpos, ypos, button, mods);
  else
    overview_renderer->mouseRelease(xpos, ypos, button, mods);
}

void detail_window_mousebuttonfun(GLFWwindow* window, int button, int action, int mods)
{
  double xpos = 0;
  double ypos = 0;

  // retrieve latest cursor position
  glfwGetCursorPos(window, &xpos, &ypos);

  if (action)
    detail_renderer->mousePress(xpos, ypos, button, mods);
  else
    detail_renderer->mouseRelease(xpos, ypos, button, mods);
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
    overview_renderer->mouseScrollEnhance();
  else
    overview_renderer->mouseScrollDecrease();
}

void detail_window_scrollfun(GLFWwindow* window, double xoffset, double yoffset)
{
  if (yoffset > 0)
    detail_renderer->mouseScrollEnhance();
  else
    detail_renderer->mouseScrollDecrease();
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
  overview_renderer->mouseMove(xpos, ypos);
}

void detail_window_cursorposfun(GLFWwindow* window, double xpos, double ypos)
{
  detail_renderer->mouseMove(xpos, ypos);
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
//  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
//  {
//    delete overview_renderer;
//
//    glfwSetWindowShouldClose(window, GL_TRUE);
//    return;
//  }

  // default key function
  if (action == GLFW_PRESS)
    overview_renderer->keyPress(key, mods);
  else // GLFW_RELEASE
    overview_renderer->keyRelease(key, mods);
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
  graph = new vta::Visualization();

  //TODO create DB from tsv files

//  graph->create_graph_from_db("/media/HDD/RAM_CORPUS/SUBSETS/subset_sim_900_999.dat", "/media/HDD/RAM_CORPUS/SUBSETS/offset_subset_sim_900_999.dat");

  graph->get_next_cluster();

  // GLFW
  glfwSetErrorCallback(glfw_errorfun);

  if (!glfwInit())
      exit(1);

  // new detail visualization instance
  detail_renderer = new vta::DetailRenderer(graph);

  if (!detail_renderer->initialize())
  {
    std::cerr << "error initializing cluster vis" << std::endl;
    exit(EXIT_FAILURE);
  }


  // GLFW detail window init
  GLFWwindow* detail_window = glfwCreateWindow(1920, 4320, "Detail Window", NULL, NULL);

  if (!detail_window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(detail_window);
  glfwSetCursorPosCallback(detail_window, detail_window_cursorposfun);
  glfwSetMouseButtonCallback(detail_window, detail_window_mousebuttonfun);
  glfwSetFramebufferSizeCallback(detail_window, detail_window_resizefun);
  glfwSetScrollCallback(detail_window, detail_window_scrollfun);

  int detail_window_width = 0;
  int detail_window_height = 0;

  glfwGetFramebufferSize(detail_window, &detail_window_width, &detail_window_height);

  detail_renderer->resize(detail_window_width, detail_window_height); // initial resize

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
  overview_renderer = new vta::OverviewRenderer(graph);

  if (!overview_renderer->initialize())
  {
    std::cerr << "error initializing main vis" << std::endl;
    exit(EXIT_FAILURE);
  }

  int main_window_width = 0;
  int main_window_height = 0;

  glfwGetFramebufferSize(main_window, &main_window_width, &main_window_height);

  overview_renderer->resize(main_window_width, main_window_height); // initial resize

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
  int* num_of_clusters = new int(1);

  overview_renderer->get_next_cluster(20);

  // Main loop
  while (!glfwWindowShouldClose(imgui_window))
  {
    glfwPollEvents();

    // Main Window (Visualization)
    glfwMakeContextCurrent(main_window);
    main_window_display(); // display function
    glfwSwapBuffers(main_window);

    // detail Window (/Visualization)
    glfwMakeContextCurrent(detail_window);
    detail_window_display(); // display function
    glfwSwapBuffers(detail_window);

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
        overview_renderer->add_to_blacklist(std::string (buf));
      }

      std::vector<std::string> current_blacklist = overview_renderer->get_blacklist();
      for (unsigned i = 0; i != current_blacklist.size(); ++i)
      {
        std::string title = current_blacklist[i];
        const char *cstr = title.c_str();

        ImGui::Text(cstr);

//        delete [] cstr;
      }

      ImGui::End();
    }

    {
      ImGui::Begin("Cluster");
      ImGui::InputInt("Number of Clusters", num_of_clusters, 1, 2);

      if (ImGui::Button("get next cluster"))
      {
        overview_renderer->get_next_cluster(*num_of_clusters);
      }
      ImGui::End();
    }

    overview_renderer->set_minimum_similarity(min_similarity);
    overview_renderer->set_maximum_similarity(max_similarity);

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
