#version 330

layout(location=0) in vec3 in_Position;
layout(location=1) in vec3 in_Normal;

out vec3 ex_Normal;
out vec3 ex_ViewDirection;

uniform mat4 MVP;
uniform mat4 ModelView;
uniform mat3 NormalMatrix;

void main(void) {
  ex_Normal = NormalMatrix * in_Normal;

  vec4 position = vec4(in_Position, 1.0);

  vec4 viewSpacePos = ModelView * position;
  vec3 viewSpacePosScaled = vec3(viewSpacePos) / viewSpacePos.w;
  ex_ViewDirection = -viewSpacePosScaled;

  gl_Position = MVP * position;
}
