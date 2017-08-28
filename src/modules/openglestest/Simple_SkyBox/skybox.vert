static const char gVShaderStr[] = STRINGIFY(
attribute vec3 aPosition;
uniform mat4 uWVP;
varying vec3 vTexCoord;

void main()
{
    //vec4 WVP_Pos = uWVP * vec4(aPosition, 1.0);
    //gl_Position = WVP_Pos.xyzw;
    gl_Position = uWVP * vec4(aPosition, 1.0);
    vTexCoord = aPosition;
}
);
