#pragma once

// Defines several possible options for camera movement. Used as abstraction
// to stay away from window-system specific input methods
enum Camera_Movement {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  UP,
  DOWN
};

// Default camera values
const float yaw0 = -90.0f;
const float pitch0 = 0.0f;
const float speed0 = 2.5f;
const float sensitivity0 = 0.1f;
const float zoom0 = 45.0f;


// An abstract camera class that processes input and calculates the corresponding
// Euler Angles, Vectors and Matrices for use in OpenGL
class FreeCamera
{
public:
  FreeCamera(
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
    float yaw = yaw0,
    float pitch = pitch0);
  FreeCamera(float posX, float posY, float posZ, float upX, float upY, float upZ,
    float yaw, float pitch);

  glm::mat4 getViewMatrix() const;
  void processKeyboard(Camera_Movement direction, float deltaTime);
  void processMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
  void processMouseScroll(float yoffset);

  inline float zoom() const { return m_zoom; }
  inline float fov() const { return m_fov; }
  inline glm::vec3 position() const { return m_position; }

private:
  void updateCameraVectors();

  // Camera Attributes
  glm::vec3 m_position, m_front, m_up, m_right, m_worldUp;
  
  // Euler Angles
  float m_yaw;
  float m_pitch;
  
  // Camera options
  float m_movementSpeed;
  float m_mouseSensitivity;
  float m_zoom;
  float m_fov;
};

// Defined in main file Alumbra.cpp
extern FreeCamera g_camera;
