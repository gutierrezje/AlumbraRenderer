#include "../pch.h"
#include "Shapes.h"

Cube::Cube(const std::string& diffTexPath, const std::string& specTexPath, const std::string& normTexPath)
{
    m_positions = cubePositions;
    m_normals = cubeNormals;
    m_texCoords = cubeTexCoords;

    TextureLoader texture;
    TextureOptions texOps;
    texOps.wrapS = GL_REPEAT;
    texOps.wrapT = GL_REPEAT;
    texture.createNew(GL_TEXTURE_2D, texOps);
    texture.fileTexture(diffTexPath);
    m_textures.push_back({ texture.textureID(), "texture_diffuse", texture.path() });

    if (specTexPath != "") {
        texture.createNew(GL_TEXTURE_2D, texOps);
        texture.fileTexture(specTexPath);
        m_textures.push_back({ texture.textureID(), "texture_specular", texture.path() });
    }
    else {
        // Use the same as diffuse texture
        m_textures.push_back({ texture.textureID(), "texture_specular", texture.path() });
    }

    if (normTexPath != "") {
        texture.createNew(GL_TEXTURE_2D, texOps);
        texture.fileTexture(normTexPath);
        m_textures.push_back({ texture.textureID(), "texture_normal", texture.path() });
        
        for (auto i = 0ll; i < m_positions.size(); i += 3ll) {
            glm::vec3 edge1 = m_positions[i + 1ll] - m_positions[i];
            glm::vec3 edge2 = m_positions[i + 2ll] - m_positions[i];
            if (m_normals[i].x < 0 | m_normals[i].y < 0 | m_normals[i].z < 0) {
                edge1 = m_positions[i] - m_positions[i + 1ll];
                edge2 = m_positions[i] - m_positions[i + 2ll];
            }

            glm::vec2 deltaUV1 = m_texCoords[i + 1ll] - m_texCoords[i];
            glm::vec2 deltaUV2 = m_texCoords[i + 2ll] - m_texCoords[i];
            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

            glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
            
            m_tangents.push_back(tangent);
            m_tangents.push_back(tangent);
            m_tangents.push_back(tangent);


            glm::vec3 bitangent = f * (-deltaUV2.x * edge1 + deltaUV1.x * edge2);
            m_bitangents.push_back(bitangent);
            m_bitangents.push_back(bitangent);
            m_bitangents.push_back(bitangent);
        }
    }
    else {
        m_textures.push_back({ 0, "texture_null", "" });
    }

    setupMesh();
}

Cube::~Cube() {}

Quad::Quad(const std::string& diffTexPath, const std::string& specTexPath, const std::string& normTexPath)
{
    m_positions = quadPositions;
    m_normals = quadNormals;
    m_texCoords = quadTexCoords;

    TextureLoader texture;
    TextureOptions texOps;
    texOps.wrapS = GL_REPEAT;
    texOps.wrapT = GL_REPEAT;
    texture.fileTexture(diffTexPath);
    m_textures.push_back({ texture.textureID(), "texture_diffuse", texture.path() });

    if (specTexPath != "") {
        texture.createNew(GL_TEXTURE_2D, texOps);
        texture.fileTexture(specTexPath);
        m_textures.push_back({ texture.textureID(), "texture_specular", texture.path() });
    }
    else {
        // Use the same as diffuse texture
        m_textures.push_back({ texture.textureID(), "texture_specular", texture.path() });
    }

    if (normTexPath != "") {
        texture.fileTexture(normTexPath);
        texture.createNew(GL_TEXTURE_2D, texOps);
        m_textures.push_back({ texture.textureID(), "texture_normal", texture.path() });
    }
    else {
        m_textures.push_back({ 0, "texture_null", "" });
    }

    setupMesh();
}

Quad::~Quad() {}

/* Credit to http://www.songho.ca/opengl/gl_sphere.html */
Sphere::Sphere()
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    const float PI = 3.1415926f;
    float radius = 1.0f;
    float sectorCount = 64, stackCount = 32;

    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
    float s, t;                                     // vertex texCoord

    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i)
    {
        stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // the first and last vertices have same position and normal, but different tex coords
        for (int j = 0; j <= sectorCount; ++j)
        {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            vertices.push_back(glm::vec3(x, y, z));

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            normals.push_back(glm::vec3(nx, ny, nz));

            // vertex tex coord (s, t) range between [0, 1]
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            texCoords.push_back(glm::vec2(s,t));
        }
    }

    int k1, k2;
    for (int i = 0; i < stackCount; ++i)
    {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (i != (stackCount - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    TextureLoader texture;
    TextureOptions texOps;
    texOps.wrapS = GL_REPEAT;
    texOps.wrapT = GL_REPEAT;

    texture.createNew(GL_TEXTURE_2D, texOps);
    texture.fileTexture("res/textures/rusted_iron/rustediron2_basecolor.png");
    m_textures.push_back({ texture.textureID(), "texture_albedo", texture.path() });

    // TODO: Figure out how to do tangent space if actual bump mapping is needed

    texture.createNew(GL_TEXTURE_2D, texOps);
    texture.fileTexture("res/textures/rusted_iron/rustediron2_metallic.png");
    m_textures.push_back({ texture.textureID(), "texture_metal", texture.path() });

    texture.createNew(GL_TEXTURE_2D, texOps);
    texture.fileTexture("res/textures/rusted_iron/rustediron2_roughness.png");
    m_textures.push_back({ texture.textureID(), "texture_rough", texture.path() });

    m_positions = vertices;
    m_normals = normals;
    m_texCoords = texCoords;
    m_indices = indices;

    setupMesh();
}

Sphere::~Sphere() {}
