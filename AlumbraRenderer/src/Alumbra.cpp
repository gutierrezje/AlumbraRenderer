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

int main() {
	Window window("Alumbra", windowWidth, windowHeight);
	
  Scene scene;
  Renderer renderer(&scene);

	bool wireframe = false;
	
	while (!window.isClosed()) {
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		window.processInput(deltaTime);

		window.clear();
    
#ifdef _DEBUG
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			ImGui::Begin("Debug Settings");
			ImGui::Checkbox("Wireframe", &wireframe);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::Text("Keyboard Controls:");
			ImGui::Text("W/A/S/D - Forward/Left/Back/Right");
			ImGui::Text("L_CTRL/SPACE - Down/Up");
			ImGui::Text("P - Show/Hide Mouse Pointer");
			ImGui::Text("ESC - Exit Program");

			ImGui::End();
		}

		if (wireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#endif //!_DEBUG

    renderer.drawScene();

		window.update();
	}

	return 0;
}


