#include "pch.h"
#include "Buffers.h"

Buffer::Buffer(float* data, int amount, int componentCount)
{
	m_componentCount = componentCount;

	glCreateBuffers(1, &m_bufferID);
	glNamedBufferData(m_bufferID, amount, data, GL_STATIC_DRAW);
}

Buffer::~Buffer()
{
	glInvalidateBufferData(m_bufferID);
}

VertexArray::VertexArray(Buffer& buffer)
{
	glCreateVertexArrays(1, &m_vertexArrayID);

	glVertexArrayVertexBuffer(m_vertexArrayID, 0, buffer.bufferID(), 0,
		sizeof(float) * 8);

	int dataSize, offset = 0;
	for (int index = 0; index < buffer.componentCount(); ++index) {
		dataSize = index == texIndex ? texDataSize : otherDataSize;
		glEnableVertexArrayAttrib(m_vertexArrayID, index);
		glVertexArrayAttribFormat(m_vertexArrayID, index, dataSize, GL_FLOAT, GL_FALSE, offset * sizeof(float));
		glVertexArrayAttribBinding(m_vertexArrayID, index, 0);
		offset += dataSize;
	}
}

VertexArray::~VertexArray()
{
}

void VertexArray::bind()
{
	glBindVertexArray(m_vertexArrayID);
}