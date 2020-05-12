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

void DataBuffer::addData(const float* data, int dataPerVertex)
{
    auto dataSize = dataPerVertex * sizeof(float) * m_vertexCount;

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
    glVertexArrayElementBuffer(m_vertexArrayID, buffer.dataBufferID());
    auto fsize = sizeof(float);
    glVertexArrayVertexBuffer(m_vertexArrayID, 0, buffer.dataBufferID(), 
        buffer.indexEnd(), fsize * 3);
    // If just position and texCoord data, only need 1 more binding point
    if (texIndex == 1) {
        glVertexArrayVertexBuffer(m_vertexArrayID, 1, buffer.dataBufferID(),
            buffer.indexEnd() + fsize * texIndex * 3l * buffer.vertexCount(),
            fsize * 2);
    }
    // Else make as many binding points as we need
    if (texIndex > 1) {
        // TODO: support for 4+ binding points
        glVertexArrayVertexBuffer(m_vertexArrayID, 1, buffer.dataBufferID(),
            buffer.indexEnd() + fsize * 3l * buffer.vertexCount(),
            fsize * 3);
        glVertexArrayVertexBuffer(m_vertexArrayID, 2, buffer.dataBufferID(),
            buffer.indexEnd() + fsize * texIndex * 3l * buffer.vertexCount(),
            fsize * 2);
    }

    int dataPerVertex = 3, bindingindex = 0;
    //long vaOffset, totalOffset = 0;
    for (int attribindex = 0; attribindex < buffer.numComponents(); attribindex++) {
        //vaOffset = totalOffset;
        if (attribindex == texIndex) {
            dataPerVertex = 2;
        }
        bindingindex = attribindex;
        glEnableVertexArrayAttrib(m_vertexArrayID, attribindex);
        glVertexArrayAttribFormat(m_vertexArrayID, attribindex, dataPerVertex, GL_FLOAT,
            GL_FALSE, 0);
        glVertexArrayAttribBinding(m_vertexArrayID, attribindex, bindingindex);
        //totalOffset += (long)(dataPerVertex * buffer.vertexCount() * sizeof(float));
    }
}

void VertexArray::bind()
{
    glBindVertexArray(m_vertexArrayID);
}
