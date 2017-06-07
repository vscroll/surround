__kernel void stitch_2d	(__global uchar* image_front,
                        __global uchar* image_rear,
                        __global uchar* image_left,
                        __global uchar* image_right,
                        int side_width,
                        int side_height,
                        __global float* lut_front,
                        __global float* lut_rear,
                        __global float* lut_left,
                        __global float* lut_right,
                        __global uchar* mask,
                        __global float* weight,
                        int pano_width,
                        int pano_height,
                        __global uchar* pano)
{
    // gets the global id
    int col = get_global_id(0);
    int row = get_global_id(1);
    if (col >= pano_width*2
            || row >= pano_height)
    {
        return;
    }

    // copy the input to output
    int id = row*pano_width*2+col;
    int flag = mask[id];
   
    switch (flag)
    {
    case 0:
        {
            float w = weight[id];
            int index1 = lut_front[id];
            int index2 = lut_left[id];
            pano[id] = w*image_front[index1] + (1- w)*image_left[index2];
            break;
        }
    case 1:
        {
            int index1 = lut_front[id];
            pano[id] = image_front[index1];
            break;
        }
    case 2:
        {
            float w = weight[id];
            int index1 = lut_front[id];
            int index2 = lut_right[id];
            pano[id] = w*image_front[index1] + (1 - w)*image_right[index2];
            break;
        }
    case 3:
        {
            int index1 = lut_left[id];
            pano[id] = image_left[index1];
            break;
        }
    case 4:
        {
            pano[id] = 0;
            break;
        }
    case 5:
        {
            int index1 = lut_right[id];
            pano[id] = image_right[index1];
            break;
        }
    case 6:
        {
            float w = weight[id];
            int index1 = lut_rear[id];
            int index2 = lut_left[id];
            pano[id] = w*image_rear[index1] + (1 - w)*image_left[index2];
            break;
        }
    case 7:
        {
            int index1 = lut_rear[id];
            pano[id] = image_rear[index1];
            break;
        }
    case 8:
        {
            float w = weight[id];
            size_t index1 = lut_rear[id];
            size_t index2 = lut_right[id];
            pano[id] = w*image_rear[index1] + (1 - w)*image_right[index2];
            break;
        }
    default:
        break;
    }
}
