#pragma once

struct TextureOptions {
    GLint minFilter;
    GLint magFilter;
    GLint wrapS;
    GLint wrapT;
    GLint wrapR;

    TextureOptions() : minFilter(GL_LINEAR), magFilter(GL_LINEAR),
        wrapS(GL_CLAMP_TO_EDGE), wrapT(GL_CLAMP_TO_EDGE), wrapR(GL_CLAMP_TO_EDGE) {}
};

class TextureLoader {
public:
    TextureLoader();
    ~TextureLoader();
    void createNew(GLenum target, const TextureOptions& texOps);
    GLuint emptyTexture(GLenum format, GLsizei width, GLsizei height, GLsizei levels = 1);
    GLuint fileTexture(const std::string& path);
    void bind(int index);
    inline GLuint textureID() const { return m_textureID; }
    inline const std::string& path() { return m_path; }

private:
    GLuint m_textureID;
    std::string m_path;
};
