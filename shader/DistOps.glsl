
//geometry

float sdSphere(vec3 p, float radius, vec3 o) {
	return length(p - o) - radius;
}

float udPlane(vec3 p, vec4 n) {
	return abs(dot(p, n.xyz) + n.w);
}

float udBox(vec3 p, vec3 b, vec3 o){
    return length(max(abs(p - o) - b, 0));
}

float udRoundBox(vec3 p, vec3 b, float r, vec3 o){
    return length(max(abs(p - o) - b, 0)) - r;
}

//operations
vec3 opRepeat(vec3 p, vec3 r){
	p.x = mod(p.x, r.x) - r.x * 0.5;
	p.z = mod(p.z, r.z) - r.z * 0.5;
    return p;
}

float opUnion(float d1, float d2){
    return min(d1, d2);
}

float opSubtract(float d1, float d2){
	return max(-d1, d2);
}

float opIntersect(float d1, float d2){
    return max(d1,d2);
}

vec3 opTwist( vec3 p, float t) {
    float c = cos(t*p.y);
    float s = sin(t*p.y);
    mat2  m = mat2(c,-s,s,c);
    return vec3(m*p.xz,p.y);
}

float smin( float a, float b, float k ) {
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}

float opBlend( float d1, float d2 ) {
    return smin( d1, d2, 0.1);
}

float Apollonian(vec3 p, float s){
	float scale = 1.0;
	vec4 orb = vec4(1000.0);
	for( int i=0; i<8;i++ ) {
		p = -1.0 + 2.0*fract(0.5*p+0.5);
		float r2 = dot(p,p);
        orb = min( orb, vec4(abs(p),r2) );
		float k = s/r2;
		p     *= k;
		scale *= k;
	}

	return 0.25*abs(p.y)/scale;
}
