/*
 * cocos2d for iPhone: http://www.cocos2d-iphone.org
 *
 * Copyright (c) 2011 Ricardo Quesada
 * Copyright (c) 2012 Zynga Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

const char* MeltFragPVR = R"(
                                       
#ifdef GL_ES
precision lowp float;
#endif

varying vec2 v_texCoord;
varying vec4 v_fragmentColor;
                                    
uniform float uFactor;
uniform float vFactor;
uniform float _thread;
uniform float _burnRange;
uniform vec2 offsetUV;
uniform vec3 _rBeginColor;
uniform vec3 _rEndColor;

uniform sampler2D u_texture;
uniform sampler2D noiseTexture;
                                    
void main()
{
    vec2 mapTex = vec2(v_texCoord.x * uFactor, v_texCoord.y * vFactor);
    vec4 noise = texture2D(noiseTexture, mapTex + offsetUV);
    
    if(noise.r < _thread)
    {
        discard;
    }
    else if(noise.r - _thread < _burnRange)
    {
        float p = smoothstep(0.0, _burnRange, noise.r - _thread);
        gl_FragColor = mix(vec4(_rBeginColor, 1.0), vec4(_rEndColor, 1.0), p);
    }
    else
    {
        vec4 diffuse = texture2D(u_texture, v_texCoord);
        gl_FragColor = v_fragmentColor * diffuse;
    }
}
)";

