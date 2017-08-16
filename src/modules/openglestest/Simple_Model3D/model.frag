#if (!TEST)

static const char gFShaderStr[] = STRINGIFY(
precision mediump float;
uniform sampler2D uTexture;
varying vec2 vTextureCoord;
varying  vec4 vColor;
varying vec4 vDiffuse;
void main()
{
    gl_FragColor = (vDiffuse + vec4(0.6,0.6,0.6,1))*texture2D(uTexture, vec2(vTextureCoord.s,vTextureCoord.t));
}
);

#else

static const char gFShaderStr[] = STRINGIFY(
precision mediump float;
void main()
{
    gl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );
}
);

#endif

