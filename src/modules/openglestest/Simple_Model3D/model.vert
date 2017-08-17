
#if (!TEST)

static const char gVShaderStr[] = STRINGIFY(
attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec2 aTextureCoord;
uniform mat4 uMVPMatrix;
uniform mat4 uMMatrix;
uniform vec3 uLightLocation;
varying vec2 vTextureCoord;
varying vec4 vDiffuse;

void main()
{
    vec3 normalVectorOrigin = aNormal;
    vec3 normalVector = normalize((uMMatrix*vec4(normalVectorOrigin,1)).xyz);
    vec3 vectorLight = normalize(uLightLocation - (uMMatrix * vec4(aPosition,1)).xyz);
    float factor = max(0.0, dot(normalVector, vectorLight));
    vDiffuse = factor*vec4(1,1,1,1.0);
    gl_Position = uMVPMatrix * vec4(aPosition,1);
    vTextureCoord = aTextureCoord;
}
);

#else

static const char gVShaderStr[] = STRINGIFY(
uniform mat4 uMVPMatrix;
attribute vec4 aPosition;
void main()
{
    gl_Position = uMVPMatrix * aPosition;
}
);

#endif
