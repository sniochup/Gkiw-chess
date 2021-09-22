#version 330

out vec4 pixelColor;

in vec4 ic;
in vec4 l;
in vec4 n;
in vec4 v;


void main(void) {

	vec4 ml = normalize(l);
	vec4 mn = normalize(n);
	vec4 mv = normalize(v);
	vec4 mr = reflect(-ml, mn);
	
	float nl = clamp(dot(mn, ml), 0, 1);
    float rv = pow(clamp(dot(mr,mv), 0, 1), 25);

	pixelColor = vec4(ic.rgb * nl, ic.a) + vec4(rv,rv,rv,0);
}
