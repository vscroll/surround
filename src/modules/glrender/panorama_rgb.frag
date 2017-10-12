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
uniform sampler2D s_front;
uniform sampler2D s_rear;
uniform sampler2D s_left;
uniform sampler2D s_right;
uniform sampler2D s_lut;
uniform int u_panorama_view;

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
    mediump vec4 v_lut;
    lowp vec4 rgb;

    int x_screen = int(gl_FragCoord.x);
    int y_screen = int(gl_FragCoord.y);

    if (u_panorama_view == -1)
    {
        int display_width = screen_width/2;
        int display_height = screen_height/2;
 
        if (x_screen < display_width)
        {
            if (y_screen < display_height)
            {
                int y_image = display_height - y_screen - 1;
                int x_image = x_screen;
                int display_y = y_image*image_height/display_height;
                int display_x = x_image*image_width/display_width;
                float s = float(display_x)/float(image_width);
                float t = float(display_y)/float(image_height);
                vec2 texCoord = vec2(s, t);
                rgb = texture(s_rear, texCoord);
            }
            else
            {
                int y_image = display_height - (y_screen - display_height) - 1;
                int x_image = x_screen;
                int display_y = y_image*image_height/display_height;
                int display_x = x_image*image_width/display_width;
                float s = float(display_x)/float(image_width);
                float t = float(display_y)/float(image_height);
                vec2 texCoord = vec2(s, t);
                rgb = texture(s_front, texCoord);
            }
        }
        else
        {
            if (y_screen < display_height)
            {
                int y_image = display_height - y_screen - 1;
                int x_image = x_screen - display_width;
                int display_y = y_image*image_height/display_height;
                int display_x = x_image*image_width/display_width;
                float s = float(display_x)/float(image_width);
                float t = float(display_y)/float(image_height);
                vec2 texCoord = vec2(s, t);
                rgb = texture(s_right, texCoord);
            }
            else
            {
                int y_image = display_height - (y_screen - display_height) - 1;
                int x_image = x_screen - display_width;
                int display_y = y_image*image_height/display_height;
                int display_x = x_image*image_width/display_width;
                float s = float(display_x)/float(image_width);
                float t = float(display_y)/float(image_height);
                vec2 texCoord = vec2(s, t);
                rgb = texture(s_left, texCoord);
            }
        }       

        o_fragColor = rgb;
        return;
    }

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
                    rgb = texture(s_front, texCoord);
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
                    lowp vec4 rgb1 = texture(s_front, texCoord1);

                    vec2 texCoord2 = lutIndex2TextureCoord(index2);
                    lowp vec4 rgb2 = texture(s_right, texCoord2);

                    rgb = alpha*rgb1 + (1.0 - alpha)*rgb2;
                }
                break;
            }
            case 3:
            {
                int index = int(v_lut.g);
                if (index < image_size)
                {
                    vec2 texCoord = lutIndex2TextureCoord(index);
                    rgb = texture(s_right, texCoord);
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
                    lowp vec4 rgb1 = texture(s_rear, texCoord1);

                    vec2 texCoord2 = lutIndex2TextureCoord(index2);
                    lowp vec4 rgb2 = texture(s_right, texCoord2);

                    rgb = alpha*rgb1 + (1.0 - alpha)*rgb2;
                }
                break;
            }
            case 5:
            {
                int index = int(v_lut.r);
                if (index < image_size)
                {
                    vec2 texCoord = lutIndex2TextureCoord(index);
                    rgb = texture(s_rear, texCoord);
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
                    lowp vec4 rgb1 = texture(s_rear, texCoord1);

                    vec2 texCoord2 = lutIndex2TextureCoord(index2);
                    lowp vec4 rgb2 = texture(s_left, texCoord2);

                    rgb = alpha*rgb1 + (1.0 - alpha)*rgb2;
                }
                break;
            }
            case 7:
            {
                int index = int(v_lut.g);
                if (index < image_size)
                {
                    vec2 texCoord = lutIndex2TextureCoord(index);
                    rgb = texture(s_left, texCoord);
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
                    lowp vec4 rgb1 = texture(s_front, texCoord1);

                    vec2 texCoord2 = lutIndex2TextureCoord(index2);
                    lowp vec4 rgb2 = texture(s_left, texCoord2);

                    rgb = alpha*rgb1 + (1.0 - alpha)*rgb2;
                }
                break;
            }
            default:
            {
                rgb = vec4(0.0, 0.0, 0.0, 1.0);
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
        rgb = texture(s_front, texCoord);

    }

    o_fragColor = rgb;
}
