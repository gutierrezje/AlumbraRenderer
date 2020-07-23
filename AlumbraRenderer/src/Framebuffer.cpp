#include "pch.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "Window.h"

Framebuffer::Framebuffer()
{
    glCreateFramebuffers(1, &m_framebufferID);
}

Framebuffer::~Framebuffer() {}

void Framebuffer::attachColorBuffers(std::vector<GLuint>&& buffers)
{
    m_colorBuffers = buffers;
    std::vector<GLenum> attachments;
    for (int i = 0; i < buffers.size(); i++) {
        glNamedFramebufferTexture(m_framebufferID, GL_COLOR_ATTACHMENT0 + i, buffers[i], 0);
        attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
    if (attachments.size() > 1) {
        bindAs(GL_FRAMEBUFFER);
        glDrawBuffers(attachments.size(), attachments.data());
        unbind();
    }
}

void Framebuffer::attachRenderbuffer(int width, int height) {
    glCreateRenderbuffers(1, &m_renderbufferID);
    glNamedRenderbufferStorage(m_renderbufferID, GL_DEPTH_COMPONENT24,
        width, height);
    glNamedFramebufferRenderbuffer(m_framebufferID, GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER, m_renderbufferID);
}

void Framebuffer::bindAs(GLenum fbType) const
{
    glBindFramebuffer(fbType, m_framebufferID);
}

void Framebuffer::unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Framebuffer::isComplete()
{
    bindAs(GL_FRAMEBUFFER);
    bool complete = true;
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        complete = false;
        std::cout << "ERROR::FRAMEBUFFER:: \
            Framebuffer is not complete" << std::endl;
    }
    unbind();
    return complete;
}

void Framebuffer::clear() const
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::resizeRB(int width, int height) const
{
    glNamedRenderbufferStorage(m_renderbufferID, GL_DEPTH_COMPONENT24, width, height);
}

void Framebuffer::bindTexture(unsigned int index)
{
    glBindTextureUnit(0, m_colorBuffers[index]);
}