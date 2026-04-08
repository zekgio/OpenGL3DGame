#version 450

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_color;
layout (location = 2) in vec2 vertex_texcoord; 
layout (location = 3) in vec3 vertex_normal;

out vec3 vs_color;
out vec2 vs_texcoord;
out vec3 vs_normal;

uniform mat4 ModelMatrix;
uniform float aspectRatio; 

void main() {
    vs_color    = vertex_color;
    vs_texcoord = vertex_texcoord;
    vs_normal   = vertex_normal;
    
    // ModelMatrix is responsible for "movement"
    vec4 pos = ModelMatrix * vec4(vertex_position, 1.0);
    
    gl_Position = vec4(pos.x * aspectRatio, pos.y, pos.z, 1.0);
}