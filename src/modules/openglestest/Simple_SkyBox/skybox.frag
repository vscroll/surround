static const char gFShaderStr[] = STRINGIFY(
varying vec3 vTexCoord;
uniform samplerCube uCubemapTexture;
void main()
{
    gl_FragColor = textureCube(uCubemapTexture, vTexCoord);
}
);
