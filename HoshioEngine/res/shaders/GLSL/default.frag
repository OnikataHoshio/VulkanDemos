#version 460 
#pragma shader_stage(fragment)

layout(location = 0) in vec2 i_Texcoord;
layout(location = 0) out vec4 o_Color;
layout(set = 0,binding = 0) uniform sampler2D u_Texture;

layout(push_constant) uniform pushConstant{
	bool enableLod;
	float lod;
};

void main(){
	vec4 color = vec4(0.0f);
	if(enableLod)
		color = textureLod(u_Texture,i_Texcoord, lod);
	else
		color = textureLod(u_Texture,i_Texcoord, 0.0f);
	o_Color = color;
}
