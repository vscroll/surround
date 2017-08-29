static const char gFShaderStr[] = STRINGIFY(

//declare a uniform sampler2d that contains the texture data
uniform samplerCube uSkyBoxTexture;

//declare varying type which will transfer the texture coordinates from the vertex shader
varying mediump vec3 vTexCoord;

void main()
{
    //set the final color to the output of the fragment shader
    gl_FragColor = textureCube(uSkyBoxTexture, vTexCoord);
}
);
