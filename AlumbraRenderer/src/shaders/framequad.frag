#version 450
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float nearPlane;
uniform float farPlane;

float linearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	return (2.0 * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main()
{
	float depthValue = texture(screenTexture, TexCoords).r;
	//FragColor = vec4(vec3(linearizeDepth(depthValue) / farPlane), 1.0);
	FragColor = vec4(vec3(depthValue), 1.0);
}