#include "pch.h"
#include "Buffers.h"

Buffer::Buffer(int bufferSize, int vertexCount, int numComponents)
{
    m_bufferSize = bufferSize;
    m_numComponents = numComponents;
    m_dataOffset = 0;
    m_vertexCount = vertexCount;

    glCreateBuffers(1, &m_bufferID);
    glNamedBufferStorage(m_bufferID, bufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
}

Buffer::~Buffer()
{
    glInvalidateBufferData(m_bufferID);
    glDeleteBuffers(1, &m_bufferID);
}

void Buffer::addData(const float* data, int dataSize)
{
    glNamedBufferSubData(m_bufferID, m_dataOffset, dataSize, data);
    m_dataOffset += dataSize;
}

VertexArray::VertexArray()
{
    glCreateVertexArrays(1, &m_vertexArrayID);
}

VertexArray::~VertexArray() {}

void VertexArray::useBuffer(const Buffer& buffer)
{
    glVertexArrayVertexBuffer(m_vertexArrayID, 0, buffer.bufferID(), 0, sizeof(float) * 3);

    int dataSize, offset = 0;
    for (int index = 0; index < buffer.numComponents(); ++index) {
        dataSize = (index == texIndex) ? texDataSize : otherDataSize;
        glEnableVertexArrayAttrib(m_vertexArrayID, index);
        glVertexArrayAttribFormat(m_vertexArrayID, index, dataSize, GL_FLOAT,
            GL_FALSE, offset * sizeof(float));
        glVertexArrayAttribBinding(m_vertexArrayID, index, 0);
        offset += dataSize * buffer.vertexCount();
    }
}

void VertexArray::bind()
{
    glBindVertexArray(m_vertexArrayID);
}