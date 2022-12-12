#version 400 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;

uniform mat4 viewProjMatrix = mat4(1);

out Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec3 position;
} OUT;

void main(void)
{
	vec4 temp 		= viewProjMatrix * vec4(position, 1.0);
	gl_Position		= temp;
	OUT.texCoord	= texCoord;
	OUT.colour		= colour;
	OUT.position 	= vec3(temp.x, temp.y, temp.z);
	//OUT.position 	= position;
}