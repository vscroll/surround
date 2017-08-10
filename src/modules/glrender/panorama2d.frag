#if 1

static const char gFShaderStr[] = STRINGIFY(
precision mediump float;
uniform float lookupTabFront[508800];
uniform float lookupTabRear[508800];
uniform float lookupTabLeft[508800];
uniform float lookupTabRight[508800];
uniform int mask[508800];
uniform float weight[508800];
varying vec2 v_texCoord;
varying float v_s;
varying float v_t;
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
    mat3 yuv2rgb = mat3( 1, 1, 1,
                        0, -0.39465, 2.03211,
                        1.13983, -0.58060, 0);
    mediump vec3 yuv_front;
    mediump vec3 yuv_rear;
    mediump vec3 yuv_left;
    mediump vec3 yuv_right;
    mediump vec3 yuv_focus;
    yuv_front.x = texture2D(s_frontY, v_texCoord).r;
    yuv_front.y = texture2D(s_frontU, v_texCoord).r - 0.5;
    yuv_front.z = texture2D(s_frontV, v_texCoord).r - 0.5;
    yuv_rear.x = texture2D(s_rearY, v_texCoord).r;
    yuv_rear.y = texture2D(s_rearU, v_texCoord).r - 0.5;
    yuv_rear.z = texture2D(s_rearV, v_texCoord).r - 0.5;
    yuv_left.x = texture2D(s_leftY, v_texCoord).r;
    yuv_left.y = texture2D(s_leftU, v_texCoord).r - 0.5;
    yuv_left.z = texture2D(s_leftV, v_texCoord).r - 0.5;
    yuv_right.x = texture2D(s_rightY, v_texCoord).r;
    yuv_right.y = texture2D(s_rightU, v_texCoord).r - 0.5;
    yuv_right.z = texture2D(s_rightV, v_texCoord).r - 0.5;
    yuv_focus.x = texture2D(s_focusY, v_texCoord).r;
    yuv_focus.y = texture2D(s_focusU, v_texCoord).r - 0.5;
    yuv_focus.z = texture2D(s_focusV, v_texCoord).r - 0.5;
    lowp vec3 rgb;
    if (v_s < 0.5)
    {
        rgb = yuv2rgb * yuv_front;
    }
    else
    {
        rgb = yuv2rgb * yuv_focus;
    }
    gl_FragColor = vec4(rgb, 1);
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
