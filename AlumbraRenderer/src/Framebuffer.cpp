#include "pch.h"
#include "Framebuffer.h"
#include "Window.h"

Framebuffer::Framebuffer() : m_colorBuffers(2, 0)
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
    glCreateTextures(GL_TEXTURE_2D, 2, m_colorBuffers.data());
    for (int i = 0; i < m_colorBuffers.size(); i++) {
        glTextureStorage2D(m_colorBuffers[i], 1, GL_RGBA16F,
            Window::width(), Window::height());
        glTextureParameteri(m_colorBuffers[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(m_colorBuffers[i], GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(m_colorBuffers[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_colorBuffers[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glNamedFramebufferTexture(m_framebufferID, GL_COLOR_ATTACHMENT0 + i,
            m_colorBuffers[i], 0);
    }
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferID);
    glDrawBuffers(2, attachments);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Currently attaches a depth + stencil renderbuffer
void Framebuffer::attachRenderbuffer() {
    glCreateRenderbuffers(1, &m_renderbufferID);
    glNamedRenderbufferStorage(m_renderbufferID, GL_DEPTH24_STENCIL8,
        Window::width(), Window::height());
    glNamedFramebufferRenderbuffer(m_framebufferID, GL_DEPTH_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER, m_renderbufferID);
}

void Framebuffer::bindTexture(unsigned int index)
{
    glBindTextureUnit(0, m_colorBuffers[index]);
}