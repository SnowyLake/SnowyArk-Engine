#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (binding = 0) uniform ARK_PerDraw
{
	mat4 modelMat;
	mat4 viewMat;
    mat4 projMat;
} perDraw;

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec3 inColor;

layout (location = 0) out vec3 fragColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    gl_Position = perDraw.projMat * perDraw.viewMat * perDraw.modelMat * vec4(inPosition, 0.0, 1.0);
    fragColor = inColor;
}