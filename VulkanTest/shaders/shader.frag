#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTextCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in vec3 fragCameraPos;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

layout(set = 0, binding = 3) uniform MaterialUBO {
	vec4 baseColorFactor;
} material;

layout(push_constant) uniform ModelLightPC {
	mat4 model;
    vec4 pos;
} lightPC;


void main() {

	// | beging blinn-phong

	vec3 albedo = material.baseColorFactor.rgb * texture(texSampler, vec2(fragTextCoord.x, fragTextCoord.y)).rgb;
	float shininess = 32.0;
	vec3 lightPos = lightPC.pos.xyz;
	float lightDistance = length(lightPos - fragPos);

	float radius = 3.0;   // <- increase this for more coverage
	float attenuation = clamp(1.0 - (lightDistance / radius), 0.0, 1.0);
	attenuation *= attenuation;  // smoother falloff

	vec3 N = normalize(fragNormal);
	vec3 L = normalize(lightPos - fragPos);
	vec3 V = normalize(fragCameraPos - fragPos);
	vec3 H = normalize(L + V);

	float diff = max(dot(N, L), 0.0);
	float spec = pow(max(dot(N, H), 0.0), shininess);

	vec3 ambient = 0.1 * albedo;

	float lightPower = 2.0f;

	vec3 diffuse = diff * albedo * lightPower * attenuation;

	vec3 specular = spec * vec3(1.0) * lightPower * attenuation;

	vec3 color = ambient + diffuse + specular;

	outColor = vec4(color, 1.0);

}