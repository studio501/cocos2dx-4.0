#ifdef GL_ES
precision lowp float;
#endif

varying vec4 v_fragmentColor;
varying vec2 v_texCoord;

uniform sampler2D _MASK;
uniform float _rockTime;
uniform float _BAND;
uniform float _DIM;

void main()
{
    vec4 mask = texture2D(_MASK, vec2(v_texCoord.x, 1.0 - v_texCoord.y));
    vec4 prevScene = texture2D(CC_Texture0, v_texCoord);
    prevScene = vec4(prevScene.rgb * _DIM, prevScene.a);

    vec4 display;
    float line = 1.0 - _rockTime;
    float diff = mask.a - line + _BAND;
    if (diff >= 0.0) {
        if (diff <= _BAND) {
            float alpha = 1.0 - diff / _BAND;
            display = vec4(prevScene.rgb * alpha, alpha);
        } else {
            display = vec4(0.0, 0.0, 0.0, 0.0);
        }
    } else {
        display = prevScene;
    }
    gl_FragColor = display;
}
