#version 450 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 viewPos;
uniform samplerCube cubemap;

float shlick(float cosine, float refractIdx);
float rand(vec2 co);

void main()
{    
    vec3 unitDir = normalize(Position - viewPos);
    float ratio = 1.00 / 1.52;
    vec3 refracted = refract(unitDir, normalize(Normal), ratio);
    FragColor = vec4(texture(cubemap, refracted).rgb, 1.0);
}

float shlick(float cosine, float refractIdx) {
    float r_0 = (1.0 - refractIdx) / (1.0 + refractIdx);
    r_0 *= r_0;
    return r_0 + (1.0 - r_0) * pow((1.0 - cosine), 5.0);
}
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453123);
}
