
vec3 TriLerp3(vec3 a, vec3 b, vec3 c, float u, float v){
    return (1.0 - u - v) * a + b * u + c * v;
}