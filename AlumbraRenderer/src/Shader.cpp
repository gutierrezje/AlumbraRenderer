#include "pch.h"
#include "Shader.h"

void Shader::graphicsShaders(const std::vector<std::string>& shaderFiles)
{
    std::vector<TypedShader> typedShaders;
    for (const auto& shaderFileName : shaderFiles) {
        auto extLoc = shaderFileName.find_last_of('.');

        if (extLoc == std::string::npos) {
            std::cout << "No file extension found\n";
            assert(0);
        }

        GLenum shaderType;
        std::string ext = shaderFileName.substr(extLoc + 1);
        if (ext == "vert")
            shaderType = GL_VERTEX_SHADER;
        else if (ext == "frag")
            shaderType = GL_FRAGMENT_SHADER;
        else if (ext == "geom")
            shaderType = GL_GEOMETRY_SHADER;
        else {
            std::cout << "No valid shader type found.\n";
            assert(0);
        }

        typedShaders.push_back(make_pair(shaderType, shaderFileName));
    }

    compileProgram(typedShaders);
}

void Shader::compileProgram(const std::vector<TypedShader>& shaders)
{
    ID = glCreateProgram();
    for (const auto& shader : shaders) {
        // Read in the shader source
        std::string shaderCode;
        std::ifstream shaderFile;
        shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try {
            shaderFile.open(shader.second);
            std::stringstream shaderStream;
            shaderStream << shaderFile.rdbuf();
            shaderFile.close();
            shaderCode = shaderStream.str();
        }
        catch (std::ifstream::failure e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ\n";
        }

        // Compile shader source and attach to program
        auto shaderCodeString = shaderCode.c_str();
        GLuint shaderID = glCreateShader(shader.first);
        glShaderSource(shaderID, 1, &shaderCodeString, NULL);
        glCompileShader(shaderID);
        checkCompileErrors(shaderID, "SHADER");
        glAttachShader(ID, shaderID);
        glDeleteShader(shaderID);
    }

    // Finally link the program
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");
}

// activate the shader
    // ------------------------------------------------------------------------
void Shader::use() const
{
    glUseProgram(ID);
}

unsigned int Shader::getUniformLocation(const std::string& name) const
{
    auto keyval = m_uniformLocationCache.find(name);
    if (keyval != m_uniformLocationCache.end())
        return keyval->second;

    auto location = glGetUniformLocation(ID, name.c_str());
    auto ret = m_uniformLocationCache.insert(make_pair(name, location));
    return ret.first->second;
}

// utility uniform functions
    // ------------------------------------------------------------------------
void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(getUniformLocation(name), (int)value);
}

// ------------------------------------------------------------------------
void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(getUniformLocation(name), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(getUniformLocation(name), value);
}
// ------------------------------------------------------------------------
void Shader::setSampler(const std::string& name, int sampler) const
{
    glProgramUniform1i(ID, getUniformLocation(name), sampler);
}
// ------------------------------------------------------------------------
void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(getUniformLocation(name), 1, &value[0]);
}
void Shader::setVec2(const std::string& name, float x, float y) const
{
    glUniform2f(getUniformLocation(name), x, y);
}
// ------------------------------------------------------------------------
void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(getUniformLocation(name), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, float x, float y, float z) const
{
    glUniform3f(getUniformLocation(name), x, y, z);
}
// ------------------------------------------------------------------------
void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(getUniformLocation(name), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const
{
    glUniform4f(getUniformLocation(name), x, y, z, w);
}
// ------------------------------------------------------------------------
void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
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