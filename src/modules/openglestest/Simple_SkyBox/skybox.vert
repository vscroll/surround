static const char gVShaderStr[] = STRINGIFY(

attribute vec3 aPosition;
//attribute vec2 aTextureCoord;
//attribute vec3 aNormal;
uniform mat4 uWVPMatrix;
varying mediump vec3 vTexCoord;

void main()
{
    //transform every position vertex by the model-view-projection matrix
    vec4 position = uWVPMatrix * vec4(aPosition, 1.0);

    //Trick to place the skybox behind any other 3D model
    gl_Position = position.xyww;

    vTexCoord = aPosition;
}
);
