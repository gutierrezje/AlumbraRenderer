#pragma once

class Texture {
public:
	Texture();
	~Texture();
	void loadTexture(std::string path, bool gamma = false);
	void bind(int index);
	inline unsigned int textureID() { return m_textureID; }
	inline std::string path() { return m_path; }

private:
	unsigned int m_textureID;
	std::string m_path;
};
