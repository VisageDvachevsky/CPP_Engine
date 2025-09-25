#version 330 core

in vec3 WorldPos;
in float DistanceFromCamera;

out vec4 FragColor;

uniform vec3 u_gridColor;
uniform float u_fadeDistance;

void main() {
    float alpha = 1.0 - smoothstep(0.0, u_fadeDistance, DistanceFromCamera);
    
    bool isXAxis = abs(WorldPos.z) < 0.05;
    bool isZAxis = abs(WorldPos.x) < 0.05;
    
    vec3 color = u_gridColor;
    if (isXAxis) {
        color = vec3(1.0, 0.3, 0.3); // Red X axis
        alpha *= 2.0;
    } else if (isZAxis) {
        color = vec3(0.3, 0.3, 1.0); // Blue Z axis  
        alpha *= 2.0;
    }
    
    alpha = clamp(alpha, 0.0, 1.0);
    FragColor = vec4(color, alpha);
}