#version 450 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in VS_OUT {
    vec2 TexCoords;
    vec3 FragPos;
    vec3 Normal;
    mat3 TBN;
} fs_in;

struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_normal1;
};

uniform bool useNormalMap;
uniform Material material;

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
        gNormal = normalize(fs_in.Normal);
    }
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = texture(material.texture_specular1, fs_in.TexCoords).r;
}
