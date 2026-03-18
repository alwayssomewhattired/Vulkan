#version 450


layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec4 pos;

} camera;

layout(set = 1, binding = 0) uniform BonesUBO {
    mat4 bones[100];
} bones;

// | ignore the red underlines
layout(push_constant) uniform ModelLightPC {
    mat4 model;
    vec4 pos;
} modelPC;
 
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTextCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;
layout(location = 6) in ivec4 inBoneIDs;
layout(location = 7) in vec4 inWeights;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTextCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPos;
layout(location = 4) out vec3 fragCameraPos;
layout(location = 5) out mat3 fragTBN;

void main() {

    // | animation
    mat4 skinMatrix = 
        inWeights.x * bones[inBoneIDs.x] +
        inWeights.y * bones[inBoneIDs.y] +
        inWeights.z * bones[inBoneIDs.z] +
        inWeights.w * bones[inBoneIDs.w];
    vec4 skinnedPos = skinMatrix * vec4(inPosition, 1.0);
    
    // | lighting
    vec3 T = normalize(mat3(modelPC.model) * inTangent);
    vec3 B = normalize(mat3(modelPC.model) * inBitangent);
    vec3 N = normalize(mat3(modelPC.model) * inNormal);
//    vec3 B = cross(N, T) * inTangent.w;
   fragTBN = mat3(T, B, N);

    vec4 worldPos = modelPC.model * vec4(inPosition, 1.0);

    gl_Position = camera.proj * camera.view * worldPos * skinnedPos;

    fragColor = inColor;
    fragTextCoord = inTextCoord;
    fragPos = worldPos.xyz;

    // | normal transform
    mat3 normalMatrix = transpose(inverse(mat3(modelPC.model)));

    fragNormal = normalize(normalMatrix * inNormal);

    fragCameraPos = camera.pos.xyz;
}