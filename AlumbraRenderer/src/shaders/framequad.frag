#version 450
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float exposure;
uniform float nearPlane;
uniform float farPlane;

float linearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main()
{
	vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
	vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
	FragColor = vec4(mapped, 1.0);
}