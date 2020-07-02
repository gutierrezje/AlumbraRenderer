#version 450 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedo;
layout (location = 3) out vec3 gMetalRoughAO;

in VS_OUT {
    vec2 TexCoords;
    vec3 FragPos;
    vec3 Normal;
    mat3 TBN;
} fs_in;

struct Material {
    sampler2D texture_albedo;
    sampler2D texture_normal;
    sampler2D texture_metal;
    sampler2D texture_rough;
};

uniform bool useNormalMap;
uniform bool useMetalMap;
uniform bool useRoughMap;
uniform Material material;

uniform vec3  albedo;
uniform float metallic;
uniform float roughness;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = fs_in.FragPos;
    // also store the per-fragment normals into the gbuffer
    if (useNormalMap) {
        vec3 norm = texture(material.texture_normal, fs_in.TexCoords).rgb;
        norm = norm * 2.0 - 1.0;
        gNormal = normalize(fs_in.TBN * norm);
    }
    else {
        gNormal = fs_in.Normal;
    }
    // and the diffuse per-fragment color
    gAlbedo = texture(material.texture_albedo, fs_in.TexCoords);
    // store pbr properties into separate gbuffer texture
    gMetalRoughAO.r = texture(material.texture_metal, fs_in.TexCoords).r;
    gMetalRoughAO.g = texture(material.texture_rough, fs_in.TexCoords).r;
    gMetalRoughAO.b = 1.0;
}
