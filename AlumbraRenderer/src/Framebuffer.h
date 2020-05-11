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
    void bindTexture();

    inline unsigned int textureID() { return m_textureID; }
private:
    unsigned int m_framebufferID;
    unsigned int m_textureID;
    unsigned int m_renderbufferID;
};


