#pragma once

class Buffer {
public:
    Buffer(int bufferSize, int vertexCount, int numComponents);
    ~Buffer();
    void addData(const float* data, int dataSize);

    inline unsigned int bufferSize() const { return m_bufferSize; }
    inline int numComponents() const { return m_numComponents; }
    inline unsigned int vertexCount() const { return m_vertexCount; }
    inline unsigned int bufferID() const { return m_bufferID; }

private:
    unsigned int m_bufferID, m_vertexCount, m_bufferSize, m_dataOffset;
    int m_numComponents;
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();
    void useBuffer(const Buffer& buffer);
    void bind();
    inline unsigned int vertexArrayID() const { return m_vertexArrayID; }

private:
    unsigned int m_vertexArrayID;
    const int texDataSize = 2;
    const int otherDataSize = 3;
    const int texIndex = 2;
};

class IndexBuffer {
public:
    IndexBuffer();
    ~IndexBuffer();
    void addIndices(float* idcs);

private:
};

class Framebuffer {
public:
    Framebuffer();
    ~Framebuffer();
    void bind();
};
