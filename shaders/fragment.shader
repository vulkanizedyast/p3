#version 450

layout (location = 0) out vec4 out_color;

layout (binding = 0)
uniform UniformBuffer {
    vec4 color;
    mat4 transformation;
} uniform_buffer;

void main()
{
    out_color = uniform_buffer.color;
}