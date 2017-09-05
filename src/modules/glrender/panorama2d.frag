static const char gFShaderStr[] = STRINGIFY(
precision mediump float;
const int pano_width = 424;
const int pano_height = 600;
const int size = pano_width*pano_height;
uniform float lookupTabHor[size];
uniform float lookupTabVer[size];
uniform float mask[size];
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
    yuv_left.x = texture2D(s_leftY, v_texCoord).r;
    yuv_left.y = texture2D(s_leftU, v_texCoord).r - 0.5;
    yuv_left.z = texture2D(s_leftV, v_texCoord).r - 0.5;
    yuv_right.x = texture2D(s_rightY, v_texCoord).r;
    yuv_right.y = texture2D(s_rightU, v_texCoord).r - 0.5;
    yuv_right.z = texture2D(s_rightV, v_texCoord).r - 0.5;
    lowp vec3 rgb;
#if 1
    int x = int(gl_FragCoord.x);
    int y = int(gl_FragCoord.y);
    if (x < 512)
    {
        yuv_rear.x = texture2D(s_rearY, v_texCoord).r;
        yuv_rear.y = texture2D(s_rearU, v_texCoord).r - 0.5;
        yuv_rear.z = texture2D(s_rearV, v_texCoord).r - 0.5;
        rgb = yuv2rgb * yuv_rear;
    }
    else
    {
        vec2 tc_offset = vec2(0.5, 0.0);
        yuv_focus.x = texture2D(s_focusY, v_texCoord.xy - tc_offset).r;
        yuv_focus.y = texture2D(s_focusU, v_texCoord.xy - tc_offset).r - 0.5;
        yuv_focus.z = texture2D(s_focusV, v_texCoord.xy- tc_offset).r - 0.5;
        rgb = yuv2rgb * yuv_focus;
        //rgb = vec3(1.0, 0.0, 0.0);
    }
#endif

    //rgb = yuv2rgb * yuv_front;
    gl_FragColor = vec4(rgb, 1);
}
);
