#version 450 core

in VS_OUT {
    vec2 TexCoords;
    vec3 FragPos;
    vec3 Normal;
    vec4 FragPosLightSpace;
} fs_in;

out vec4 FragColor;

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

struct SpotLight {
    vec3 position;
    vec3 color;
    float radius;
    float intensity;
    
    vec3 direction;
    float umbra;
    float penumbra;
};

struct Material {
	sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    float shininess;
};

#define MAX_LIGHTS 5

layout (std140, binding = 2) uniform LightsUBO
{
    PointLight pointLights[MAX_LIGHTS];
};

uniform Material material;
uniform vec3 viewPos;
uniform int numPointLights;
uniform float farPlane;
// Currently this has to match exactly how many point lights there are in the scene to work
uniform samplerCube pointDepthMaps[2];
uniform DirectionalLight directLight;
uniform sampler2D directionalDepthMap;
uniform SpotLight spotLight;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int lightIndex);
float calcPointShadow(vec3 fragPos, vec3 lightPos, float bias, int lightIndex);
vec3 calcDirectLight(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float calcDirectionalShadow(vec4 fragPosLightSpace, float bias);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

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
    vec3 norm = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    int numLights = MAX_LIGHTS < numPointLights ? MAX_LIGHTS : numPointLights;
    vec3 result = calcDirectLight(directLight, norm, fs_in.FragPos, viewDir);
    
    for (int i = 0; i < numLights; i++) {
        result += calcPointLight(pointLights[i], norm, fs_in.FragPos, viewDir, i);
    }
    FragColor = vec4(result, 1.0);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int lightIndex) {
    // Normalize direction to light vector
    vec3 d = light.position.xyz - fragPos;
    float r = length(d);
    vec3 lightDir = d / r;

    vec3 diffTex = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 specTex = texture(material.texture_specular1, fs_in.TexCoords).rgb;

    // Ambient shading
    vec3 ambient = 0.1 * diffTex;
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * diffTex;
    // Specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = spec * specTex;
    // Inverse square light attenuation
    float windowing = clamp(1 - pow(r / light.radius, 4.0), 0.0, 1.0);
    float attenuation = windowing * windowing / (r * r + 1.0);

    vec3 radiance = light.intensity * light.color.xyz * attenuation;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = calcPointShadow(fragPos, light.position.xyz, bias, lightIndex);
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

    vec3 diffTex = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 specTex = texture(material.texture_specular1, fs_in.TexCoords).rgb;
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 ambient = 0.1 * diffTex;
    vec3 diffuse = diff * diffTex;
    vec3 specular = spec * specTex;

    vec3 radiance = light.intensity * light.color;
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    float shadow = calcDirectionalShadow(fs_in.FragPosLightSpace, bias);
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

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 d = light.position - fragPos;
    float r = length(d);
    vec3 lightDir = d / r;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    vec3 diffTex = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;
    vec3 specTex = texture(material.texture_specular1, fs_in.TexCoords).rgb;
    vec3 ambient = 0.1 * diffTex;
    vec3 diffuse = diff * diffTex;
    vec3 specular = spec * specTex;
    
    float windowing = clamp(1 - pow(r / light.radius, 4.0), 0.0, 1.0);
    float attenuation = windowing * windowing / (r * r + 1.0);

    // Angle between spotlight direction vector and reversed light vector
    float theta = dot(lightDir, normalize(-light.direction));
    float t = clamp((cos(theta) - cos(light.umbra)) / (cos(light.penumbra) - cos(light.umbra)), 0.0, 1.0);
    float dirFalloff = t * t * (3.0 - 2.0 * t);
    vec3 radiance = light.intensity * light.color * attenuation;
    vec3 result = (ambient + diffuse + specular) * radiance * dirFalloff;
    return result;
}
