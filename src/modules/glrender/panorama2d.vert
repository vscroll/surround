static const char gVShaderStr[] = STRINGIFY(
attribute vec4 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
varying float v_s;
varying float v_t;
void main()
{
    vec4 position;
    gl_Position = a_position;
    position = a_position/a_position.w;
    v_s = (position.s + 1.0)/2.0;
    v_t = (position.t + 1.0)/2.0;
    v_texCoord = a_texCoord;
}
);
