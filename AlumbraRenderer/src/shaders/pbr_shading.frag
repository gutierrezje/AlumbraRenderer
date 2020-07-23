#version 450 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec2 TexCoords;

#define PI 3.14159265359
#define MAX_REFLECTION_LOD 4.0
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

// Directional lighting
uniform DirectionalLight directLight;
uniform sampler2D directionalDepthMap;
uniform mat4 lightSpaceMatrix;

// Punctual lighting
layout (std140, binding = 2) uniform LightsUBO
{
    PointLight pointLights[MAX_LIGHTS];
};
uniform samplerCube pointDepthMaps[4];
uniform int numPointLights;
uniform float farPlane;

// PBR Shading
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

vec3 fresnelSchlick(float cosTheta, vec3 F0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness);
float distributionGGX(vec3 N, vec3 H, float roughness);
float geometrySchlickGGX(float NdotV, float roughness);
float geometrySmith(float NdotV, float NdotL, float roughness);
vec3 calcPuncLight(PointLight light, vec3 V, vec3 N, vec3 P, vec3 albedo, vec3 F0, float rough, float metal, int index);
vec3 calcDirLight(DirectionalLight light, vec3 V, vec3 N, vec3 albedo, vec3 F0, float rough, float metal);

void main()
{
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    float metallic = texture(gMetalRoughAO, TexCoords).r;
    float roughness = texture(gMetalRoughAO, TexCoords).g;
    vec3 P = texture(gPosition, TexCoords).rgb;

    vec3 N = normalize(texture(gNormal, TexCoords).rgb);
    vec3 V = normalize(viewPos - P);
    vec3 R = reflect(-V, N);

    // The characteristic specular color, either white or a variable reflectance color, dependent on metallic
    vec3 F0 = vec3(0.04); 
    F0      = mix(F0, albedo, metallic);
    
    // calculate per-light outgoing radiance
    vec3 Lo = calcDirLight(directLight, V, N, albedo, F0, roughness, metallic);
    for(int i = 0; i < numPointLights; i++) {
        Lo += calcPuncLight(pointLights[i], V, N, P, albedo, F0, roughness, metallic, i);
    }

    // IBL Lighting
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;
    
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

vec3 calcPuncLight(PointLight light, vec3 V, vec3 N, vec3 P, vec3 albedo, vec3 F0, float rough, float metal, int index)
{
    // Precalcs for light attenuation
    vec3 lightPos = light.position.xyz;
    vec3 lightDir = lightPos - P;
    float lightLen = length(lightDir);
    float dist2 = lightLen * lightLen;
    float dist4 = dist2 * dist2;
    float radius4 = pow(light.radius, 4.0);

    float windowing = clamp(1.0 - dist4 / radius4, 0.0, 1.0);
    float attenuation = windowing * windowing / (dist2 + 1.0);
    vec3 radianceIn = light.color.xyz * light.intensity * attenuation;

    vec3 L = lightDir / lightLen;
    vec3 H = normalize(L + V);
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float D     = distributionGGX(N, H, rough);
    float G     = geometrySmith(NdotV, NdotL, rough);
    vec3 F      = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    // Specular BRDF term
    vec3 numerator      = D * G * F;
    float denominator   = 4 * NdotV * NdotL;
    vec3 specular       = numerator / max(denominator, 0.0001);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;
    vec3 radianceOut = (kD * albedo / PI + specular) * radianceIn * NdotL;
    return radianceOut;
}

vec3 calcDirLight(DirectionalLight light, vec3 V, vec3 N, vec3 albedo, vec3 F0, float rough, float metal)
{
    // Light is not attenuated
    vec3 radianceIn = light.color * light.intensity;
    
    vec3 L = normalize(-light.direction);
    vec3 H = normalize(L + V);
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float D     = distributionGGX(N, H, rough);
    float G     = geometrySmith(NdotV, NdotL, rough);
    vec3 F      = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

    vec3 numerator      = D * G * F;
    float denominator   = 4 * NdotV * NdotL;
    vec3 specular       = numerator / max(denominator, 0.0001);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metal;
    vec3 radianceOut = (kD * albedo / PI + specular) * radianceIn * NdotL;
    return radianceOut;
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
float geometrySmith(float NdotV, float NdotL, float roughness)
{
    float ggx2  = geometrySchlickGGX(NdotV, roughness);
    float ggx1  = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
