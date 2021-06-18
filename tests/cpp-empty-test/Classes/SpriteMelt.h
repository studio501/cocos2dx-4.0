//
//  SpriteMelt.h
//  IF

#ifndef __IF__SpriteMelt__
#define __IF__SpriteMelt__

#include "cocos2d.h"
using namespace cocos2d;

//////////////////////////////////////////////////////////////////////////
//消融精灵
//////////////////////////////////////////////////////////////////////////
class SpriteMelt : public cocos2d::Sprite
{
public:
    SpriteMelt();
    virtual ~SpriteMelt();
    
    static SpriteMelt* create(const std::string& filename, float duration, float range, Vec3 beginColor, Vec3 endColor);
    static SpriteMelt* createWithTexture(Texture2D *texture, float duration, float range, Vec3 beginColor, Vec3 endColor);
    static SpriteMelt* createWithSpriteFrame(SpriteFrame *spriteFrame, float duration, float range, Vec3 beginColor, Vec3 endColor);
    
    virtual bool initWithFile(const std::string& filename);
    virtual bool initWithTexture(Texture2D *texture);
    virtual bool initWithSpriteFrame(SpriteFrame *spriteFrame);
    
    void initMeltProgram();
    
    virtual void onEnter();
    virtual void onExit();
    virtual void update(float dt);
private:
    float m_duration;
    float m_timePass;
    float m_threadDt;
    float m_thread;
    float m_range;
    Vec3 m_rBeginColor;
    Vec3 m_rEndColor;

    backend::UniformLocation _thread;
};



#endif /* defined(__IF__SpriteMelt__) */
