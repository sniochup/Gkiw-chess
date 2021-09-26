#version 330

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec4 lp;
uniform vec4 lp1;

in vec4 vertex;
in vec2 texCoord0;
in vec4 c1;
in vec4 c2;
in vec4 c3;

out vec4 l;
out vec4 l1;
out vec4 v;
out vec2 iTexCoord0;


void main(void) {
    mat4 invTBN = mat4(c1,c2,c3,vec4(0,0,0,1));

    l = normalize(invTBN*inverse(M)*lp - invTBN*vertex);
    l1 = normalize(invTBN*inverse(M)*lp1 - invTBN*vertex);

    v = normalize(invTBN*inverse(V*M)*vec4(0, 0, 0, 1) - invTBN*vertex);

    iTexCoord0 = texCoord0;
    
    gl_Position=P*V*M*vertex;
}
