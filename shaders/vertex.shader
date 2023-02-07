#version 450

layout (location = 0) in vec3 in_position;

layout (binding = 0)
uniform UniformBuffer {
    vec4 color;
    mat4 transformation;
} uniform_buffer;

void main()
{
    gl_Position = uniform_buffer.transformation * vec4(in_position, 1.0);
}