__kernel void stitch_2d	(__global int* image_front,
			__global int* image_rear,
			__global int* image_left,
			__global int* image_right,
			__global int *mask,
			__global int *map_x,
			__global int *map_y,
			int width,
			int height,
			__global int *pano_2d)
{
	// gets the global id
	int row = get_global_id(1);
	int col = get_global_id(0);

	// copy the input to output
	int flag = mask[row*width+col];
	int x = map_x[row*width+col];
	int y = map_y[row*width+col];
	switch (flag)
	{
	case 50:
		{
			pano_2d[row*width+col] = image_front[y*width+x];
			break;
		}
	case 100:
		{
			pano_2d[row*width+col] = image_right[y*width+x];
			break;
		}
	case 150:
		{
			pano_2d[row*width+col] = image_left[y*width+x];
			break;
		}
	case 200:
		{
			pano_2d[row*width+col] = image_rear[y*width+x];
			break;
		}
	default:
		break;		
    }
}
