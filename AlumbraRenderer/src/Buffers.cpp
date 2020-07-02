#include "pch.h"
#include "Buffers.h"
#include "Framebuffer.h"
#include "Window.h"

DataBuffer::DataBuffer(int bufferSize, int vertexCount, int numComponents, int indexCount)
{
    m_numComponents = numComponents;
    m_vertexCount = vertexCount;
    m_indexCount = indexCount;

    glCreateBuffers(1, &m_bufferID);
    glNamedBufferStorage(m_bufferID, bufferSize, nullptr, GL_DYNAMIC_STORAGE_BIT);
}

DataBuffer::~DataBuffer() {}

void DataBuffer::addIndices(const unsigned int* idcs)
{
    if (idcs == nullptr) {
        return;
    }
    auto dataSize = sizeof(unsigned int) * m_indexCount;
    glNamedBufferSubData(m_bufferID, m_dataOffset, dataSize, idcs);
    m_indexEnd = dataSize;
}

void DataBuffer::addVec3s(const glm::vec3* data)
{
    auto dataSize = sizeof(data[0]) * m_vertexCount;

    glNamedBufferSubData(m_bufferID, m_indexEnd + m_dataOffset, dataSize, data);
    m_dataOffset += dataSize;
}

void DataBuffer::addVec2s(const glm::vec2* data)
{
    auto dataSize = sizeof(data[0]) * m_vertexCount;

    glNamedBufferSubData(m_bufferID, m_indexEnd + m_dataOffset, dataSize, data);
    m_dataOffset += dataSize;
}

VertexArray::VertexArray()
{
    glCreateVertexArrays(1, &m_vertexArrayID);
}

VertexArray::~VertexArray() {}

void VertexArray::loadBuffer(const DataBuffer& buffer, int texIndex)
{
    // Set elements/indices buffer location
    glVertexArrayElementBuffer(m_vertexArrayID, buffer.dataBufferID());
    // Always set binding point for position data
    glVertexArrayVertexBuffer(m_vertexArrayID, 0, buffer.dataBufferID(), buffer.indexEnd(), sizeof(float) * 3);

    auto bufferOffset = sizeof(float) * 3 * buffer.vertexCount();
    // If just position and texCoord data, only need 1 more binding point
    if (texIndex == 1) {
        glVertexArrayVertexBuffer(m_vertexArrayID, 1, buffer.dataBufferID(), 
            buffer.indexEnd() + texIndex * bufferOffset, sizeof(float) * 2);
    }
    // Else make as many binding points as we need
    else {
        for (int i = 1; i < buffer.numComponents(); i++) {
            int dataPerVertex = i == texIndex ? 2 : 3;
            glVertexArrayVertexBuffer(m_vertexArrayID, i, buffer.dataBufferID(),
                buffer.indexEnd() + bufferOffset, sizeof(float) * dataPerVertex);
            bufferOffset += sizeof(float) * dataPerVertex * buffer.vertexCount();
        }
    }

    int dataPerVertex = 3, bindingindex = 0;
    for (int attribindex = 0; attribindex < buffer.numComponents(); attribindex++) {
        if (attribindex == texIndex) {
            dataPerVertex = 2;
        }
        bindingindex = attribindex;
        glEnableVertexArrayAttrib(m_vertexArrayID, attribindex);
        glVertexArrayAttribFormat(m_vertexArrayID, attribindex, dataPerVertex, GL_FLOAT,
            GL_FALSE, 0);
        glVertexArrayAttribBinding(m_vertexArrayID, attribindex, bindingindex);
    }
}

void VertexArray::bind()
{
    glBindVertexArray(m_vertexArrayID);
}
