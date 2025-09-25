#version 330 core

layout (location = 0) in vec3 aPos;

out vec3 WorldPos;
out float DistanceFromCamera;

uniform mat4 u_mvp;
uniform vec3 u_cameraPos;

void main() {
    WorldPos = aPos;
    DistanceFromCamera = length(u_cameraPos - aPos);
    gl_Position = u_mvp * vec4(aPos, 1.0);
}