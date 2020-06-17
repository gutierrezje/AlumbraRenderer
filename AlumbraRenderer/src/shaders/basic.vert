#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec2 TexCoords;
    vec3 FragPos;
    vec4 FragPosLightSpace;
    vec3 Normal;
    mat3 TBN;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
} vs_out;

uniform vec3 viewPosV;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

void main()
{

    vs_out.TexCoords = aTexCoords;
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.Normal = normalMatrix * aNormal;
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    vs_out.TBN = transpose(mat3(T, B, N));
    vs_out.TangentViewPos  = vs_out.TBN * viewPosV;
    vs_out.TangentFragPos  = vs_out.TBN * vs_out.FragPos;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
