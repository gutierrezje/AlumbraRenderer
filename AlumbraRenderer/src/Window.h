#pragma once

#include "FreeCamera.h"

#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_glfw.h"
#include "vendor/imgui/imgui_impl_opengl3.h"

/**
 * This class is responsible for handling the application window using GLFW 
 */
class Window {
public:
	Window(const char* title, int width, int height);
	~Window();

	void update();
	bool isClosed();
	void clear();
	void bind();
	void processInput(float deltaTime);

	inline GLFWwindow* windowInstance() { return m_window; }
	static inline int width() { return s_width; }
	static inline void setWidth(int width) { s_width = width; }
	static inline int height() { return s_height; }
	static inline void setHeight(int height) { s_height = height; }
	static inline bool cursorHidden() { return s_cursorHidden; }
	static inline void setCursorHidden(bool mode) { s_cursorHidden = mode; }

private:
	const char* m_title;
	GLFWwindow* m_window;
	static int s_width, s_height;
	static bool s_cursorHidden;

	bool init();
	
};

static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void messageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam);
