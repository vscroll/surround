// author: Andre Silva 
// email: andreluizeng@yahoo.com.br

__kernel void hello_world	(__global uchar *input, 
				 __global uchar *output)
{
	// gets the global id
	int id = get_global_id (0);

	// copy the input to output
	output[id] = input[id];
}
