//
//  PrismaticSprite.hpp
//  HelloCpp
//
//  Created by mac on 2021/6/18.
//

#ifndef PrismaticSprite_hpp
#define PrismaticSprite_hpp

#include "2d/CCSprite.h"
USING_NS_CC;
NS_CC_BEGIN


class PrismaticSprite : public cocos2d::Sprite
{
public:
    static PrismaticSprite* createWithTexture(Texture2D *texture);
    static PrismaticSprite* createWithTexture(Texture2D *texture, const Rect& rect, bool rotated=false);
protected:
    virtual void setTextureCoords(const Rect& rect, V3F_C4B_T2F_Quad* outQuad) override;
    virtual void setVertexCoords(const Rect& rect, V3F_C4B_T2F_Quad* outQuad) override;
    
    virtual void updatePoly() override;
};


NS_CC_END


#endif /* PrismaticSprite_hpp */
