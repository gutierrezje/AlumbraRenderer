#version 450 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
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

uniform Material material;
uniform vec3 viewPos;
uniform int numPointLights;
uniform PointLight pointLights[MAX_LIGHTS];
uniform SpotLight spotLight;
uniform DirectionalLight directLight;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcDirectLight(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    
    // Properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    int numLights = MAX_LIGHTS < numPointLights ? MAX_LIGHTS : numPointLights;
    vec3 result = calcDirectLight(directLight, norm, FragPos, viewDir);
    for (int i = 0; i < numLights; i++) {
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir);
    }
    result += calcSpotLight(spotLight, norm, FragPos, viewDir);
    FragColor = vec4(result, 1.0);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    // Normalize direction to light vector
    vec3 d = light.position - fragPos;
    float r = length(d);
    vec3 lightDir = d / r;

    vec3 diffTex = texture(material.texture_diffuse1, TexCoords).rgb;
    vec3 specTex = texture(material.texture_specular1, TexCoords).rgb;

    // Ambient shading
    vec3 ambient = 0.3 * diffTex;
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * diffTex;
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = spec * specTex;
    // Inverse square light attenuation
    float windowing = clamp(1 - pow(r / light.radius, 4.0), 0.0, 1.0);
    float attenuation = windowing * windowing / (r * r + 1.0);

    vec3 radiance = light.intensity * light.color * attenuation;
    vec3 result = (ambient + diffuse + specular) * radiance;
    return result;
}

vec3 calcDirectLight(DirectionalLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);

    vec3 diffTex = texture(material.texture_diffuse1, TexCoords).rgb;
    vec3 specTex = texture(material.texture_specular1, TexCoords).rgb;
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 ambient = 0.3 * diffTex;
    vec3 diffuse = diff * diffTex;
    vec3 specular = spec * specTex;

    vec3 radiance = light.intensity * light.color;
    vec3 result = (ambient + diffuse + specular) * radiance;
    return result;
}

vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 d = light.position - fragPos;
    float r = length(d);
    vec3 lightDir = d / r;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);

    vec3 diffTex = texture(material.texture_diffuse1, TexCoords).rgb;
    vec3 specTex = texture(material.texture_specular1, TexCoords).rgb;
    vec3 ambient = 0.3 * diffTex;
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
