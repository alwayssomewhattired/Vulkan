#version 450


layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
    vec4 pos;

} camera;

layout(set = 2, binding = 0) uniform BonesUBO {
    mat4 bones[100];
} bonesUBO;

// | ignore the red underlines
layout(push_constant) uniform ModelLightPC {
    mat4 model;
    vec4 pos;
    uint materialIndex;
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

    mat4 skinMatrix = mat4(0.0);
    float totalWeight = 0.0;

    for (int i = 0; i < 4; i++) {
        int id = inBoneIDs[i];
        float w = inWeights[i];

        if (id >= 0 && id < bonesUBO.bones.length() && w > 0.0) {
            skinMatrix += w * bonesUBO.bones[id];
            totalWeight += w;
        }
    }

    if (totalWeight > 0.0) {
        skinMatrix /= totalWeight;
    } else {
        skinMatrix = mat4(1.0);
    }

    mat3 skinMat3 = transpose(inverse(mat3(skinMatrix)));

    vec3 skinnedNormal = normalize(skinMat3 * inNormal);
    vec3 skinnedTangent = normalize(skinMat3 * inTangent);
    vec3 skinnedBitangent = normalize(skinMat3 * inBitangent);
    
    // | lighting
    vec3 T = normalize(mat3(modelPC.model) * skinnedTangent);
    vec3 B = normalize(mat3(modelPC.model) * skinnedBitangent);
    vec3 N = normalize(mat3(modelPC.model) * skinnedNormal);

   fragTBN = mat3(T, B, N);

   vec4 skinnedPos = skinMatrix * vec4(inPosition, 1.0);


//    vec4 skinnedPos = bonesUBO.bones[0] * vec4(inPosition, 1.0);
//    vec4 skinnedPos = vec4(inPosition, 1.0);

    vec4 worldPos = modelPC.model * skinnedPos;

    gl_Position = camera.proj * camera.view * worldPos;

    fragColor = inColor;
    fragTextCoord = inTextCoord;
    fragPos = worldPos.xyz;

    // | normal transform
    mat3 normalMatrix = transpose(inverse(mat3(modelPC.model)));

    fragNormal = normalize(normalMatrix * skinnedNormal);

    fragCameraPos = camera.pos.xyz;
}