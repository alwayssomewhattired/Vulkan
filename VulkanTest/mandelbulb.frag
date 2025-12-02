#version 450

layout(location = 0) in vec2 v_uv;
layout(location = 0) out vec4 outColor;

// Uniforms: camera + fractal params
layout(set = 0, binding = 0) uniform UBO {
    mat4 invProjection;
    mat4 invView;
    vec3 camPos;
    float time;
    vec2 resolution;
    float power;        // fractal power (usually 8)
    int maxIter;
    float bail;         // bailout radius (e.g., 2.0 or 8.0)
} ubo;

// helpers
const float EPSILON = 1e-4;
const float MAX_DIST = 100.0;

// distance estimator for Mandelbulb with derivative
float mandelbulbDE(vec3 pos, out int steps) {
    vec3 z = pos;
    float dr = 1.0;
    float r = 0.0;
    float power = ubo.power;
    float bailout = ubo.bail;
    steps = 0;
    for (int i = 0; i < ubo.maxIter; ++i) {
        r = length(z);
        if (r > bailout) {
            break;
        }

        // convert to spherical coords
        float theta = acos(clamp(z.z / r, -1.0, 1.0));
        float phi = atan(z.y, z.x);
        // scale and rotate
        float zr = pow(r, power);
        float newTheta = theta * power;
        float newPhi = phi * power;
        // spherical back to cartesian
        z = zr * vec3(sin(newTheta) * cos(newPhi),
                      sin(newTheta) * sin(newPhi),
                      cos(newTheta));
        z += pos; // add c

        // derivative
        dr = zr * power * dr / max(r, 1e-6) + 1.0;
        ++steps;
    }
    // distance estimate formula
    return 0.5 * log(max(r, 1e-6)) * r / max(dr, 1e-6);
}

// ray-march using sphere tracing
float rayMarch(vec3 ro, vec3 rd, out int stepsTotal) {
    float t = 0.0;
    stepsTotal = 0;
    for (int i = 0; i < 256; ++i) {
        vec3 p = ro + rd * t;
        int steps;
        float d = mandelbulbDE(p, steps);
        stepsTotal += steps;
        if (d < EPSILON) return t;
        if (t > MAX_DIST) return -1.0;
        t += d;
    }
    return -1.0;
}

// estimate normal using gradient
vec3 estimateNormal(vec3 p) {
    float h = 1e-3;
    int dummy;
    float dx = mandelbulbDE(p + vec3(h,0,0), dummy) - mandelbulbDE(p - vec3(h,0,0), dummy);
    float dy = mandelbulbDE(p + vec3(0,h,0), dummy) - mandelbulbDE(p - vec3(0,h,0), dummy);
    float dz = mandelbulbDE(p + vec3(0,0,h), dummy) - mandelbulbDE(p - vec3(0,0,h), dummy);
    return normalize(vec3(dx,dy,dz));
}

vec3 getRayDir(vec2 uv) {
    // uv in [0,1] -> NDC [-1,1]
    vec2 ndc = uv * 2.0 - 1.0;
    ndc.x *= ubo.resolution.x / ubo.resolution.y;
    // reconstruct view-space direction
    vec4 clip = vec4(ndc, -1.0, 1.0);
    vec4 view = ubo.invProjection * clip;
    view /= view.w;
    vec4 world = ubo.invView * vec4(view.xyz, 0.0);
    return normalize(world.xyz);
}

void main() {
    vec2 uv = v_uv;
    vec3 ro = ubo.camPos;
    vec3 rd = getRayDir(uv);

    int marchSteps;
    float t = rayMarch(ro, rd, marchSteps);
    if (t < 0.0) {
        // background gradient
        vec3 bg = mix(vec3(0.02,0.03,0.06), vec3(0.08,0.12,0.2), clamp(rd.y * 0.5 + 0.5, 0.0, 1.0));
        outColor = vec4(bg, 1.0);
        return;
    }

    vec3 p = ro + rd * t;
    vec3 n = estimateNormal(p);

    // simple lighting
    vec3 lightDir = normalize(vec3(0.5, 0.8, -0.6));
    float diff = max(dot(n, lightDir), 0.0);
    float spec = pow(max(dot(reflect(-lightDir, n), -rd), 0.0), 32.0);

    // ambient occlusion (cheap approximation)
    float ao = clamp(1.0 - float(marchSteps) * 0.01, 0.3, 1.0);

    vec3 baseColor = vec3(0.7, 0.4, 0.9) * (0.5 + 0.5 * sin(float(marchSteps) * 0.1 + ubo.time * 0.5));
    vec3 color = baseColor * (0.2 + 0.8 * diff) + vec3(1.0) * spec * 0.25;
    color *= ao;

    outColor = vec4(color, 1.0);
}
