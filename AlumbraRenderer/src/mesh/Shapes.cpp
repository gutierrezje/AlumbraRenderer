#include "../pch.h"
#include "Shapes.h"

Cube::Cube(std::string texturePath)
{
  vertices = cubeVertices;
  Texture texture;
  texture.loadTexture(texturePath);
  textures = std::vector<MeshTexture>{ {texture.textureID(), "material.albedo", texture.path()} };

  setupMesh();
}

Cube::~Cube() {}