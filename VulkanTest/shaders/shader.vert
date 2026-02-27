#version 450


layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} camera;

// | ignore the red underlines
layout(push_constant) uniform ModelPC {
    mat4 model;
} modelPC;
 
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTextCoord;
layout(location = 3) in vec3 normal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTextCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPos;
 
void main() {
    
    vec4 worldPos = modelPC.model * vec4(inPosition, 1.0);

    gl_Position = camera.proj * camera.view * worldPos;

    fragColor = inColor;
    fragTextCoord = inTextCoord;
//    fragPos = worldPos.xyz;
    fragPos = (camera.view * modelPC.model * vec4(inPosition, 1.0)).xyz;

    // | normal transform
    mat3 normalMatrix = transpose(inverse(mat3(modelPC.model)));

//    fragNormal = normalize(normalMatrix * normal);
    fragNormal = normalize(mat3(camera.view * modelPC.model) * normal);
}