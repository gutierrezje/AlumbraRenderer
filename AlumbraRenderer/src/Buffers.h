#pragma once

class DataBuffer {
public:
    DataBuffer(int bufferSize, int vertexCount, int numComponents, int indexCount = 0);
    ~DataBuffer();
    void addIndices(const unsigned int* idcs);
    void addVec3s(const glm::vec3* data);
    void addVec2s(const glm::vec2* data);

    //inline unsigned int bufferSize() const { return m_bufferSize; }
    inline int numComponents() const { return m_numComponents; }
    inline int vertexCount() const { return m_vertexCount; }
    inline unsigned int dataBufferID() const { return m_bufferID; }
    inline int indexEnd() const { return m_indexEnd; }

private:
    unsigned int m_bufferID;
    int m_numComponents, m_vertexCount, m_indexCount;
    long long m_indexEnd = 0ll;
    long long m_dataOffset = 0ll;
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();
    void loadBuffer(const DataBuffer& buffer, int texIndex);
    void bind();
    inline unsigned int vertexArrayID() const { return m_vertexArrayID; }

private:
    unsigned int m_vertexArrayID;
};
