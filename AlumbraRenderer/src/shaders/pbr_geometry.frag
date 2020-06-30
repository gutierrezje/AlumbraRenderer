#version 450 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedo;
layout (location = 3) out vec3 gMetalRoughAO;

in VS_OUT {
    vec2 TexCoords;
    vec3 FragPos;
    vec3 Normal;
    mat3 TBN;
} fs_in;

struct Material {
    sampler2D texture_albedo1;
    sampler2D texture_normal1;
    sampler2D texture_metallic1;
    sampler2D texture_roughness1;
};

uniform bool useNormalMap;
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
        vec3 norm = texture(material.texture_normal1, fs_in.TexCoords).rgb;
        norm = norm * 2.0 - 1.0;
        gNormal = normalize(fs_in.TBN * norm);
    }
    else {
        gNormal = fs_in.Normal;
    }
    // and the diffuse per-fragment color
    gAlbedo.rgb = albedo;
    // store specular intensity in gAlbedoSpec's alpha component
    gMetalRoughAO.r = metallic;
    gMetalRoughAO.g = roughness;
    gMetalRoughAO.b = 0;
}
