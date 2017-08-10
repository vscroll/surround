#if 1

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

#else

//this fragment shader cannot work
static const char gFShaderStr[] = STRINGIFY(
precision mediump float;
varying vec2 v_texCoord;
uniform sampler2D s_frontY;
uniform sampler2D s_frontU;
uniform sampler2D s_frontV;
uniform sampler2D s_rearY;
uniform sampler2D s_rearU;
uniform sampler2D s_rearV;
uniform sampler2D s_leftY;
uniform sampler2D s_leftU;
uniform sampler2D s_leftV;
uniform sampler2D s_rightY;
uniform sampler2D s_rightU;
uniform sampler2D s_rightV;
uniform sampler2D s_focusY;
uniform sampler2D s_focusU;
uniform sampler2D s_focusV;
void main()
{
    mediump vec3 yuv;
    vec4 color;
    yuv.x = texture2D(s_frontY, v_texCoord).r;
    yuv.y = texture2D(s_frontU, v_texCoord).r - 0.5;
    yuv.z = texture2D(s_frontV, v_texCoord).r - 0.5;
    color.r = yuv.r + 1.4022*yuv.b - 0.7011;
    color.r = (color.r < 0.0) ? 0.0 : ((color.r > 1.0) ? 1.0 : color.r);
    color.g = yuv.r - 0.3456*yuv.g - 0.7145*yuv.b + 0.53005;
    color.g = (color.g < 0.0) ? 0.0 : ((color.g > 1.0) ? 1.0 : color.g);
    color.b = yuv.r + 1.771*yuv.g - 0.8855;
    color.b = (color.b < 0.0) ? 0.0 : ((color.b > 1.0) ? 1.0 : color.b);
    gl_FragColor = color;
}
);

#endif
