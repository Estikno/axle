#version 460 core

layout (location = 0) in vec3 aPos;

out vec2 TexCoords;

layout (std140, binding = 0) uniform Scene {
    // Camera
    mat4 u_ViewMatrix;
    mat4 u_ProjectionMatrix;
    mat4 u_ViewProjectionMatrix;
    vec3 u_CameraPosition;

    // Frame info
    double u_Time;
};

void main(){
    TexCoords = aPos.xy * 0.5 + 0.5;
    gl_Position = vec4(aPos, 1.0);
}
