#version 450 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec4 position;
    vec4 color;
    float radius;
    float intensity;
};

#define MAX_LIGHTS 5

layout (std140, binding = 2) uniform LightsUBO
{
    PointLight pointLights[MAX_LIGHTS];
};
uniform samplerCube pointDepthMaps[1];

uniform DirectionalLight directLight;
uniform sampler2D directionalDepthMap;
uniform mat4 lightSpaceMatrix;

uniform vec3 viewPos;
uniform int numPointLights;
uniform float farPlane;

// GBuffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int lightIndex);
float calcPointShadow(vec3 fragPos, vec3 lightPos, float bias, int lightIndex);
vec3 calcDirectLight(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float calcDirectionalShadow(vec4 fragPosLightSpace, float bias);

vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);  

void main()
{    
    // Properties
    vec3 normal = texture(gNormal, TexCoords).rgb;
    vec3 fragPos = texture(gPosition, TexCoords).rgb;
    vec3 viewDir = normalize(viewPos - fragPos);

    vec3 result = calcDirectLight(directLight, normal, fragPos, viewDir);
    
    int numLights = MAX_LIGHTS < numPointLights ? MAX_LIGHTS : numPointLights;
    for (int i = 0; i < numLights; i++) {
        result += calcPointLight(pointLights[i], normal, fragPos, viewDir, i);
    }
    //result = norm;
    FragColor = vec4(result, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int lightIndex) {
    // Normalize direction to light vector
    vec3 lightPos = light.position.xyz;
    vec3 d = lightPos - fragPos;
    float r = length(d);
    vec3 lightDir = d / r;

    vec3 diffTex = texture(gAlbedoSpec, TexCoords).rgb;
    vec3 specTex = texture(gAlbedoSpec, TexCoords).aaa;

    // Ambient shading
    vec3 ambient = 0.1 * diffTex;
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * diffTex;
    // Specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 16.0);
    vec3 specular = spec * specTex;
    // Inverse square light attenuation
    float windowing = clamp(1 - pow(r / light.radius, 4.0), 0.0, 1.0);
    float attenuation = windowing * windowing / (r * r + 1.0);

    vec3 radiance = light.intensity * light.color.xyz * attenuation;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = calcPointShadow(fragPos, lightPos, bias, lightIndex);
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * radiance;
    return result;
}

float calcPointShadow(vec3 fragPos, vec3 lightPos, float bias, int lightIndex) {
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    float viewDistance = length(viewPos - fragPos);
    float discRadius = (1.0 + (viewDistance / farPlane)) / 25.0;
    float shadow = 0.0;
    int samples = 20;
    for (int i = 0; i < samples; i++) {
        float closestDepth = texture(pointDepthMaps[lightIndex], fragToLight + sampleOffsetDirections[i] * discRadius).r;
        closestDepth *= farPlane;
        if (currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    return shadow / float(samples);
}

vec3 calcDirectLight(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);

    vec3 diffTex = texture(gAlbedoSpec, TexCoords).rgb;
    vec3 specTex = texture(gAlbedoSpec, TexCoords).aaa;
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 ambient = 0.1 * diffTex;
    vec3 diffuse = diff * diffTex;
    vec3 specular = spec * specTex;

    vec3 radiance = light.intensity * light.color;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = calcDirectionalShadow(lightSpaceMatrix * vec4(fragPos, 1.0), bias);
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * radiance;
    return result;
}

float calcDirectionalShadow(vec4 fragPosLightSpace, float bias)
{
    // Transorm from clip-space to NDC [-1,1]; needed for perspective proj, but not ortho
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(directionalDepthMap, projCoords.xy).r;
    float currentDepth = projCoords.z;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(directionalDepthMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(directionalDepthMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    if (projCoords.z > 1.0) {
        shadow = 0.0;
    }
    return shadow;
}
