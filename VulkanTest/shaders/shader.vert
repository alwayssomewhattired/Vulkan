#version 450


layout(push_constant) uniform PushConsts {
    mat4 model;
} pc;

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 proj;
} camera;

layout(set = 0, binding = 1) uniform ModelUBO {
    mat4 model;
} modelUbo;
 
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTextCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTextCoord;
 
void main() {
    //gl_Position = camera.proj * camera.view * pc.model * vec4(inPosition, 1.0);
    gl_Position = camera.proj * camera.view * modelUbo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTextCoord = inTextCoord;
}