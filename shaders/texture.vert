#version 330

layout(location=0) in vec4 in_Position;
layout(location=1) in vec2 in_UV;
out vec2 ex_UV;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main(void)
{
  gl_Position = (ProjectionMatrix * ViewMatrix * ModelMatrix) * in_Position;
  ex_UV = in_UV;
}
