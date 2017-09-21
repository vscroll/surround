#version 300 es

precision mediump float;

const int screen_width = 1024;
const int screen_height = 600;
const int panorama_width = 424;
const int panorama_height = 600;
const int image_width = 704;
const int image_height = 574;
const int image_size = image_width*image_height;

in vec2 v_texCoord;
uniform sampler2D s_frontY;
uniform sampler2D s_frontUV;
uniform sampler2D s_rearY;
uniform sampler2D s_rearUV;
uniform sampler2D s_leftY;
uniform sampler2D s_leftUV;
uniform sampler2D s_rightY;
uniform sampler2D s_rightUV;
uniform sampler2D s_mask;
uniform sampler2D s_lutHor;
uniform sampler2D s_lutVer;
out vec4 o_fragColor;

//screen coordinates: left bottom
//image coordinates: left top
//texture coordinates: left bottom
vec2 lutIndex2TextureCoord(int index)
{
    int y_image = index/image_width; 
    int x_image = index - y_image*image_width;
    float s = float(x_image)/float(image_width);
    float t = float(y_image)/float(image_height);
    return vec2(s,t);
}

void main()
{
    mediump vec3 yuv_front;
    mediump vec3 yuv_rear;
    mediump vec3 yuv_left;
    mediump vec3 yuv_right;

    mediump vec4 mask;
    mediump vec4 lut_hor;
    mediump vec4 lut_ver;

    lowp vec3 rgb;

    int x_screen = int(gl_FragCoord.x);
    int y_screen = int(gl_FragCoord.y);

    mat3 yuv2rgb = mat3(1, 0, 1.2802,  
                        1, -0.214821, -0.380589,  
                        1, 2.127982, 0); 

    if (x_screen < panorama_width)
    {
        int x_panoram = x_screen;
        int y_panoram = screen_height - y_screen - 1;
        float lut_s = float(x_panoram)/float(panorama_width);
        float lut_t = float(y_panoram)/float(panorama_height);
        vec2 lut_texCoord = vec2(lut_s, lut_t);

        mask = texture(s_mask, lut_texCoord);
        lut_hor = texture(s_lutHor, lut_texCoord);
#if 0
        lut_ver = texture(s_lutVer, lut_texCoord);
#endif
        int select_num = int(mask.r);
        float alpha = mask.r - float(select_num);

        switch (select_num)
        {
            case 1:
            {
                int index = int(lut_hor.r);
                if (index < image_size)
                {
                    vec2 texCoord = lutIndex2TextureCoord(index);
                    yuv_front.x = 1.1643 * (texture(s_frontY, texCoord).r - 0.0625);
                    yuv_front.y = texture(s_frontUV, texCoord).a - 0.5;
                    yuv_front.z = texture(s_frontUV, texCoord).r - 0.5;
                    rgb = yuv2rgb * yuv_front;
                }
                break;
            }
            case 2:
            {
                rgb = vec3(1.0, 1.0, 0.5);
                break;
            }
            case 3:
            {
#if 0
                int index = int(lut_ver.r);
                if (index < image_size)
                {
                    vec2 texCoord = lutIndex2TextureCoord(index);
                    yuv_right.x = 1.1643 * (texture(s_rightY, texCoord).r - 0.0625);
                    yuv_right.y = texture(s_rightUV, texCoord).a - 0.5;
                    yuv_right.z = texture(s_rightUV, texCoord).r - 0.5;
                    rgb = yuv2rgb * yuv_right;
                }
#else
                rgb = vec3(1.0, 1.0, 1.0);
#endif
                break;
            }
            case 4:
            {
                rgb = vec3(0.5, 1.0, 1.0);
                break;
            }
            case 5:
            {
                int index = int(lut_hor.r);
                if (index < image_size)
                {
                    vec2 texCoord = lutIndex2TextureCoord(index);
                    yuv_rear.x = 1.1643 * (texture(s_rearY, texCoord).r - 0.0625);
                    yuv_rear.y = texture(s_rearUV, texCoord).a - 0.5;
                    yuv_rear.z = texture(s_rearUV, texCoord).r - 0.5;
                    rgb = yuv2rgb * yuv_rear;
                }
                break;
            }
            case 6:
            {
                rgb = vec3(0.0, 0.5, 1.0);
                break;
            }
            case 7:
            {
#if 0
                int index = int(lut_ver.r);
                if (index < image_size)
                {
                    vec2 texCoord = lutIndex2TextureCoord(index);
                    yuv_left.x = 1.1643 * (texture(s_leftY, texCoord).r - 0.0625);
                    yuv_left.y = texture(s_leftUV, texCoord).a - 0.5;
                    yuv_left.z = texture(s_leftUV, texCoord).r - 0.5;
                    rgb = yuv2rgb * yuv_left;
                }
#else
                rgb = vec3(0.0, 0.0, 1.0);
#endif
                break;
            }
            case 8:
            {
                rgb = vec3(0.0, 0.0, 0.5);
                break;
            }
            default:
            {
                rgb = vec3(0.0, 0.0, 0.0);
                break;
            }
        }
    }
    else
    {
        int y_image = screen_height - y_screen - 1;
        if (y_image >= image_height)
        {
            return;
        }

        int x_image = x_screen - panorama_width;
        float s = float(x_image)/float(image_width);
        float t = float(y_image)/float(image_height);
        vec2 texCoord = vec2(s, t);
        yuv_front.x = 1.1643 * (texture(s_frontY, texCoord).r - 0.0625);
        yuv_front.y = texture(s_frontUV, texCoord).a - 0.5;
        yuv_front.z = texture(s_frontUV, texCoord).r - 0.5;
        rgb = yuv2rgb * yuv_front;

    }

    //rgb = vec3(1.0, 0.0, 0.0);
    o_fragColor = vec4(rgb, 1);
}
