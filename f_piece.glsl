#version 330

uniform sampler2D textureMap0;
uniform sampler2D textureMap1;

in vec4 l;
in vec4 l1;
in vec4 n;
in vec4 v;
in vec2 iTexCoord0;
in vec2 iTexCoord1;

out vec4 pixelColor;


void main(void) {
	vec4 ml = normalize(l);
	vec4 ml1 = normalize(l1);

	vec4 mn = normalize(n);
	vec4 mv = normalize(v);

	vec4 mr = reflect(-ml, mn);
	vec4 mr1 = reflect(-ml1, mn);
	
	vec4 kd = mix(texture(textureMap0, iTexCoord0), texture(textureMap1, iTexCoord1), 0.3);
	vec4 ks = vec4(1,1,1,1);

	float nl = clamp(dot(mn, ml), 0, 1);
	float nl1 = clamp(dot(mn, ml1), 0, 1);
    float rv = pow(clamp(dot(mr,mv), 0, 1), 50);
	float rv1 = pow(clamp(dot(mr1,mv), 0, 1), 50);

	pixelColor = vec4(kd.rgb * (nl + nl1), kd.a) + vec4(ks.rgb * (rv + rv1), 0);
}
