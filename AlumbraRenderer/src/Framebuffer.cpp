#include "pch.h"
#include "Framebuffer.h"
#include "Window.h"

Framebuffer::Framebuffer()
{
    glCreateFramebuffers(1, &m_framebufferID);
    attachTexture();
    attachRenderbuffer();
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: \
            Framebuffer is not complete" << std::endl;
}

Framebuffer::~Framebuffer() {}

void Framebuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
}

void Framebuffer::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::clear()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Currently attaches a colorbuffer texture
void Framebuffer::attachTexture() {
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);
    glTextureStorage2D(m_textureID, 1, GL_RGBA8,
        Window::width(), Window::height());
    glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glNamedFramebufferTexture(m_framebufferID, GL_COLOR_ATTACHMENT0,
        m_textureID, 0);
    //glGenerateTextureMipmap(m_textureID);
}

// Currently attaches a depth + stencil renderbuffer
void Framebuffer::attachRenderbuffer() {
    glCreateRenderbuffers(1, &m_renderbufferID);
    glNamedRenderbufferStorage(m_renderbufferID, GL_DEPTH24_STENCIL8,
        Window::width(), Window::height());
    glNamedFramebufferRenderbuffer(m_framebufferID, GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER, m_renderbufferID);
}

void Framebuffer::bindTexture()
{
    glBindTextureUnit(0, m_textureID);
}