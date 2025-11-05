#version 460 
#pragma shader_stage(fragment)

layout(location = 0) in vec3 i_Position;
layout(location = 1) in vec3 i_Normal;
layout(location = 2) in vec2 i_Texcoord;

layout(location = 0) out vec4 o_Color;

layout(set = 0,binding = 0) uniform sampler2D u_Texture;

void main(){
	vec4 texColor = texture(u_Texture, i_Texcoord);
	o_Color = texColor;
}