#pragma once

class Buffer
{
public:
	Buffer(float* data, int amount, int componentCount);
	~Buffer();

	inline unsigned int componentCount() const { return m_componentCount; }
	inline unsigned int bufferID() const { return m_bufferID; }

private:
	unsigned int m_bufferID;
	int m_componentCount;
};

class VertexArray
{
public:
	VertexArray(Buffer& buffer);
	~VertexArray();
	void bind();
	inline unsigned int vertexArrayID() const { return m_vertexArrayID; }

private:
	unsigned int m_vertexArrayID;
	const int texDataSize = 2;
	const int otherDataSize = 3;
	const int texIndex = 2;
};