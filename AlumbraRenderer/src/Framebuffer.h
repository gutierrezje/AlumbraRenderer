#pragma once

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();
    void attachColorBuffers(std::vector<GLuint>&& buffers);
    void attachRenderbuffer(int width, int height);
    void bindAs(GLenum fbType);
    void unbind();
    bool isComplete();

    void clear();
    void resizeRB(int width, int height);
    void bindTexture(unsigned int index);

    inline GLuint id() { return m_framebufferID; }
    inline const std::vector<GLuint>& colorBuffers() { return m_colorBuffers; }
    inline GLuint colorBuffer(unsigned int index) { return m_colorBuffers[index]; }
private:
    GLuint m_framebufferID;
    std::vector<GLuint> m_colorBuffers;
    GLuint m_renderbufferID;
    int m_width, m_height;
};


