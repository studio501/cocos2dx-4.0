//
//  SpriteMelt.cpp
//  IF
//

#include "SpriteMelt.h"
#include "CCMathUtils.h"
using namespace cocos2d;


SpriteMelt::SpriteMelt()
:m_duration(1.0f)
,m_timePass(0.0f)
,m_threadDt(0.1f)
,m_thread(0.0f)
,m_range(0.1f)
,m_rBeginColor(Vec3(255, 0, 0))
,m_rEndColor(Vec3(255, 255, 0))
{
    
}

SpriteMelt::~SpriteMelt()
{
    
}

SpriteMelt* SpriteMelt::create(const std::string& filename, float duration, float range, Vec3 beginColor, Vec3 endColor)
{
    SpriteMelt* pRet = new (std::nothrow) SpriteMelt();
    if (nullptr == pRet)
        return nullptr;
    pRet->m_duration = duration;
    pRet->m_range = range;
    pRet->m_rBeginColor = beginColor;
    pRet->m_rEndColor = endColor;
    
    if (pRet->initWithFile(filename))
        pRet->autorelease();
    else
        CC_SAFE_DELETE(pRet);
    return pRet;
}


SpriteMelt* SpriteMelt::createWithTexture(Texture2D *texture, float duration, float range, Vec3 beginColor, Vec3 endColor)
{
    SpriteMelt* pRet = new (std::nothrow) SpriteMelt();
    if (nullptr == pRet)
        return nullptr;
    pRet->m_duration = duration;
    pRet->m_range = range;
    pRet->m_rBeginColor = beginColor;
    pRet->m_rEndColor = endColor;
    
    if (pRet->initWithTexture(texture))
        pRet->autorelease();
    else
        CC_SAFE_DELETE(pRet);
    return pRet;
}

SpriteMelt* SpriteMelt::createWithSpriteFrame(SpriteFrame *spriteFrame, float duration, float range, Vec3 beginColor, Vec3 endColor)
{
    SpriteMelt* pRet = new (std::nothrow) SpriteMelt();
    if (nullptr == pRet)
        return nullptr;
    pRet->m_duration = duration;
    pRet->m_range = range;
    pRet->m_rBeginColor = beginColor;
    pRet->m_rEndColor = endColor;
    
    if (pRet->initWithSpriteFrame(spriteFrame))
        pRet->autorelease();
    else
        CC_SAFE_DELETE(pRet);
    return pRet;
}


bool SpriteMelt::initWithFile(const std::string& filename)
{
    if (Sprite::initWithFile(filename))
    {
        initMeltProgram();
        return true;
    }
    return false;
}

bool SpriteMelt::initWithTexture(cocos2d::Texture2D* texture)
{
    if (Sprite::initWithTexture(texture))
    {
        initMeltProgram();
        return true;
    }
    return false;
}

bool SpriteMelt::initWithSpriteFrame(SpriteFrame *spriteFrame)
{
    if (Sprite::initWithSpriteFrame(spriteFrame))
    {
        initMeltProgram();
        return true;
    }
    return false;
}

void SpriteMelt::initMeltProgram()
{
//    if(true)
//        return;
    Texture2D* noiseTexture = Director::getInstance()->getTextureCache()->addImage("burn_noise.png");

    auto texture = getTexture();
    auto size = texture->getContentSize();
    float width = size.width;
    float height = size.height;
    float maxPix = width > height ? width : height;
    
    int i_slot = 1;

    this->updateShaders(positionTextureColor_vert, MeltFragPVR);

    
    auto glProgramState = this->getProgramState();
        
    float offsetU = CCMathUtils::getRandom(0.0f, 1.0f);
    float offsetV = CCMathUtils::getRandom(0.0f, 1.0f);
    
    SET_UNIFORM(glProgramState, "uFactor", width / maxPix);
    SET_UNIFORM(glProgramState, "vFactor", height / maxPix);
    SET_UNIFORM(glProgramState, "_burnRange", m_range);
    SET_UNIFORM(glProgramState, "offsetUV", Vec2(offsetU, offsetV));
    SET_UNIFORM(glProgramState, "_rBeginColor", m_rBeginColor / 255.0f);
    SET_UNIFORM(glProgramState, "_rEndColor", m_rEndColor / 255.0f);
    
    backend::SamplerDescriptor texParams(backend::SamplerFilter::LINEAR, backend::SamplerFilter::LINEAR, backend::SamplerAddressMode::REPEAT, backend::SamplerAddressMode::REPEAT);
    
    noiseTexture->setTexParameters(texParams);
    
    
    auto noise_loc = glProgramState->getUniformLocation("noiseTexture");
    glProgramState->setTexture(noise_loc, i_slot, noiseTexture->getBackendTexture());
    
    
    _thread = glProgramState->getUniformLocation("_thread");

}

void SpriteMelt::update(float dt)
{
    m_timePass = m_timePass + dt;
    float t_thread = m_timePass / m_duration;
    getProgramState()->setUniform(_thread, &t_thread,sizeof(t_thread));
    m_thread = m_thread + m_threadDt;
}

void SpriteMelt::onEnter()
{
    Sprite::onEnter();
    Director::getInstance()->getScheduler()->scheduleUpdate(this, 0, false);
}

void SpriteMelt::onExit()
{
    Sprite::onExit();
}
