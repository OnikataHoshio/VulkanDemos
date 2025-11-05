#version 460 core
#pragma shader_stage(vertex)

layout(location = 0) in vec2 i_Position;
layout(location = 1) in vec2 i_Texcoord;

layout(location = 0) out vec2 o_Texcoord;

void main(){
	gl_Position = vec4(i_Position,0.0f,1.0f);
	o_Texcoord = i_Texcoord;
}