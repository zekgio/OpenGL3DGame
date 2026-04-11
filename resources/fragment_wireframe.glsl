#version 450

out vec4 fs_color;

void main() {
    // Just black for wireframe
    fs_color = vec4(0.0, 0.0, 0.0, 1.0);
}