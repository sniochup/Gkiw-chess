#version 330

uniform sampler2D textureMap0;
uniform sampler2D textureMap1;
uniform sampler2D textureMap2;
uniform sampler2D textureMap3;
uniform sampler2D textureMap4;

in vec4 l;
in vec4 l1;
in vec4 v;
in vec2 iTexCoord0;

out vec4 pixelColor;


vec2 parallaxTexCoords (vec4 v, vec2 t, float h, float s) {
	vec2 ti = -h*v.xy / s;
	float hi = -1 / s;

	vec2 tc = t;
	float hc = 1;
	float ht = texture(textureMap2, tc).r;

	if (v.z<=0) discard;

	while(hc>ht) {
		tc = tc + ti;
		if (tc.x<0 || tc.x>1 || tc.y<0|| tc.y>1) discard;
		hc = hc + hi;
		ht = texture(textureMap2, tc).r;
	}

	vec2 tco = tc - ti;
	float hco = hc - hi;
	float hto = texture(textureMap2, tco).r;

	float x = (hco-hto)/(hco-hto-(hc-ht));

	return (1-x)*tco+x*tc;
}

void main(void) {

	vec4 mv = normalize(v);
	vec2 nt = parallaxTexCoords(mv, iTexCoord0, 0.1, 100);

	vec4 ml = normalize(l);
	vec4 ml1 = normalize(l1);

	vec4 mn = normalize(vec4(texture(textureMap1, nt).rgb*2-1,0));

	vec4 mr = reflect(-ml, mn);
	vec4 mr1 = reflect(-ml1, mn);

	vec2 iTexCoord1 = (mn.xy+1)/2;

	vec4 kd = mix(texture(textureMap0, nt), texture(textureMap4, iTexCoord1), 0.3);
	vec4 ks = texture(textureMap3, nt);

	float nl = clamp(dot(mn, ml), 0, 1);
	float nl1 = clamp(dot(mn, ml1), 0, 1);
	float rv = pow(clamp(dot(mr, mv), 0, 1), 50);
	float rv1 = pow(clamp(dot(mr1, mv), 0, 1), 50);
	
	pixelColor = vec4(kd.rgb * (nl + nl1), kd.a) + vec4(ks.rgb * (rv + rv1), 0);
}
