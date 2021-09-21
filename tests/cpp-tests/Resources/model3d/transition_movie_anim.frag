#ifdef GL_ES
precision lowp float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
uniform float _lightPlus;

void main()
{
    vec4 color = texture2D(CC_Texture0, v_texCoord);
    float plus = 1.0 + pow(1.0 - color.a, 1.2);
    color = color * plus;
    gl_FragColor = v_fragmentColor * color;
}
