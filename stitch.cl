__kernel void stitch_2d	(__global uchar* image_front,
                                __global uchar* image_rear,
                                __global uchar* image_left,
                                __global uchar* image_right,
                                int side_width,
                                __global uchar* mask,
                                __global int *map_x,
                                __global int *map_y,
                                __global uchar* pano2d,
                                int pano2d_width)
{
    // gets the global id
    int col = get_global_id(0);
    int row = get_global_id(1);

    // copy the input to output
    int id = row*pano2d_width+col;
    int flag = mask[id];
    int x = map_x[id];
    int y = map_y[id];

    int in_id = y*side_width*3+x*3;
    int out_id = row*pano2d_width*3+col*3;
    switch (flag)
    {
    case 50:
        {
            pano2d[out_id] = image_front[in_id];
            pano2d[out_id+1] = image_front[in_id+1];
            pano2d[out_id+2] = image_front[in_id+2];
            break;
        }
    case 100:
        {
            pano2d[out_id] = image_right[in_id];
            pano2d[out_id+1] = image_right[in_id+1];
            pano2d[out_id+2] = image_right[in_id+2];
            break;
        }
    case 150:
        {
            pano2d[out_id] = image_left[in_id];
            pano2d[out_id+1] = image_left[in_id+1];
            pano2d[out_id+2] = image_left[in_id+2];
            break;
        }
    case 200:
        {
            pano2d[out_id] = image_rear[in_id];
            pano2d[out_id+1] = image_rear[in_id+1];
            pano2d[out_id+2] = image_rear[in_id+2];
            break;
        }
    default:
        break;
    }
}
