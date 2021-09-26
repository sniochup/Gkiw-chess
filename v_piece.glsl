#version 330

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec4 lp;
uniform vec4 lp1;

in vec4 vertex;
in vec4 normal;
in vec2 texCoord0;

out vec4 l;
out vec4 l1;
out vec4 n;
out vec4 v;
out vec2 iTexCoord0;
out vec2 iTexCoord1;


void main(void) {
  l = normalize(V*lp - V*M*vertex);
  l1 = normalize(V*lp1 - V*M*vertex);

  n = normalize(V*M*normal);
  v = normalize(vec4(0, 0, 0, 1)-V*M*vertex);

  iTexCoord0 = texCoord0;
  iTexCoord1 = (n.xy+1)/2;

  gl_Position=P*V*M*vertex;
}