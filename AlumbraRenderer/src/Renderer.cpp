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
    , m_depthShader(Shader("src/shaders/simpleDepthShader.vert", "src/shaders/simpleDepthShader.frag"))
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
    glDepthFunc(GL_LESS);
    glFrontFace(GL_CCW);

    // Setting up lights UBO
    auto& lights = m_scene->pointLights();
    GLuint lightsUBO;
    glCreateBuffers(1, &lightsUBO);
    glNamedBufferStorage(lightsUBO, lights.size() * sizeof(struct PointLight),
        nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, lightsUBO);
    glNamedBufferSubData(lightsUBO, 0, lights.size() * sizeof(struct PointLight),
        lights.data());

    
    // Setup shadow map + framebuffer
    glCreateTextures(GL_TEXTURE_2D, 1, &m_depthMap);
    glTextureStorage2D(m_depthMap, 1, GL_DEPTH_COMPONENT24,
        SHADOW_WIDTH, SHADOW_HEIGHT);
    glTextureParameteri(m_depthMap, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_depthMap, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_depthMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(m_depthMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[]{1.0f, 1.0f, 1.0f, 1.0f};
    glTextureParameterfv(m_depthMap, GL_TEXTURE_BORDER_COLOR, borderColor);
    glCreateFramebuffers(1, &m_depthMapFBO);
    glNamedFramebufferTexture(m_depthMapFBO, GL_DEPTH_ATTACHMENT,
        m_depthMap, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: \
            Framebuffer is not complete" << std::endl;

    // Setting up a quad to render to
    int size = sizeof(fbQuadPos) + sizeof(fbQuadTex);
    DataBuffer vbo(size, 6, 2);
    vbo.addData(fbQuadPos, 3);
    vbo.addData(fbQuadTex, 2);
    VertexArray vao;
    vao.loadBuffer(vbo, 1);
    m_fbQuadVAO = vao.vertexArrayID();
    m_fbShader.use();
    m_fbShader.setSampler("screenTexture", 0);

    m_modelShader.setSampler("shadowMap", 2);
}

void Renderer::beginDraw()
{
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);

    // Shadow depth pass
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    //glCullFace(GL_FRONT);

    auto& directLight = m_scene->directionalLight();
    float nearPlane = 1.0f, farPlane = 7.5f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
    glm::mat4 lightView = glm::lookAt(
        -directLight.direction,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    m_depthShader.use();
    m_depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    const auto& models = m_scene->models();
    const auto& transforms = m_scene->transforms();
    for (int i = 0; i < models.size(); i++) {
        glm::mat4 modelM = glm::mat4(1.0f);
        modelM = glm::scale(modelM, transforms[i].scale);
        modelM = glm::translate(modelM, transforms[i].translate);
        m_depthShader.setMat4("model", modelM);
        models[i]->draw(m_depthShader);
    }

    // Draw depth map to a quad
    /*
    glDisable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    m_fbShader.use();
    m_fbShader.setFloat("nearPlane", nearPlane);
    m_fbShader.setFloat("farPlane", farPlane);
    glBindVertexArray(m_fbQuadVAO);
    glBindTextureUnit(0, m_depthMap);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    //*/

    // Forward pass
    ///*
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glViewport(0, 0, Window::width(), Window::height());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    m_modelShader.use();

    // Lighting
    m_modelShader.setVec3("directLight.direction", directLight.direction);
    m_modelShader.setVec3("directLight.color", directLight.color);
    m_modelShader.setFloat("directLight.intensity", directLight.intensity);
    m_modelShader.setInt("numPointLights", m_scene->pointLights().size());
    
    m_modelShader.setVec3("viewPos", g_camera.position());
    m_modelShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    glBindTextureUnit(2, m_depthMap);

    glm::mat4 projectionM = glm::perspective(glm::radians(g_camera.zoom()),
        (float)Window::width() / (float)Window::height(), 0.1f, 100.0f);
    glm::mat4 viewM = g_camera.getViewMatrix();
    m_modelShader.setMat4("projection", projectionM);
    m_modelShader.setMat4("view", viewM);

    // Draw models
    m_modelShader.setFloat("material.shininess", 32.0f);
    for (int i = 0; i < models.size(); i++) {
        glm::mat4 modelM = glm::mat4(1.0f);
        modelM = glm::scale(modelM, transforms[i].scale);
        modelM = glm::translate(modelM, transforms[i].translate);

        m_modelShader.setMat4("model", modelM);
        models[i]->draw(m_modelShader);
    }
    
    // Draw cubemap
    glm::mat4 cmView = glm::mat4(glm::mat3(viewM));
    m_cubemapShader.use();
    m_cubemapShader.setMat4("projection", projectionM);
    m_cubemapShader.setMat4("view", cmView);
    m_scene->cubemap().draw(m_cubemapShader);
    //*/
    //drawGUI();
    
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
