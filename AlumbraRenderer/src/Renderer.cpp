#include "pch.h"
#include "Renderer.h"
#include "Window.h"
#include "Buffers.h"
#include <stb_image.h>

Renderer::Renderer(Scene* scene)
    : m_scene(scene)
    , m_fbo()
    , m_pointDepthFBOs(scene->pointLights().size(), 0)
    , m_pointDepthMaps(scene->pointLights().size(), 0)
{
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);

#ifdef _DEBUG
    // Enable auto debugging
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(&messageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION,
        0, nullptr, GL_FALSE);
#endif // _DEBUG

    setupShaders();
    setupFramebuffers();
    setupUBOs();
}

Renderer::~Renderer() {}

void Renderer::setupShaders()
{
    m_modelShader.addShaders({ "src/shaders/deferred_shading.vert", "src/shaders/deferred_shading.frag" });
    m_gBufferShader.addShaders({ "src/shaders/deferred_geometry.vert", "src/shaders/deferred_geometry.frag" });
    m_skyboxShader.addShaders({ "src/shaders/skybox.vert", "src/shaders/skybox.frag" });
    m_postProcessShader.addShaders({ "src/shaders/screen_quad.vert", "src/shaders/screen_quad.frag" });
    m_directDepthShader.addShaders({"src/shaders/directional_depth_map.vert"});
    m_pointDepthShader.addShaders({
        "src/shaders/point_depth_map.vert",
        "src/shaders/point_depth_map.geom",
        "src/shaders/point_depth_map.frag" });
    m_blurShader.addShaders({ "src/shaders/gaussian_blur.vert", "src/shaders/gaussian_blur.frag" });
}

