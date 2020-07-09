#version 450 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

#define PI 3.14159265359
#define MAX_LIGHTS 5

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

uniform vec3 viewPos;

// GBuffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMetalRoughAO;

uniform DirectionalLight directLight;
uniform sampler2D directionalDepthMap;
uniform mat4 lightSpaceMatrix;

layout (std140, binding = 2) uniform LightsUBO
{
    PointLight pointLights[MAX_LIGHTS];
};
uniform samplerCube pointDepthMaps[4];
uniform int numPointLights;
uniform float farPlane;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
float distributionGGX(vec3 N, vec3 H, float roughness);
float geometrySchlickGGX(float NdotV, float roughness);
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 getNormalFromMap(vec3 WorldPos, vec3 Normal);

void main()
{
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    float metallic = texture(gMetalRoughAO, TexCoords).r;
    float roughness = texture(gMetalRoughAO, TexCoords).g;
    vec3 WorldPos = texture(gPosition, TexCoords).rgb;

    vec3 N = normalize(texture(gNormal, TexCoords).rgb);
    vec3 V = normalize(viewPos - WorldPos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, albedo, metallic);
    
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < numPointLights; i++) {
        // calculate per-light radiance
        vec3 L = normalize(pointLights[i].position.xyz - WorldPos);
        vec3 H = normalize(V + L);
        float distance = length(pointLights[i].position.xyz - WorldPos);
        // TODO: windowing
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = pointLights[i].color.xyz * pointLights[i].intensity * attenuation;

        // Cook-Torrance BRDF
        float NDF = distributionGGX(N, H, roughness);   
        float G   = geometrySmith(N, V, L, roughness);      
        vec3 F    = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
           
        vec3 nominator    = NDF * G * F; 
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        vec3 specular = nominator / max(denominator, 0.001);
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;	  

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);        

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = kD * diffuse + specular;
    vec3 color = ambient + Lo;
    
    FragColor = vec4(color, 1.0);

    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0)
        BrightColor = vec4(FragColor.rgb, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}

// Unmodified Schlick approximation of Fresnel reflectance, interpolating between the characteristic
// specular color F0 and white, resulting in the whiter reflections at glancing angles
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// Trowbridge-Reitz/GGX NDF that uses Disney's model for remapping roughness
float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// Smith G1 function approximation proposed by Epic Games
float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// Smith G2 function of the separable form
float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = geometrySchlickGGX(NdotV, roughness);
    float ggx1  = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
