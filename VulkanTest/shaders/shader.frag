#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTextCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

layout(push_constant) uniform LightPC {
    vec4 pos;
} lightPC;


void main() {

	// | beging blinn-phong

//	vec3 albedo = vec3(1.0, 0.8, 0.7);
	vec3 albedo = texture(texSampler, vec2(fragTextCoord.x, 1.0 - fragTextCoord.y)).rgb;
	float shininess = 32.0;

	vec3 N = normalize(fragNormal);
	vec3 L = normalize(lightPC.pos.xyz - fragPos);
	vec3 V = normalize(-fragPos);
	vec3 H = normalize(L + V);

	float diff = max(dot(N, L), 0.0);
	float spec = pow(max(dot(N, H), 0.0), shininess);

	vec3 ambient = 0.1 * albedo;
	vec3 diffuse = diff * albedo;
	vec3 specular = spec * vec3(1.0);
	vec3 color = ambient + diffuse + specular;

	outColor = vec4(color, 1.0);

	// | flip texture 
//	vec2 uv = fragTextCoord;
//	uv.y = 1.0 - uv.y;

//	outColor = texture(texSampler, uv);
}