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
struct Material {
	sampler2D albedo;
    float shininess;
};

uniform Material material;
uniform vec3 viewPos;
uniform PointLight pointLight;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{    
    // Properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = calcPointLight(pointLight, norm, FragPos, viewDir);
    FragColor = vec4(result, 1.0);
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 d = light.position - fragPos;
    float r = length(d);
    vec3 lightDir = d / r;

    vec3 albedo = texture(material.albedo, TexCoords).rgb;
    // Ambient shading
    vec3 ambient = 0.3 * albedo;
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * albedo;
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = spec * albedo;
    // Inverse square light attenuation
    float windowing = clamp(1 - pow(r / light.radius, 4.0), 0.0, 1.0);
    float attenuation = windowing * windowing / (r * r + 1.0);

    vec3 radiance = light.intensity * light.color * attenuation;
    vec3 result = (ambient + diffuse + specular) * radiance;
    return result;
}