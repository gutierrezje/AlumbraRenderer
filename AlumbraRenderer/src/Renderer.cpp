#include "pch.h"
#include "Renderer.h"
#include "Window.h"
#include "Buffers.h"
#include <stb_image.h>

Renderer::Renderer(Scene* scene)
    : m_scene(scene)
    , m_pointDepthFBOs(scene->pointLights().size(), 0)
    , m_pointDepthMaps(scene->pointLights().size(), 0)
{
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

#ifdef _DEBUG
    // Enable auto debugging
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(&messageCallback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION,
        0, nullptr, GL_FALSE);
#endif // _DEBUG

    // Setting up a screen quad to render to
    auto size = sizeof(fbQuadPos[0]) * fbQuadPos.size() + sizeof(fbQuadTex[0]) * fbQuadTex.size();
    DataBuffer vbo(size, 6, 2);
    vbo.addVec3s(fbQuadPos.data());
    vbo.addVec2s(fbQuadTex.data());
    VertexArray vao;
    vao.loadBuffer(vbo, 1);
    m_screenQuadVAO = vao.vertexArrayID();

    setupShaders();
    setupFramebuffers();
    setupUniforms();
}

Renderer::~Renderer() {}

void Renderer::setupShaders()
{
    bool success = true;
    success &= m_pbrLightingShader.graphicsShaders({ "src/shaders/pbr_shading.vert", "src/shaders/pbr_shading.frag" });
    success &= m_gBufferShader.graphicsShaders({ "src/shaders/pbr_geometry.vert", "src/shaders/pbr_geometry.frag" });
    success &= m_skyboxShader.graphicsShaders({ "src/shaders/skybox.vert", "src/shaders/skybox.frag" });
    success &= m_cubemapCaptureShader.graphicsShaders({ "src/shaders/cubemap.vert", "src/shaders/cubemap_from_equirect.frag" });
    success &= m_cubemapConvolveShader.graphicsShaders({ "src/shaders/cubemap.vert", "src/shaders/cubemap_convolve_irrad.frag" });
    success &= m_cubemapPrefilterShader.graphicsShaders({ "src/shaders/cubemap.vert", "src/shaders/cubemap_prefilter_spec.frag" });
    success &= m_brdfPrecomputeShader.graphicsShaders({ "src/shaders/screen_quad.vert", "src/shaders/brdf_quad.frag" });
    success &= m_postProcessShader.graphicsShaders({ "src/shaders/screen_quad.vert", "src/shaders/screen_quad.frag" });
    success &= m_directDepthShader.graphicsShaders({"src/shaders/directional_depth_map.vert"});
    success &= m_pointDepthShader.graphicsShaders({
        "src/shaders/point_depth_map.vert",
        "src/shaders/point_depth_map.geom",
        "src/shaders/point_depth_map.frag" });
    success &= m_blurShader.graphicsShaders({ "src/shaders/gaussian_blur.vert", "src/shaders/gaussian_blur.frag" });
    assert(success);
}

