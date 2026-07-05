#version 460 core

layout(std140, binding = 0) uniform PerFrameData{
    uniform mat4 MVP;
};

struct Vertex{
    float p[3];
    float tc[2];
};

layout(std430, binding = 1) restrict readonly buffer Vertices {
    Vertex inVertices[];
};

vec3 getPosition(int i){
    return vec3(inVertices[i].p[0], inVertices[i].p[1], inVertices[i].p[2]);
}

vec2 getTexCoord(int i){
    return vec2(inVertices[i].tc[0], inVertices[i].tc[1]);
}

layout (location = 0) out vec2 uv;

void main(){
    vec3 pos = getPosition(gl_VertexID);
    gl_Position = MVP * vec4(pos, 1.0);

    uv = getTexCoord(gl_VertexID);
}
