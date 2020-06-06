#include "pch.h"
#include "Renderer.h"
#include "Window.h"
#include "Buffers.h"
#include <stb_image.h>

Renderer::Renderer(Scene* sceneView)
    : m_scene(sceneView)
    , m_modelShader(Shader("src/shaders/basic.vert", "src/shaders/basic.frag"))
    , m_cubemapShader(Shader("src/shaders/cubemap.vert", "src/shaders/cubemap.frag"))
    , m_fbShader(Shader("src/shaders/framequad.vert", "src/shaders/framequad.frag"))
    , m_fbo()
{
#ifdef _DEBUG
    // Enable auto debugging
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(&messageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION,
        0, nullptr, GL_FALSE);
#endif // _DEBUG

    init();
}

Renderer::~Renderer() {}

void Renderer::init()
{
    // Setting up lights UBO
    auto& lights = m_scene->pointLights();
    unsigned int lightsUBO;
    glCreateBuffers(1, &lightsUBO);
    glNamedBufferStorage(lightsUBO, lights.size() * sizeof(struct PointLight),
        nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, lightsUBO);
    glNamedBufferSubData(lightsUBO, 0, lights.size() * sizeof(struct PointLight),
        lights.data());

    // Setting up framebuffer quad
    int size = sizeof(fbQuadPos) + sizeof(fbQuadTex);
    DataBuffer vbo(size, 6, 2);
    vbo.addData(fbQuadPos, 3);
    vbo.addData(fbQuadTex, 2);
    VertexArray vao;
    vao.loadBuffer(vbo, 1);
    m_fbQuadVAO = vao.vertexArrayID();
}

void Renderer::beginDraw()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_FRAMEBUFFER_SRGB);

    // Enable drawing to framebuffer
    m_fbo.bind();
    m_fbo.clear();

    glm::mat4 projectionM = glm::perspective(glm::radians(g_camera.zoom()),
        (float)Window::width() / (float)Window::height(), 0.1f, 100.0f);
    glm::mat4 viewM = g_camera.getViewMatrix();

    m_modelShader.use();

    m_modelShader.setInt("numPointLights", m_scene->pointLights().size());
    m_modelShader.setVec3("viewPos", g_camera.position());

    m_modelShader.setFloat("material.shininess", 32.0f);

    m_modelShader.setMat4("projection", projectionM);
    m_modelShader.setMat4("view", viewM);

    // Draw models
    const auto& models = m_scene->models();
    for (auto model : models) {
        glm::mat4 modelM = glm::mat4(1.0f);

        m_modelShader.setMat4("model", modelM);
        model->draw(m_modelShader);
    }

    // Draw cubemap
    glm::mat4 cmView = glm::mat4(glm::mat3(viewM));
    m_cubemapShader.use();
    m_cubemapShader.setMat4("projection", projectionM);
    m_cubemapShader.setMat4("view", cmView);
    m_scene->cubemap().draw(m_cubemapShader);

    m_fbo.unbind();
    glDisable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_fbShader.use();
    glBindVertexArray(m_fbQuadVAO);
    m_fbo.bindTexture();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    drawGUI();
}

void Renderer::drawGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Debug window
    {
        ImGui::Begin("Debug Settings");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Keyboard Controls:");
        ImGui::Text("W/A/S/D - Forward/Left/Back/Right");
        ImGui::Text("L_ALT/SPACE - Down/Up");
        ImGui::Text("P - Show/Hide Mouse Pointer");
        ImGui::Text("ESC - Exit Program");

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
