#pragma once

#include <unordered_map>

using TypedShader = std::pair<GLenum, std::string>;

class Shader
{
public:
    int ID;

    Shader() = default;

    void graphicsShaders(const std::vector<std::string>& shaderFiles);
    void compileProgram(const std::vector<TypedShader>& shaders);

    void use() const;
    unsigned int getUniformLocation(const std::string& name) const;
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setSampler(const std::string& name, int sampler) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    mutable std::unordered_map<std::string, unsigned int> m_uniformLocationCache;
    void checkCompileErrors(GLuint shader, std::string type);
};