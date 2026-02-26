#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTextCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

//vec3 fresnelSchlick(float cosTheta, vec3 F0) {
//	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
//}
//
//float distributionGGX(vec3 N, vec3 H, float roughness) {
//	
//	float a = roughness * roughness;
//	float a2 = a * a;
//	float NdotH = max(dot(N, H), 0.0);
//	float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
//	return a2 / (PI * denom * denom);
//}
//
//
//float geometrySchlickGGX(float NdotV, float roughness) {
//
//	float r = roughness + 1.0;
//	float k = (r * r) / 8.0;
//	return NdotV / (NdotV * (1.0 - k) + k);
//}


void main() {
	
	// | begin PBR lighting
	
	//vec3 L = normalize(lightPos - worldPos);
	//vec3 H = normalize(V + L);
	//
	//
	//float NDF = distributionGGX(N, H, roughness);
	//float G = geometrySchlickGGX(N, V, L, roughness);
	//vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
	//
	//vec3 numerator = NDF * G * F;
	//float denom = 4.0 * max(dot(N,V),0.0) * max(dot(N, L),0.0) + 0.001;
	//vec3 specular = numerator / denom;
	//
	//vec3 kS = F;
	//vec3 kD = (1.0 - kS) * (1.0 - metallic);
	//
	//vec3 Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	//// | end PBR lighting

	// | flip texture 
	vec2 uv = fragTextCoord;
	uv.y = 1.0 - uv.y;

	outColor = texture(texSampler, uv);
}