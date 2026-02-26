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

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTextCoord;
 
void main() {
    gl_Position = camera.proj * camera.view * modelPC.model * vec4(inPosition, 1.0);

    fragColor = inColor;
    fragTextCoord = inTextCoord;
}