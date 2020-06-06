#include "pch.h"
#include "FreeCamera.h"
#include "Window.h"
#include "Scene.h"
#include "Renderer.h"

#include <stb_image.h>

const int windowWidth = 1920;
const int windowHeight = 1080;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

FreeCamera g_camera(glm::vec3(0.0f, 1.5f, 4.0f));

int main()
{
    Window window("Alumbra", windowWidth, windowHeight);
    Scene scene;
    Renderer renderer(&scene);

    while (!window.isClosed()) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        window.processInput(deltaTime);

        window.clear();

        renderer.beginDraw();

        window.update();
    }

    return 0;
}


