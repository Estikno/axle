#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

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
    vec4 pos = inverse(u_ViewProjectionMatrix) * vec4(aPos.xy, 1.0, 1.0);
    TexCoords = pos.xyz / pos.w;
    gl_Position = vec4(aPos, 1.0);
}
