#if defined(VERTEX)

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

#endif

#if defined(FRAGMENT)

in vec3 ex_Normal;
in vec3 ex_ViewDirection;

out vec4 out_Color;

const vec3 lightDir = vec3(1.0, 1.0, 1.0);
const vec3 ambientColor = vec3(0.1, 0.0, 0.0);
const vec3 diffuseColor = vec3(0.5, 0.0, 0.0);

void main(void) {
  vec3 normal = normalize(ex_Normal);
  vec3 light_direction = normalize(lightDir);

  float diffuse = max(0.0, dot(normal, light_direction));
  vec3 halfDir = normalize(light_direction + normalize(ex_ViewDirection));
  float nh = max(0.0, dot(normal, halfDir));
  float specular = pow(nh, 16.0);

  out_Color = vec4(ambientColor + diffuse * diffuseColor + specular, 1.0);
}

#endif
