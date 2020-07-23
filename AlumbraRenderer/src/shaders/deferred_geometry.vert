#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec2 TexCoords;
    vec3 FragPos;
    vec3 Normal;
    mat3 TBN;
} vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vs_out.TexCoords = aTexCoords;
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.FragPos = worldPos.xyz;

    // Expensive and inverse could not exist
    //mat3 normalMatrix = transpose(inverse(mat3(model)));
    // As long as tranformations are affine, adjoint will cover most cases
    mat3 normalMatrix = adjoint(model); 
    // If no non-uniform scaling occured, can use original transform matrix
    //mat3 normalMatrix = mat3(model);
    // If only translations and rotations took place, no need to renormalize
    //float normalMatrix = 1;

    vs_out.Normal = normalMatrix * aNormal;

    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B =  normalize(normalMatrix * aBitangent);
    vec3 N = normalize(normalMatrix * aNormal);
    // Re-orthogonalization for when TS vectors have been averaged
    T = normalize(T - dot(T, N) * N);

    vs_out.TBN = (mat3(T, B, N));

    gl_Position = projection * view * worldPos;
}

mat3 adjoint(mat4 m)
{
    return mat3( 
        m[1][1] * m[2][2] - m[1][2] * m[2][1],
        m[1][2] * m[2][0] - m[1][0] * m[2][2],
        m[1][0] * m[2][1] - m[1][1] * m[2][0],
        m[0][2] * m[2][1] - m[0][1] * m[2][2],
        m[0][0] * m[2][2] - m[0][2] * m[2][0],
        m[0][1] * m[2][0] - m[0][0] * m[2][1],
        m[0][1] * m[1][2] - m[0][2] * m[1][1],
        m[0][2] * m[1][0] - m[0][0] * m[1][2],
        m[0][0] * m[1][1] - m[0][1] * m[1][0]);
}


