#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec2 TexCoords;
    vec3 FragPos;
    vec4 FragPosLightSpace;
    vec3 Normal;
    mat3 TBN;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{
    vs_out.TexCoords = aTexCoords;
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.FragPos = worldPos.xyz;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.Normal = normalMatrix * aNormal;

    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B =  normalize(normalMatrix * aBitangent);
    vec3 N = normalize(normalMatrix * aNormal);
    // Re-orthogonalization for when TS vectors have been averaged
    T = normalize(T - dot(T, N) * N);

    vs_out.TBN = (mat3(T, B, N));

    gl_Position = projection * view * worldPos;
}
