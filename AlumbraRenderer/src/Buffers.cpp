#include "pch.h"
#include "Buffers.h"
#include "Framebuffer.h"
#include "Window.h"

VertexBuffer::VertexBuffer(int bufferSize, int vertexCount, int numComponents)
{
    m_bufferSize = bufferSize;
    m_numComponents = numComponents;
    m_dataOffset = 0;
    m_vertexCount = vertexCount;

    glCreateBuffers(1, &m_bufferID);
    glNamedBufferStorage(m_bufferID, bufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
}

VertexBuffer::~VertexBuffer() {}

void VertexBuffer::addData(const float* data, int dataSize)
{
    glNamedBufferSubData(m_bufferID, m_dataOffset, dataSize, data);
    m_dataOffset += dataSize;
}

VertexArray::VertexArray()
{
    glCreateVertexArrays(1, &m_vertexArrayID);
}

VertexArray::~VertexArray() {}

void VertexArray::loadBuffer(const VertexBuffer& buffer, int texIndex)
{
    int fsize = sizeof(float);
    glVertexArrayVertexBuffer(m_vertexArrayID, 0, buffer.vertexBufferID(), 
        0, fsize * 3);
    if (texIndex >= 0) {
        glVertexArrayVertexBuffer(m_vertexArrayID, 1, buffer.vertexBufferID(),
            fsize * texIndex * 3 * buffer.vertexCount(), fsize * 2);
    }

    int dataSize = 3, bindingindex = 0;
    int vaOffset, totalOffset = 0;
    for (int attribindex = 0; attribindex < buffer.numComponents(); attribindex++) {
        vaOffset = totalOffset;
        if (attribindex == texIndex) {
            dataSize = 2;
            vaOffset = 0;
            bindingindex = 1;
        }
        glEnableVertexArrayAttrib(m_vertexArrayID, attribindex);
        glVertexArrayAttribFormat(m_vertexArrayID, attribindex, dataSize, GL_FLOAT,
            GL_FALSE, vaOffset);
        glVertexArrayAttribBinding(m_vertexArrayID, attribindex, bindingindex);
        totalOffset += dataSize * buffer.vertexCount() * sizeof(float);
    }
}

void VertexArray::bind()
{
    glBindVertexArray(m_vertexArrayID);
}