void Renderer::setupFramebuffers()
{
    TextureLoader texLoader;
    TextureOptions texOps;
    texOps.minFilter = GL_NEAREST;
    texOps.magFilter = GL_NEAREST;

    texLoader.createNew(GL_TEXTURE_2D, texOps);
    GLuint sceneBuffer = texLoader.emptyTexture(GL_RGBA16F, Window::width(), Window::height());
    texLoader.createNew(GL_TEXTURE_2D, texOps);
    GLuint blurBuffer = texLoader.emptyTexture(GL_RGBA16F, Window::width(), Window::height());
    m_mainBuffer.attachColorBuffers({ sceneBuffer, blurBuffer });
    m_mainBuffer.attachRenderbuffer(Window::width(), Window::height());
    
    // Setup GBuffer
    texOps.wrapS = GL_REPEAT;
    texOps.wrapT = GL_REPEAT;
    texLoader.createNew(GL_TEXTURE_2D, texOps);
    GLuint gPosition = texLoader.emptyTexture(GL_RGB16F, Window::width(), Window::height());
    texLoader.createNew(GL_TEXTURE_2D, texOps);
    GLuint gNormal = texLoader.emptyTexture(GL_RGB16F, Window::width(), Window::height());
    texLoader.createNew(GL_TEXTURE_2D, texOps);
    GLuint gAlbedo = texLoader.emptyTexture(GL_SRGB8_ALPHA8, Window::width(), Window::height());
    texLoader.createNew(GL_TEXTURE_2D, texOps);
    GLuint gMetalRoughAO = texLoader.emptyTexture(GL_RGB16F, Window::width(), Window::height());
    m_gBuffer.attachColorBuffers({ gPosition, gNormal, gAlbedo, gMetalRoughAO });
    m_gBuffer.attachRenderbuffer(Window::width(), Window::height());
    
    // Setup IBL environment map
    m_captureBuffer.attachRenderbuffer(2048, 2048);
    auto& sceneCubemap = m_scene->cubemap();
    sceneCubemap.captureEnvironment(m_captureBuffer, m_cubemapCaptureShader);
    sceneCubemap.irradianceConvolution(m_captureBuffer, m_cubemapConvolveShader);
    sceneCubemap.specularPrefilter(m_captureBuffer, m_cubemapPrefilterShader);

    // Precompute BRDF Integral
    texOps.minFilter = GL_LINEAR;
    texOps.magFilter = GL_LINEAR;
    texOps.wrapS = GL_CLAMP_TO_EDGE;
    texOps.wrapT = GL_CLAMP_TO_EDGE;
    texLoader.createNew(GL_TEXTURE_2D, texOps);
    m_brdfLUT = texLoader.emptyTexture(GL_RG16F, 512, 512);
    m_captureBuffer.attachColorBuffers({ m_brdfLUT });
    m_captureBuffer.resizeRB(512, 512);
    m_captureBuffer.bindAs(GL_FRAMEBUFFER);
    glViewport(0, 0, 512, 512);
    m_brdfPrecomputeShader.use();
    m_captureBuffer.clear();
    glBindVertexArray(m_screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Setup Ping Pong Framebuffers
    texLoader.createNew(GL_TEXTURE_2D, texOps);
    GLuint pingTexture = texLoader.emptyTexture(GL_RGBA16F, Window::width(), Window::height());
    m_pingBuffer.attachColorBuffers({ pingTexture });

    texLoader.createNew(GL_TEXTURE_2D, texOps);
    GLuint pongTexture = texLoader.emptyTexture(GL_RGBA16F, Window::width(), Window::height());;
    m_pongBuffer.attachColorBuffers({ pongTexture });

    // Setup Directional Depth Framebuffer
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
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Directional Depth Framebuffer not complete!\n";

    // Setup Point Depth Framebuffers
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

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Point Depth Framebuffer not complete!\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::setupUniforms()
{
    m_pbrLightingShader.use();
    m_pbrLightingShader.setSampler("gPosition",           0);
    m_pbrLightingShader.setSampler("gNormal",             1);
    m_pbrLightingShader.setSampler("gAlbedo",             2);
    m_pbrLightingShader.setSampler("gMetalRoughAO",       3);
    m_pbrLightingShader.setSampler("directionalDepthMap", 4);
    for (int i = 0; i < m_pointDepthMaps.size(); i++) {
        m_pbrLightingShader.setSampler("pointDepthMaps[" + std::to_string(i) + "]", 5 + i);
    }
    m_pbrLightingShader.setSampler("irradianceMap",   5 + m_pointDepthMaps.size());
    m_pbrLightingShader.setSampler("prefilterMap",    5 + m_pointDepthMaps.size() + 1);
    m_pbrLightingShader.setSampler("brdfLUT",         5 + m_pointDepthMaps.size() + 2);

    m_blurShader.use();
    m_blurShader.setSampler("image", 0);

    m_postProcessShader.use();
    m_postProcessShader.setSampler("sceneTexture", 0);
    m_postProcessShader.setSampler("bloomTexture", 1);

    // Setting up lights UBO
    auto& lights = m_scene->pointLights();
    if (lights.size() != 0) {
        GLuint lightsUBO;
        glCreateBuffers(1, &lightsUBO);
        glNamedBufferStorage(lightsUBO, lights.size() * sizeof(struct PointLight), nullptr, GL_DYNAMIC_STORAGE_BIT);
        glBindBufferBase(GL_UNIFORM_BUFFER, 2, lightsUBO);
        glNamedBufferSubData(lightsUBO, 0, lights.size() * sizeof(struct PointLight), lights.data());
    }
}

void Renderer::beginDraw()
{
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_FRAMEBUFFER_SRGB);

    // Directional light depth pass
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, m_directionalDepthFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    const auto& directLight = m_scene->directionalLight();
    
    float nearPlane = 1.0f, farPlane = m_scene->directionalLight().farPlane;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
    glm::mat4 lightView = glm::lookAt(
        -directLight.direction,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    m_directDepthShader.use();
    m_directDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

    int nrRows = 7;
    int nrColumns = 7;
    float spacing = 2.5;

    const auto& models = m_scene->models();
    const auto& transforms = m_scene->transforms();
    /*
    for (int i = 0; i < models.size(); i++) {
        glm::mat4 model = glm::mat4(1.0f);
        for (int row = 0; row < nrRows; ++row) {
            for (int col = 0; col < nrColumns; ++col) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(
                    (col - (nrColumns / 2)) * spacing,
                    (row - (nrRows / 2)) * spacing,
                    0.0f
                ));
                m_directDepthShader.setMat4("model", model);
                models[i]->draw(m_directDepthShader);
            }
        }
        //glm::mat4 modelM = glm::mat4(1.0f);
        //modelM = glm::scale(modelM, transforms[i].scale);
        //modelM = glm::translate(modelM, transforms[i].translate);
        //m_directDepthShader.setMat4("model", modelM);
        //models[i]->draw(m_directDepthShader);
    } */
    
    // Point lights depth pass
    float near = 1.0f, far = 25.0f;
    float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);
    const auto& lights = m_scene->pointLights();
    m_pointDepthShader.use();
    m_pointDepthShader.setFloat("farPlane", far);
    auto lightIndex = 0;
    std::vector<glm::mat4> shadowTransforms;
    shadowTransforms.reserve(6);
    /*for (const auto& light : lights) {
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
            glm::mat4 model = glm::mat4(1.0f);
            for (int row = 0; row < nrRows; ++row) {
                for (int col = 0; col < nrColumns; ++col) {
                    model = glm::mat4(1.0f);
                    model = glm::translate(model, glm::vec3(
                        (col - (nrColumns / 2)) * spacing,
                        (row - (nrRows / 2)) * spacing,
                        0.0f
                    ));
                    m_pointDepthShader.setMat4("model", model);
                    models[i]->draw(m_pointDepthShader);
                }
            }
            //glm::mat4 modelM = glm::mat4(1.0f);
            //modelM = glm::scale(modelM, transforms[i].scale);
            //modelM = glm::translate(modelM, transforms[i].translate);
            //m_pointDepthShader.setMat4("model", modelM);
            //models[i]->draw(m_pointDepthShader);
        }
    } */

    // Geometry Pass
    m_gBuffer.bindAs(GL_FRAMEBUFFER);
    glClearColor(0.0, 0.0, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, Window::width(), Window::height());
    m_gBufferShader.use();

    glm::mat4 projectionM = glm::perspective(glm::radians(g_camera.zoom()),
        (float)Window::width() / (float)Window::height(), 0.1f, 100.0f);
    glm::mat4 viewM = g_camera.getViewMatrix();
    m_gBufferShader.setMat4("projection", projectionM);
    m_gBufferShader.setMat4("view", viewM);

    m_gBufferShader.setVec3("albedo", m_albedo);

    for (auto i = 0; i < models.size(); i++) {
        glm::mat4 model = glm::mat4(1.0f);
        m_gBufferShader.setFloat("roughness", m_roughness);
        m_gBufferShader.setFloat("metallic", m_metallic);
        m_gBufferShader.setMat4("model", model);
        models[i]->draw(m_gBufferShader);
    }

    // Deferred shading pass
    m_mainBuffer.bindAs(GL_FRAMEBUFFER);
    m_mainBuffer.clear();

    // Lighting
    m_pbrLightingShader.use();
    m_pbrLightingShader.setVec3("directLight.direction", directLight.direction);
    m_pbrLightingShader.setVec3("directLight.color", directLight.color);
    m_pbrLightingShader.setFloat("directLight.intensity", directLight.intensity);
    m_pbrLightingShader.setInt("numPointLights", lights.size());
    
    m_pbrLightingShader.setVec3("viewPos", g_camera.position());
    m_pbrLightingShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    m_pbrLightingShader.setFloat("farPlane", far);

    for (auto i = 0; i < m_gBuffer.colorBuffers().size(); i++) {
        glBindTextureUnit(i, m_gBuffer.colorBuffer(i));
    }
    glBindTextureUnit(4, m_directionalDepthMap);
    for (auto i = 0; i < m_pointDepthMaps.size(); i++) {
        glBindTextureUnit(i + 5, m_pointDepthMaps[i]);
    }
    const auto& sceneCubemap = m_scene->cubemap();
    glBindTextureUnit(5 + m_pointDepthMaps.size(), sceneCubemap.irradianceMap());
    glBindTextureUnit(5 + m_pointDepthMaps.size() + 1, sceneCubemap.prefilterMap());
    glBindTextureUnit(5 + m_pointDepthMaps.size() + 2, m_brdfLUT);

    glBindVertexArray(m_screenQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_gBuffer.bindAs(GL_READ_FRAMEBUFFER);
    m_mainBuffer.bindAs(GL_DRAW_FRAMEBUFFER);
    glBlitFramebuffer(0, 0, Window::width(), Window::height(),
        0, 0, Window::width(), Window::height(), GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    m_mainBuffer.bindAs(GL_FRAMEBUFFER);
    // Draw cubemap
    glm::mat4 cmView = glm::mat4(glm::mat3(viewM));
    m_skyboxShader.use();
    m_skyboxShader.setMat4("projection", projectionM);
    m_skyboxShader.setMat4("view", cmView);
    m_skyboxShader.setSampler("skybox", 0);
    glBindTextureUnit(0, sceneCubemap.environmentMap());
    m_scene->cubemap().draw(m_skyboxShader);

    glDisable(GL_DEPTH_TEST);
    bool horizontal = true;
    int amount = 10;
    GLuint currentBuffer = m_mainBuffer.colorBuffer(1);
    m_pongBuffer.bindAs(GL_FRAMEBUFFER);
    m_blurShader.use();
    for (auto i = 0; i < amount; i++) {
        m_blurShader.setBool("horizontal", horizontal);
        glBindTextureUnit(0, currentBuffer);
        glBindVertexArray(m_screenQuadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        horizontal = !horizontal;
        if (horizontal) {
            m_pongBuffer.bindAs(GL_FRAMEBUFFER);
            currentBuffer = m_pingBuffer.colorBuffer(0);
        }
        else {
            m_pingBuffer.bindAs(GL_FRAMEBUFFER);
            currentBuffer = m_pongBuffer.colorBuffer(0);
        }
    }

    // Now rendering to default buffer with post processing
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glClear(GL_COLOR_BUFFER_BIT);
    m_postProcessShader.use();
    glBindVertexArray(m_screenQuadVAO);
    glBindTextureUnit(0, m_mainBuffer.colorBuffer(0));
    if (horizontal)
        glBindTextureUnit(1, m_pingBuffer.colorBuffer(0));
    else
        glBindTextureUnit(1, m_pongBuffer.colorBuffer(0));
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
        ImGui::Text("C/SPACE - Down/Up");
        ImGui::Text("P - Show/Hide Mouse Pointer");
        ImGui::Text("ESC - Exit Program");

        ImGui::SliderFloat("- Exposure", &m_exposure, 0.01f, 5.0f);
        ImGui::Text("Material");
        ImGui::SliderFloat("- Metallic", &m_metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("- Roughness", &m_roughness, 0.0f, 1.0f);
        static int matOption = 0;
        if (ImGui::RadioButton("Gold", &matOption, 0))
            m_albedo = glm::vec3(1.0f, 0.782f, 0.344f);
        ImGui::SameLine();
        if (ImGui::RadioButton("Silver", &matOption, 1))
            m_albedo = glm::vec3(0.97f, 0.96f, 0.915f);
        ImGui::SameLine();
        if (ImGui::RadioButton("Copper", &matOption, 2))
            m_albedo = glm::vec3(0.955f, 0.637f, 0.538f);
        if (ImGui::RadioButton("Zinc", &matOption, 3))
            m_albedo = glm::vec3(0.664f, 0.824f, 0.85f);
        ImGui::SameLine();
        if (ImGui::RadioButton("Titanium", &matOption, 4))
            m_albedo = glm::vec3(0.542f, 0.497f, 0.449f);
        //ImGui::SliderFloat("- DirLightFar", &(m_scene->directionalLight().farPlane), 5.0f, 25.0f);
        ImGui::SliderFloat3("- DirLightVec", glm::value_ptr(m_scene->directionalLight().direction), -10.0f, 10.0f);

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
