#version 330

uniform mat4 P;
uniform mat4 V;

in vec3 aPos;

out vec3 TexCoords;

void main()
{
    TexCoords = aPos;
    vec4 pos = P * V * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  