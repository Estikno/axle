#version 460 core

out vec4 FragColor;

in vec2 TexCoords;
layout (binding = 0) uniform sampler2D tex;

void main(){
    FragColor = texture(tex, TexCoords);
}
