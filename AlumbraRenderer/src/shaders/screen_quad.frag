#version 450
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;
uniform bool bloom;
uniform float exposure;

void main()
{
	vec3 hdrColor = texture(sceneTexture, TexCoords).rgb;
	vec3 bloomColor = texture(bloomTexture, TexCoords).rgb;
	if (bloom) {
		hdrColor += bloomColor;
	}
	vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
	FragColor = vec4(result, 1.0);
}