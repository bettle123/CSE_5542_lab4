#version 400


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 baseColor;


out vec3 EntryPoint;
out vec4 ExitPointCoord;

uniform mat4 MVP;

void main()
{	
    EntryPoint = baseColor;
    gl_Position = MVP * vec4(position,1.0);
    ExitPointCoord = gl_Position;  
}
