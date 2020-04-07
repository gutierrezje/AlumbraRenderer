#include "pch.h"
#include "Shader.h"

// constructor generates the shader on the fly
// ------------------------------------------------------------------------
Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	// 1. retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		// read file's buffer contents into streams
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		// close file handlers
		vShaderFile.close();
		fShaderFile.close();
		// convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	// 2. compile shaders
	unsigned int vertex, fragment;
	// vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);
	checkCompileErrors(vertex, "VERTEX");
	// fragment Shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);
	checkCompileErrors(fragment, "FRAGMENT");
	// shader Program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");
	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

// activate the shader
	// ------------------------------------------------------------------------
void Shader::use() const
{
	glUseProgram(ID);
}

unsigned int Shader::getUniformLocation(std::string name) const
{
	auto keyval = m_uniformLocationCache.find(name);
	if (keyval != m_uniformLocationCache.end())
		return keyval->second;

	unsigned int location = glGetUniformLocation(ID, name.c_str());
	auto ret = m_uniformLocationCache.insert(std::pair<std::string, unsigned int>(name, location));
	return ret.first->second;
}

// utility uniform functions
	// ------------------------------------------------------------------------
void Shader::setBool(std::string name, bool value) const
{
	glUniform1i(getUniformLocation(name), (int)value);
}

// ------------------------------------------------------------------------
void Shader::setInt(std::string name, int value) const
{
	glUniform1i(getUniformLocation(name), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(std::string name, float value) const
{
	glUniform1f(getUniformLocation(name), value);
}
// ------------------------------------------------------------------------
void Shader::setSampler(std::string name, int sampler) const
{
	glProgramUniform1i(ID, getUniformLocation(name), sampler);
}
// ------------------------------------------------------------------------
void Shader::setVec2(std::string name, const glm::vec2& value) const
{
	glUniform2fv(getUniformLocation(name), 1, &value[0]);
}
void Shader::setVec2(std::string name, float x, float y) const
{
	glUniform2f(getUniformLocation(name), x, y);
}
// ------------------------------------------------------------------------
void Shader::setVec3(std::string name, const glm::vec3& value) const
{
	glUniform3fv(getUniformLocation(name), 1, &value[0]);
}
void Shader::setVec3(std::string name, float x, float y, float z) const
{
	glUniform3f(getUniformLocation(name), x, y, z);
}
// ------------------------------------------------------------------------
void Shader::setVec4(std::string name, const glm::vec4& value) const
{
	glUniform4fv(getUniformLocation(name), 1, &value[0]);
}
void Shader::setVec4(std::string name, float x, float y, float z, float w) const
{
	glUniform4f(getUniformLocation(name), x, y, z, w);
}
// ------------------------------------------------------------------------
void Shader::setMat2(std::string name, const glm::mat2& mat) const
{
	glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat3(std::string name, const glm::mat3& mat) const
{
	glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat4(std::string name, const glm::mat4& mat) const
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
void Shader::checkCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog 
				<< "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog 
				<< "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}