void Renderer::setupFramebuffers()
{
    // Setup GBuffer
    glCreateTextures(GL_TEXTURE_2D, 1, &m_gPosition);
    glTextureStorage2D(m_gPosition, 1, GL_RGBA16F, Window::width(), Window::height());
    glTextureParameteri(m_gPosition, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_gPosition, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_gNormal);
    glTextureStorage2D(m_gNormal, 1, GL_RGBA16F, Window::width(), Window::height());
    glTextureParameteri(m_gNormal, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_gNormal, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_gAlbedoSpec);
    glTextureStorage2D(m_gAlbedoSpec, 1, GL_SRGB8_ALPHA8, Window::width(), Window::height());
    glTextureParameteri(m_gAlbedoSpec, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_gAlbedoSpec, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glCreateRenderbuffers(1, &m_gDepthMap);
    glNamedRenderbufferStorage(m_gDepthMap, GL_DEPTH24_STENCIL8, Window::width(), Window::height());

    // Add attachments to GBuffer
    glCreateFramebuffers(1, &m_gBufferFBO);
    glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT0, m_gPosition, 0);
    glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT1, m_gNormal, 0);
    glNamedFramebufferTexture(m_gBufferFBO, GL_COLOR_ATTACHMENT2, m_gAlbedoSpec, 0);
    glNamedFramebufferRenderbuffer(m_gBufferFBO, GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER, m_gDepthMap);
    // Setting multiple render targets
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO);
    GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!\n";

    // TODO: Unify these setup routines under the Framebuffer class
    glCreateFramebuffers(2, m_pingPongFBOs);
    glCreateTextures(GL_TEXTURE_2D, 2, m_pingPongBuffers);
    for (int i = 0; i < 2; i++) {
        glTextureStorage2D(m_pingPongBuffers[i], 1, GL_RGBA16F, Window::width(), Window::height());
        glTextureParameteri(m_pingPongBuffers[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_pingPongBuffers[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTextureParameteri(m_pingPongBuffers[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_pingPongBuffers[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glNamedFramebufferTexture(m_pingPongFBOs[i], GL_COLOR_ATTACHMENT0, m_pingPongBuffers[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!\n";
    }
    // Setup shadow map + framebuffer
    glCreateTextures(GL_TEXTURE_2D, 1, &m_directionalDepthMap);
    glTextureStorage2D(m_directionalDepthMap, 1, GL_DEPTH_COMPONENT24, SHADOW_WIDTH, SHADOW_HEIGHT);
    glTextureParameteri(m_directionalDepthMap, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_directionalDepthMap, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(m_directionalDepthMap, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTextureParameteri(m_directionalDepthMap, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[]{1.0f, 1.0f, 1.0f, 1.0f};
    glTextureParameterfv(m_directionalDepthMap, GL_TEXTURE_BORDER_COLOR, borderColor);

    glCreateFramebuffers(1, &m_directionalDepthFBO);
    glNamedFramebufferTexture(m_directionalDepthFBO, GL_DEPTH_ATTACHMENT, m_directionalDepthMap, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, m_directionalDepthFBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_modelShader.use();

    m_modelShader.setSampler("gPosition", 0);
    m_modelShader.setSampler("gNormal", 1);
    m_modelShader.setSampler("gAlbedoSpec", 2);
    m_modelShader.setSampler("directionalDepthMap", 3);

    // Setup shadow cubemap
    glCreateFramebuffers(m_pointDepthFBOs.size(), m_pointDepthFBOs.data());
    glCreateTextures(GL_TEXTURE_CUBE_MAP, m_pointDepthMaps.size(), m_pointDepthMaps.data());
    for (int i = 0; i < m_pointDepthMaps.size(); i++) {
        glTextureStorage2D(m_pointDepthMaps[i], 1, GL_DEPTH_COMPONENT24, SHADOW_WIDTH, SHADOW_HEIGHT);
        glTextureParameteri(m_pointDepthMaps[i], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(m_pointDepthMaps[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(m_pointDepthMaps[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_pointDepthMaps[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_pointDepthMaps[i], GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glNamedFramebufferTexture(m_pointDepthFBOs[i], GL_DEPTH_ATTACHMENT, m_pointDepthMaps[i], 0);
        glBindFramebuffer(GL_FRAMEBUFFER, m_pointDepthFBOs[i]);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);

        m_modelShader.setSampler("pointDepthMaps[" + std::to_string(i) + "]", 4 + i);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_blurShader.use();
    m_blurShader.setSampler("image", 0);

    m_postProcessShader.use();
    m_postProcessShader.setSampler("sceneTexture", 0);
    m_postProcessShader.setSampler("bloomTexture", 1);
    
    // Setting up a screen quad to render to
    auto size = sizeof(fbQuadPos[0]) * fbQuadPos.size() + sizeof(fbQuadTex[0])  * fbQuadTex.size();
    DataBuffer vbo(size, 6, 2);
    vbo.addVec3s(fbQuadPos.data());
    vbo.addVec2s(fbQuadTex.data());
    VertexArray vao;
    vao.loadBuffer(vbo, 1);
    m_screenQuadVAO = vao.vertexArrayID();
}

void Renderer::setupUBOs()
{
    // Setting up lights UBO
    auto& lights = m_scene->pointLights();
    GLuint lightsUBO;
    glCreateBuffers(1, &lightsUBO);
    glNamedBufferStorage(lightsUBO, lights.size() * sizeof(struct PointLight), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, lightsUBO);
    glNamedBufferSubData(lightsUBO, 0, lights.size() * sizeof(struct PointLight), lights.data());
}

void Renderer::beginDraw()
{
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);
    //glCullFace(GL_FRONT);

    // Directional light depth pass
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, m_directionalDepthFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    const auto& directLight = m_scene->directionalLight();
    
    float nearPlane = 1.0f, farPlane = 7.5f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
    glm::mat4 lightView = glm::lookAt(
        -directLight.direction,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    m_directDepthShader.use();
    m_directDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    const auto& models = m_scene->models();
    const auto& transforms = m_scene->transforms();
    for (int i = 0; i < models.size(); i++) {
        glm::mat4 modelM = glm::mat4(1.0f);
        modelM = glm::scale(modelM, transforms[i].scale);
        modelM = glm::translate(modelM, transforms[i].translate);
        m_directDepthShader.setMat4("model", modelM);
        models[i]->draw(m_directDepthShader);
    }
    
    // Point lights depth pass
    float near = 1.0f, far = 25.0f;
    float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
    const auto& lights = m_scene->pointLights();
    m_pointDepthShader.use();
    m_pointDepthShader.setFloat("farPlane", far);
    // TODO: Point shadow mapping is currently a huge performance hit, do better
    auto lightIndex = 0;
    std::vector<glm::mat4> shadowTransforms;// (6, glm::mat4(0));
    shadowTransforms.reserve(6);
    for (const auto& light : lights) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_pointDepthFBOs[lightIndex++]);
        glClear(GL_DEPTH_BUFFER_BIT);
        auto lightPos = light.position.xyz();
        shadowTransforms.emplace_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.emplace_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.emplace_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
        shadowTransforms.emplace_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
        shadowTransforms.emplace_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
        shadowTransforms.emplace_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));

        m_pointDepthShader.setVec3("lightPos", lightPos);
        for (int i = 0; i < 6; i++) {
            m_pointDepthShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        }

        shadowTransforms.clear();
        shadowTransforms.reserve(6);

        for (int i = 0; i < models.size(); i++) {
            glm::mat4 modelM = glm::mat4(1.0f);
            modelM = glm::scale(modelM, transforms[i].scale);
            modelM = glm::translate(modelM, transforms[i].translate);
            m_pointDepthShader.setMat4("model", modelM);
            models[i]->draw(m_pointDepthShader);
        }
    }

    // Geometry Pass
    glBindFramebuffer(GL_FRAMEBUFFER, m_gBufferFBO);
    glClearColor(0.0, 0.0, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, Window::width(), Window::height());
    glDepthFunc(GL_LESS);
    m_gBufferShader.use();

    glm::mat4 projectionM = glm::perspective(glm::radians(g_camera.zoom()),
        (float)Window::width() / (float)Window::height(), 0.1f, 100.0f);
    glm::mat4 viewM = g_camera.getViewMatrix();
    m_gBufferShader.setMat4("projection", projectionM);
    m_gBufferShader.setMat4("view", viewM);

    for (int i = 0; i < models.size(); i++) {
        glm::mat4 modelM = glm::mat4(1.0f);
        modelM = glm::scale(modelM, transforms[i].scale);
        modelM = glm::translate(modelM, transforms[i].translate);

        m_gBufferShader.setMat4("model", modelM);
        models[i]->draw(m_gBufferShader);
    }

    // Deferred shading pass
    m_fbo.bind();
    m_fbo.clear();
    m_modelShader.use();

    // Lighting
    m_modelShader.setVec3("directLight.direction", directLight.direction);
    m_modelShader.setVec3("directLight.color", directLight.color);
    m_modelShader.setFloat("directLight.intensity", directLight.intensity);
    m_modelShader.setInt("numPointLights", lights.size());
    
    m_modelShader.setVec3("viewPos", g_camera.position());
    m_modelShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    m_modelShader.setFloat("farPlane", far);
    glBindTextureUnit(0, m_gPosition);
    glBindTextureUnit(1, m_gNormal);
    glBindTextureUnit(2, m_gAlbedoSpec);
    glBindTextureUnit(3, m_directionalDepthMap);
    for (int i = 0; i < m_pointDepthMaps.size(); i++) {
        glBindTextureUnit(4 + i, m_pointDepthMaps[i]);
    }

    glBindVertexArray(m_screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gBufferFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo.id());
    glBlitFramebuffer(0, 0, Window::width(), Window::height(),
        0, 0, Window::width(), Window::height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    m_fbo.bind();
    // Draw cubemap
    glm::mat4 cmView = glm::mat4(glm::mat3(viewM));
    m_skyboxShader.use();
    m_skyboxShader.setMat4("projection", projectionM);
    m_skyboxShader.setMat4("view", cmView);
    m_scene->cubemap().draw(m_skyboxShader);

    glDisable(GL_DEPTH_TEST);
    bool horizontal = true, firstIteration = true;
    int amount = 10;
    m_blurShader.use();
    for (int i = 0; i < amount; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, m_pingPongFBOs[horizontal]);
        m_blurShader.setInt("horizontal", horizontal);
        auto currentBuffer = firstIteration ? m_fbo.colorBuffer(1) : m_pingPongBuffers[!horizontal];
        // Render to a buffer quad
        glBindTextureUnit(0, currentBuffer);
        glBindVertexArray(m_screenQuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        horizontal = !horizontal;
        if (firstIteration)
            firstIteration = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glClear(GL_COLOR_BUFFER_BIT);
    m_postProcessShader.use();
    glBindVertexArray(m_screenQuadVAO);
    glBindTextureUnit(0, m_fbo.colorBuffer(0));
    glBindTextureUnit(1, m_pingPongBuffers[!horizontal]);
    m_postProcessShader.setFloat("exposure", m_exposure);
    m_postProcessShader.setBool("bloom", true);
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
        ImGui::Begin("Info");
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
            ImGui::GetIO().Framerate);
        ImGui::Text("Keyboard Controls:");
        ImGui::Text("W/A/S/D - Forward/Left/Back/Right");
        ImGui::Text("L_ALT/SPACE - Down/Up");
        ImGui::Text("P - Show/Hide Mouse Pointer");
        ImGui::Text("ESC - Exit Program");

        ImGui::SliderFloat("- Exposure", &m_exposure, 0.01f, 5.0f);

        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

/* Credit to https://github.com/fendevel/Guide-to-Modern-OpenGL-Functions */
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
