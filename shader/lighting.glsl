
float LambertDiffuse(vec3 normal, vec3 lightDir){
    float ndotl = dot(normal, -lightDir);
    return max(0,ndotl);
}

float BlinnSpecular(vec3 normal, vec3 lightDir, vec3 toEye, float shiny){
    vec3 reflected = reflect(lightDir, normal);
    float vdotr = max(0, dot(reflected, toEye));
    return pow(vdotr, shiny);
}
