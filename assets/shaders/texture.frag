#version 330

in vec2 ex_UV;
out vec3 out_Color;

uniform sampler2D texSampler;

uniform vec4 color1;
uniform vec4 color2;
uniform vec4 color3;
uniform vec4 color4;

void main(void)
{
  float height = texture(texSampler, ex_UV).r;

  if (height <= color3.a) {
    if (height <= color2.a) {
      out_Color = mix(color1.rgb, color2.rgb, (height - color1.a) / (color2.a - color1.a));
    } else {
      out_Color = mix(color2.rgb, color3.rgb, (height - color2.a) / (color3.a - color2.a));
    }
  } else {
    out_Color = mix(color3.rgb, color4.rgb, (height - color3.a) / (color4.a - color3.a));
  }
}
