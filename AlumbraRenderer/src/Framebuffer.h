#pragma once

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();
    void bind();
    void unbind();
    void clear();
    void attachTexture();
    void attachRenderbuffer();
    void bindTexture(unsigned int index);

    inline GLuint id() { return m_framebufferID; }
    inline GLuint colorBuffer(unsigned int index) { return m_colorBuffers[index]; }
private:
    GLuint m_framebufferID;
    std::vector<GLuint> m_colorBuffers;
    GLuint m_renderbufferID;
};


