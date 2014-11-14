#version 130

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
