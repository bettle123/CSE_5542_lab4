
#version 400

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 baseColor;

out vec3 Color;

uniform mat4 MVP;


void main()
{
    Color = baseColor;
    gl_Position = MVP * vec4(position, 1.0);
}
