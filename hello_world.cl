// author: Andre Silva 
// email: andreluizeng@yahoo.com.br

__kernel void hello_world	(__global int *input, 
				 __global int *output,
                 int width,
                 int height)
{
	// gets the global id
	int row = get_global_id (1);
	int col = get_global_id (0);

	// copy the input to output
	output[row*width+col] = input[row*width+col];
}
