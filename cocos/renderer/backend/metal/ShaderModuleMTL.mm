/****************************************************************************
 Copyright (c) 2018-2019 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
 
#include "ShaderModuleMTL.h"
#include "DeviceMTL.h"

#include "glsl_optimizer.h"

CC_BACKEND_BEGIN


static const char* test_vert = R"(
#ifndef __METAL_VERSION__
#include <TargetConditionals.h>
#endif

#include <simd/simd.h>

#include <metal_stdlib>
//#pragma clang diagnostic ignored "-Wparentheses-equality"
using namespace metal;
struct xlatMtlShaderInput1 {
    float4 a_position [[attribute(0)]];
    float2 a_texCoord [[attribute(1)]];
    float4 a_color [[attribute(2)]];
};
struct xlatMtlShaderOutput1 {
    float4 gl_Position [[position]];
    float4 v_fragmentColor;
    float2 v_texCoord;
};
struct xlatMtlShaderUniform1 {
  float4x4 u_MVPMatrix;
};
vertex xlatMtlShaderOutput1 xlatMtlMain1 (xlatMtlShaderInput1 _mtl_i [[stage_in]], constant xlatMtlShaderUniform1& _mtl_u [[buffer(1)]])
{
  xlatMtlShaderOutput1 _mtl_o;
  _mtl_o.gl_Position = (_mtl_u.u_MVPMatrix * _mtl_i.a_position);
  _mtl_o.v_fragmentColor = _mtl_i.a_color;
  _mtl_o.v_texCoord = _mtl_i.a_texCoord;
  return _mtl_o;
}
)";

static const char* test_frag = R"(
#include <metal_stdlib>
//#pragma clang diagnostic ignored "-Wparentheses-equality"
using namespace metal;
struct xlatMtlShaderInput2 {
    float4 v_fragmentColor;
    float2 v_texCoord;
};
struct xlatMtlShaderOutput2 {
  half4 gl_FragColor;
};
struct xlatMtlShaderUniform2 {
};
fragment xlatMtlShaderOutput2 xlatMtlMain2 (xlatMtlShaderInput2 _mtl_i [[stage_in]], constant xlatMtlShaderUniform2& _mtl_u [[buffer(1)]]
  ,   texture2d<float> u_texture [[texture(0)]], sampler _mtlsmp_u_texture [[sampler(0)]])
{
  xlatMtlShaderOutput2 _mtl_o;
  half4 tmpvar_1 = 0;
    tmpvar_1 = half4(u_texture.sample(_mtlsmp_u_texture, (float2)(_mtl_i.v_texCoord)));
  // _mtl_o.gl_FragColor = (half4)(_mtl_i.v_fragmentColor);

    half4 tmpvar_2 = (half4)(_mtl_i.v_fragmentColor * (float4)(tmpvar_1));
    if(tmpvar_2.w == 0.0h)
        discard_fragment();

    _mtl_o.gl_FragColor = half4(tmpvar_2.xyz, tmpvar_2.w );
    return _mtl_o;
}
)";

ShaderModuleMTL::ShaderModuleMTL(id<MTLDevice> mtlDevice, ShaderStage stage, const std::string& source)
: ShaderModule(stage)
{
    // Convert GLSL shader to metal shader
    //TODO: don't crreate/destroy ctx every time.
    glslopt_ctx* ctx = glslopt_initialize(kGlslTargetMetal);
    glslopt_shader_type shaderType = stage == ShaderStage::VERTEX ? kGlslOptShaderVertex : kGlslOptShaderFragment;
    glslopt_shader* glslShader = glslopt_optimize(ctx, shaderType, source.c_str(), 0);
    if (!glslShader)
    {
        NSLog(@"Can not translate GLSL shader to metal shader:");
        NSLog(@"%s", source.c_str());
        return;
    }
    
    char* metalShader = nullptr;
    static int ta = 0;
    if(ta < 2){
        if (ShaderStage::VERTEX == stage)
            metalShader = (char *)test_vert;
        else
            metalShader = (char *)test_frag;
    }else{
        metalShader = (char *)glslopt_get_output(glslShader);
    }
    
    ta++;
    
    if (!metalShader)
    {
        NSLog(@"Can not get metal shader:");
        NSLog(@"%s", source.c_str());
        glslopt_cleanup(ctx);
        return;
    }
    
//    NSLog(@"%s", metalShader);
    
    parseAttibute(mtlDevice, glslShader);
    parseUniform(mtlDevice, glslShader);
    parseTexture(mtlDevice, glslShader);
    setBuiltinUniformLocation();
    setBuiltinAttributeLocation();
    
    NSString* shader = [NSString stringWithUTF8String:metalShader];
    NSError* error;
    id<MTLLibrary> library = [mtlDevice newLibraryWithSource:shader
                                                     options:nil
                                                       error:&error];
    if (!library)
    {
        NSLog(@"Can not compile metal shader: %@", error);
        NSLog(@"%s", metalShader);
        glslopt_shader_delete(glslShader);
        glslopt_cleanup(ctx);
        return;
    }
    
    if (ShaderStage::VERTEX == stage)
        _mtlFunction = [library newFunctionWithName:@"xlatMtlMain1"];
    else
        _mtlFunction = [library newFunctionWithName:@"xlatMtlMain2"];
    if (!_mtlFunction)
    {
        NSLog(@"metal shader is ---------------");
        NSLog(@"%s", metalShader);
        assert(false);
    }
    
    glslopt_shader_delete(glslShader);
    glslopt_cleanup(ctx);
    [library release];
}

ShaderModuleMTL::~ShaderModuleMTL()
{
    [_mtlFunction release];
}

void ShaderModuleMTL::parseAttibute(id<MTLDevice> mtlDevice, glslopt_shader* shader)
{
    const int attributeCount = glslopt_shader_get_input_count(shader);
    for(int i = 0; i < attributeCount; i++)
    {
        const char* parName;
        glslopt_basic_type parType;
        glslopt_precision parPrec;
        int parVecSize, parMatSize, parArrSize, location;
         glslopt_shader_get_input_desc(shader, i, &parName, &parType, &parPrec, &parVecSize, &parMatSize, &parArrSize, &location);
        
        AttributeBindInfo attributeInfo;
        attributeInfo.attributeName = parName;
        attributeInfo.location = location;
        _attributeInfo[parName] = attributeInfo;
    }
}

void ShaderModuleMTL::parseUniform(id<MTLDevice> mtlDevice, glslopt_shader* shader)
{
    const int uniformCount = glslopt_shader_get_uniform_count(shader);
    _uniformBufferSize = glslopt_shader_get_uniform_total_size(shader);
    
    for (int i = 0; i < uniformCount; ++i)
    {
        int nextLocation = -1;
        const char* parName;
        glslopt_basic_type parType;
        glslopt_precision parPrec;
        int parVecSize, parMatSize, parArrSize, location;
        if( i+1 < uniformCount)
        {
            glslopt_shader_get_uniform_desc(shader, i+1, &parName, &parType, &parPrec, &parVecSize, &parMatSize, &parArrSize, &location);
            nextLocation = location;
        }
        else
        {
            nextLocation = static_cast<int>(_uniformBufferSize);
        }
        
        glslopt_shader_get_uniform_desc(shader, i, &parName, &parType, &parPrec, &parVecSize, &parMatSize, &parArrSize, &location);
        
        parArrSize = (parArrSize > 0) ? parArrSize : 1;
        UniformInfo uniform;
        uniform.count = parArrSize;
        uniform.location = location;
        uniform.isArray = parArrSize;
        uniform.size = nextLocation - location;
        uniform.bufferOffset = location;
        uniform.needConvert = (parVecSize == 3) ? true : false;
        uniform.type = static_cast<unsigned int>(parType);
        uniform.isMatrix = (parMatSize > 1) ? true : false;
        _uniformInfos[parName] = uniform;
        _activeUniformInfos[location] = uniform;
        _maxLocation = _maxLocation < location ? (location + 1) : _maxLocation;
    }
}

int ShaderModuleMTL::getUniformLocation(Uniform name) const
{
    return _uniformLocation[name];
}

int ShaderModuleMTL::getUniformLocation(const std::string& name) const
{
    const auto& iter = _uniformInfos.find(name);
    if(iter != _uniformInfos.end())
    {
        return _uniformInfos.at(name).location;
    }
    else
        return -1;
}

void ShaderModuleMTL::setBuiltinUniformLocation()
{
    std::fill(_uniformLocation, _uniformLocation + UNIFORM_MAX, -1);
    ///u_mvpMatrix
    auto iter = _uniformInfos.find(UNIFORM_NAME_MVP_MATRIX);
    if(iter != _uniformInfos.end())
    {
        _uniformLocation[Uniform::MVP_MATRIX] = iter->second.location;
    }
    
    ///u_textColor
    iter = _uniformInfos.find(UNIFORM_NAME_TEXT_COLOR);
    if(iter != _uniformInfos.end())
    {
        _uniformLocation[Uniform::TEXT_COLOR] = iter->second.location;
    }
    
    ///u_effectColor
    iter = _uniformInfos.find(UNIFORM_NAME_EFFECT_COLOR);
    if(iter != _uniformInfos.end())
    {
        _uniformLocation[Uniform::EFFECT_COLOR] = iter->second.location;
    }
    
    ///u_effectType
    iter = _uniformInfos.find(UNIFORM_NAME_EFFECT_TYPE);
    if(iter != _uniformInfos.end())
    {
        _uniformLocation[Uniform::EFFECT_TYPE] = iter->second.location;
    }
    
    ///u_texture
    iter = _uniformInfos.find(UNIFORM_NAME_TEXTURE);
    if(iter != _uniformInfos.end())
    {
        _uniformLocation[Uniform::TEXTURE] = iter->second.location;
    }
    
    ///u_texture1
    iter = _uniformInfos.find(UNIFORM_NAME_TEXTURE1);
    if(iter != _uniformInfos.end())
    {
        _uniformLocation[Uniform::TEXTURE1] = iter->second.location;
    }
}

int ShaderModuleMTL::getAttributeLocation(Attribute name) const
{
    return _attributeLocation[name];
}

int ShaderModuleMTL::getAttributeLocation(std::string name)
{
    auto iter = _attributeInfo.find(name);
    if(iter != _attributeInfo.end())
        return _attributeInfo[name].location;
    else
        return -1;
}

void ShaderModuleMTL::setBuiltinAttributeLocation()
{
    std::fill(_attributeLocation, _attributeLocation + ATTRIBUTE_MAX, -1);
    ///a_position
    auto iter = _attributeInfo.find(ATTRIBUTE_NAME_POSITION);
    if(iter != _attributeInfo.end())
    {
        _attributeLocation[Attribute::POSITION] = iter->second.location;
    }
    
    ///a_color
    iter = _attributeInfo.find(ATTRIBUTE_NAME_COLOR);
    if(iter != _attributeInfo.end())
    {
        _attributeLocation[Attribute::COLOR] = iter->second.location;
    }
    
    ///a_texCoord
    iter = _attributeInfo.find(ATTRIBUTE_NAME_TEXCOORD);
    if(iter != _attributeInfo.end())
    {
        _attributeLocation[Attribute::TEXCOORD] = iter->second.location;
    }
}

void ShaderModuleMTL::parseTexture(id<MTLDevice> mtlDevice, glslopt_shader* shader)
{
    const int textureCount = glslopt_shader_get_texture_count(shader);
    for (int i = 0; i < textureCount; ++i)
    {
        const char* parName;
        glslopt_basic_type parType;
        glslopt_precision parPrec;
        int parVecSize, parMatSize, parArrSize, location;
        glslopt_shader_get_texture_desc(shader, i, &parName, &parType, &parPrec, &parVecSize, &parMatSize, &parArrSize, &location);
        
        UniformInfo uniform;
        uniform.count = parArrSize;
        uniform.location = location;
        uniform.isArray = parArrSize > 0;
        _uniformInfos[parName] = uniform;
    }
}

CC_BACKEND_END
