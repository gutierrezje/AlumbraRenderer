#include "pch.h"
#include "Window.h"

float lastX, lastY;
bool firstMouse = true;

int Window::s_width, Window::s_height;
bool Window::s_cursorHidden;

Window::Window(const char* title, int width, int height)
{
	m_title = title;
	s_width = width;
	s_height = height;
  s_cursorHidden = true;

  lastX = (float)s_width / 2.0f;
  lastY = (float)s_height / 2.0f;

  if (!init()) {
    glfwDestroyWindow(m_window);
    glfwTerminate();
  }
}

Window::~Window()
{
  ImGui_ImplGlfw_Shutdown();
  ImGui_ImplOpenGL3_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(m_window);
  glfwTerminate();
  
}

bool Window::init()
{
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // glfw window creation
  // --------------------
  m_window = glfwCreateWindow(s_width, s_height, m_title, NULL, NULL);
  if (m_window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    return false;
  }
  glfwMakeContextCurrent(m_window);
  glfwSetWindowUserPointer(m_window, static_cast<void*>(this));
  glfwSwapInterval(0);
  //Setup callbacks
  glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
  glfwSetCursorPosCallback(      m_window, mouseCallback);
  glfwSetScrollCallback(         m_window, scrollCallback);
  glfwSetKeyCallback(            m_window, keyCallback);

  // tell GLFW to capture our mouse
  glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return false;
  }

#ifdef _DEBUG
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(&messageCallback, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION,
    0, nullptr, GL_FALSE);
#endif // _DEBUG


  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOpenGL(m_window, false);
  ImGui_ImplOpenGL3_Init("#version 450");
  ImGui::StyleColorsDark();
}

void Window::update()
{
  // Handle Window updating
  glfwSwapBuffers(m_window);
  glfwPollEvents();
}

bool Window::isClosed()
{
  return glfwWindowShouldClose(m_window);
}

void Window::clear()
{
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Window::processInput(float deltaTime)
{
  if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(m_window, true);

  if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
    g_camera.processKeyboard(FORWARD, deltaTime);
  if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
    g_camera.processKeyboard(BACKWARD, deltaTime);
  if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
    g_camera.processKeyboard(LEFT, deltaTime);
  if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
    g_camera.processKeyboard(RIGHT, deltaTime);
  if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS)
    g_camera.processKeyboard(UP, deltaTime);
  if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    g_camera.processKeyboard(DOWN, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
static void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and 
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
static void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
  // Calculate mouse position even if movement disabled to prevent jumping around
  if (firstMouse)
  {
    lastX = (float)xpos;
    lastY = (float)ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
  // Disable camera movement while cursor is showing
  if (!win->cursorHidden())
    return;

  g_camera.processMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  g_camera.processMouseScroll(yoffset);
  ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
  ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

  if (key == GLFW_KEY_P && action == GLFW_RELEASE) {
    win->setCursorHidden(!win->cursorHidden());
    GLenum cursorOption = win->cursorHidden() ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
    glfwSetInputMode(win->windowInstance(), GLFW_CURSOR, cursorOption);
  }
}

void messageCallback(GLenum source,
  GLenum type,
  GLuint id,
  GLenum severity,
  GLsizei length,
  const GLchar* message,
  const void* userParam)
{
  auto const src_str = [source]() {
		switch (source)
		{
		case GL_DEBUG_SOURCE_API: return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
		case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
		case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
		case GL_DEBUG_SOURCE_OTHER: return "OTHER";
		}
	}();

	auto const type_str = [type]() {
		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR: return "ERROR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
		case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
		case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
		case GL_DEBUG_TYPE_MARKER: return "MARKER";
		case GL_DEBUG_TYPE_OTHER: return "OTHER";
		}
	}();

	auto const severity_str = [severity]() {
		switch (severity) {
		case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
		case GL_DEBUG_SEVERITY_LOW: return "LOW";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
		case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
		}
	}();

  std::cout << src_str << ", " << type_str << ", " << severity_str << ", " 
    << id << ": " << message << '\n';
}
