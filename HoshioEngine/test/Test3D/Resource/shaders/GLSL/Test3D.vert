#version 460
#pragma shader_stage(vertex)

layout(location = 0) in vec3 i_Position;        //逐顶点
layout(location = 1) in vec4 i_Color;           //逐顶点
layout(location = 2) in vec3 i_InstancePosition; //逐实例
layout(location = 0) out vec4 o_Color;
layout(push_constant) uniform pushConstants {
    mat4 proj; //投影矩阵
    mat4 view;
};

void main() {
    gl_Position = proj * view * vec4(i_Position + i_InstancePosition, 1);
    o_Color = i_Color;
}