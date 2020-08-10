#pragma once
#include <variant>

#include "FreeCamera.h"
#include "vendor/imgui/imgui.h"
#include "vendor/imgui/imgui_impl_glfw.h"
#include "vendor/imgui/imgui_impl_opengl3.h"

enum class Key {
    a       = GLFW_KEY_A,
    c       = GLFW_KEY_C,
    d       = GLFW_KEY_D,
    m       = GLFW_KEY_M,
    s       = GLFW_KEY_S,
    w       = GLFW_KEY_W,
    space   = GLFW_KEY_SPACE,
    esc     = GLFW_KEY_ESCAPE,
};

enum class Mouse {
    left    = GLFW_MOUSE_BUTTON_LEFT,
    right   = GLFW_MOUSE_BUTTON_RIGHT,
    middle  = GLFW_MOUSE_BUTTON_MIDDLE,
    none    = 3,
};

enum class InputType {
    release = GLFW_RELEASE,
    press   = GLFW_PRESS,
    scroll  = 2,
    move    = 3,
};

struct InputEvent {
    InputType inputType;
    std::variant<Key, Mouse> inputPressed;
    int mouseX, mouseY;
};

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
    void inputQueuePush(InputEvent event);

    inline GLFWwindow* windowInstance() { return m_window; }
    inline std::vector<InputEvent>& inputQueue() { return m_inputQueue; }
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
    std::vector<InputEvent> m_inputQueue;

    bool initGLFW();
    bool initGL();
    void initImGUI();

};

static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

static const char* inputPressedString(std::variant<Key, Mouse> inputPressed);
static const char* inputTypeString(InputType type);
