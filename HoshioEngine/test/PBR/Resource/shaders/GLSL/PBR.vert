#version 460
#pragma shader_stage(vertex)

layout(location = 0) in vec3 i_Position;        //逐顶点
layout(location = 1) in vec4 i_Color;           //逐顶点
layout(location = 0) out vec4 o_Color;
layout(push_constant) uniform pushConstants {
    mat4 proj; //投影矩阵
    mat4 view;
    mat4 model;
};

void main() {
    gl_Position = proj * view * model * vec4(i_Position + i_InstancePosition, 1);
    o_Color = i_Color;
}