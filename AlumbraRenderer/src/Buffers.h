#pragma once

class VertexBuffer {
public:
    VertexBuffer(int bufferSize, int vertexCount, int numComponents);
    ~VertexBuffer();
    void addData(const float* data, int dataSize);

    inline unsigned int bufferSize() const { return m_bufferSize; }
    inline int numComponents() const { return m_numComponents; }
    inline int vertexCount() const { return m_vertexCount; }
    inline unsigned int vertexBufferID() const { return m_bufferID; }

private:
    unsigned int m_bufferID, m_bufferSize, m_dataOffset;
    int m_numComponents, m_vertexCount;
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();
    void loadBuffer(const VertexBuffer& buffer, int texIndex);
    void bind();
    inline unsigned int vertexArrayID() const { return m_vertexArrayID; }

private:
    unsigned int m_vertexArrayID;
};

class IndexBuffer {
public:
    IndexBuffer();
    ~IndexBuffer();
    void addIndices(float* idcs);

private:
};
