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
uniform sampler2D s_lut;

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

    mediump vec4 v_lut;

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

        v_lut = texture(s_lut, lut_texCoord);

        int select_num = int(v_lut.b);
        float alpha = v_lut.b - float(select_num);

        switch (select_num)
        {
            case 1:
            {
                int index = int(v_lut.r);
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
                int index1 = int(v_lut.r);
                int index2 = int(v_lut.g);
                if (index1 < image_size && index2 < image_size)
                {
                    vec2 texCoord1 = lutIndex2TextureCoord(index1);
                    yuv_front.x = 1.1643 * (texture(s_frontY, texCoord1).r - 0.0625);
                    yuv_front.y = texture(s_frontUV, texCoord1).a - 0.5;
                    yuv_front.z = texture(s_frontUV, texCoord1).r - 0.5;

                    vec2 texCoord2 = lutIndex2TextureCoord(index2);
                    yuv_right.x = 1.1643 * (texture(s_rightY, texCoord2).r - 0.0625);
                    yuv_right.y = texture(s_rightUV, texCoord2).a - 0.5;
                    yuv_right.z = texture(s_rightUV, texCoord2).r - 0.5;

                    rgb = alpha*(yuv2rgb * yuv_front) + (1.0 - alpha)*(yuv2rgb * yuv_right);
                }
                break;
            }
            case 3:
            {
                int index = int(v_lut.g);
                if (index < image_size)
                {
                    vec2 texCoord = lutIndex2TextureCoord(index);
                    yuv_right.x = 1.1643 * (texture(s_rightY, texCoord).r - 0.0625);
                    yuv_right.y = texture(s_rightUV, texCoord).a - 0.5;
                    yuv_right.z = texture(s_rightUV, texCoord).r - 0.5;
                    rgb = yuv2rgb * yuv_right;
                }

                break;
            }
            case 4:
            {
                int index1 = int(v_lut.r);
                int index2 = int(v_lut.g);
                if (index1 < image_size && index2 < image_size)
                {
                    vec2 texCoord1 = lutIndex2TextureCoord(index1);
                    yuv_rear.x = 1.1643 * (texture(s_rearY, texCoord1).r - 0.0625);
                    yuv_rear.y = texture(s_rearUV, texCoord1).a - 0.5;
                    yuv_rear.z = texture(s_rearUV, texCoord1).r - 0.5;

                    vec2 texCoord2 = lutIndex2TextureCoord(index2);
                    yuv_right.x = 1.1643 * (texture(s_rightY, texCoord2).r - 0.0625);
                    yuv_right.y = texture(s_rightUV, texCoord2).a - 0.5;
                    yuv_right.z = texture(s_rightUV, texCoord2).r - 0.5;

                    rgb = alpha*(yuv2rgb * yuv_rear) + (1.0 - alpha)*(yuv2rgb * yuv_right);
                }
                break;
            }
            case 5:
            {
                int index = int(v_lut.r);
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
                int index1 = int(v_lut.r);
                int index2 = int(v_lut.g);
                if (index1 < image_size && index2 < image_size)
                {
                    vec2 texCoord1 = lutIndex2TextureCoord(index1);
                    yuv_rear.x = 1.1643 * (texture(s_rearY, texCoord1).r - 0.0625);
                    yuv_rear.y = texture(s_rearUV, texCoord1).a - 0.5;
                    yuv_rear.z = texture(s_rearUV, texCoord1).r - 0.5;

                    vec2 texCoord2 = lutIndex2TextureCoord(index2);
                    yuv_left.x = 1.1643 * (texture(s_leftY, texCoord2).r - 0.0625);
                    yuv_left.y = texture(s_leftUV, texCoord2).a - 0.5;
                    yuv_left.z = texture(s_leftUV, texCoord2).r - 0.5;

                    rgb = alpha*(yuv2rgb * yuv_rear) + (1.0 - alpha)*(yuv2rgb * yuv_left);
                }
                break;
            }
            case 7:
            {
                int index = int(v_lut.g);
                if (index < image_size)
                {
                    vec2 texCoord = lutIndex2TextureCoord(index);
                    yuv_left.x = 1.1643 * (texture(s_leftY, texCoord).r - 0.0625);
                    yuv_left.y = texture(s_leftUV, texCoord).a - 0.5;
                    yuv_left.z = texture(s_leftUV, texCoord).r - 0.5;
                    rgb = yuv2rgb * yuv_left;
                }
                break;
            }
            case 8:
            {
                int index1 = int(v_lut.r);
                int index2 = int(v_lut.g);
                if (index1 < image_size && index2 < image_size)
                {
                    vec2 texCoord1 = lutIndex2TextureCoord(index1);
                    yuv_front.x = 1.1643 * (texture(s_frontY, texCoord1).r - 0.0625);
                    yuv_front.y = texture(s_frontUV, texCoord1).a - 0.5;
                    yuv_front.z = texture(s_frontUV, texCoord1).r - 0.5;

                    vec2 texCoord2 = lutIndex2TextureCoord(index2);
                    yuv_left.x = 1.1643 * (texture(s_leftY, texCoord2).r - 0.0625);
                    yuv_left.y = texture(s_leftUV, texCoord2).a - 0.5;
                    yuv_left.z = texture(s_leftUV, texCoord2).r - 0.5;

                    rgb = alpha*(yuv2rgb * yuv_front) + (1.0 - alpha)*(yuv2rgb * yuv_left);
                }
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
