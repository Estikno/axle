#version 460 core

layout (location = 0) in vec3 aPos;

out vec3 TexCoords;
uniform mat4 viewProjection;

void main(){
    vec4 pos = inverse(viewProjection) * vec4(aPos.xy, 1.0, 1.0);
    TexCoords = pos.xyz / pos.w;
    gl_Position = vec4(aPos, 1.0);
}
