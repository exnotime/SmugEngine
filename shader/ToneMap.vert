#version 430

layout(location = 0) out vec2 TexCoord;
void main(){
    float x = -1.0 + float((gl_VertexIndex & 1) << 2);
    float y = -1.0 + float((gl_VertexIndex & 2) << 1);
    TexCoord.x = (x+1.0) * 0.5;
    TexCoord.y = (y+1.0) * 0.5;
    gl_Position = vec4(x, y, 0, 1);
}