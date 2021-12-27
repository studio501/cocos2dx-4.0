/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
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

#include "HelloWorldScene.h"
#include "AppMacros.h"
#include "SpriteMelt.h"

USING_NS_CC;



RenderTextureFor3D* RenderTextureFor3D::create(int nWidth, int nHeight)
{
    RenderTextureFor3D *ret = new (std::nothrow) RenderTextureFor3D();
    if (ret && ret->InitWithWidthAndHeight(nWidth,nHeight))
    {
        ret->autorelease();
        return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;

}

bool RenderTextureFor3D::InitWithWidthAndHeight(int nWidth, int nHeight)
{
    if(!RenderTexture::initWithWidthAndHeight(nWidth, nHeight,backend::PixelFormat::RGBA8888,backend::PixelFormat::D24S8))
        return false;

    bool test_3d = true;
    _makeClearOn3dQueue = test_3d;
    _beginCommand.set3D(test_3d);
    _beginCommand.setTransparent(false);

    _endCommand.set3D(test_3d);
    _endCommand.setTransparent(false);

    _groupCommand.set3D(test_3d);
    _groupCommand.setTransparent(false);


    return true;
}

Scene* HelloWorld::scene()
{
     return HelloWorld::create();
}


////////////////////////////////////////////////////////
// http://vitiy.info/Code/stackblur.cpp


// The Stack Blur Algorithm was invented by Mario Klingemann,
// mario@quasimondo.com and described here:
// http://incubator.quasimondo.com/processing/fast_blur_deluxe.php

// This is C++ RGBA (32 bit color) multi-threaded version
// by Victor Laskin (victor.laskin@gmail.com)
// More details: http://vitiy.info/stackblur-algorithm-multi-threaded-blur-for-cpp

// This code is using MVThread class from my cross-platform framework
// You can exchange it with any thread implementation you like


// -------------------------------------- stackblur ----------------------------------------->

static unsigned short const stackblur_mul[255] =
        {
                512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
                454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
                482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
                437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
                497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
                320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
                446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
                329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
                505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
                399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
                324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
                268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
                451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
                385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
                332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
                289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
        };

static unsigned char const stackblur_shr[255] =
        {
                9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
                17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
                19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
                20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
                21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
                21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
                22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
                22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
                23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
                23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
                23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
                23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
                24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
                24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
                24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
                24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
        };

/// Stackblur algorithm body
void stackblurJob(unsigned char* src,				///< input image data
                  unsigned int w,					///< image width
                  unsigned int h,					///< image height
                  unsigned int radius,				///< blur intensity (should be in 2..254 range)
                  int cores,						///< total number of working threads
                  int core,							///< current thread number
                  int step,							///< step of processing (1,2)
                  unsigned char* stack				///< stack buffer
)
{
    unsigned int x, y, xp, yp, i;
    unsigned int sp;
    unsigned int stack_start;
    unsigned char* stack_ptr;

    unsigned char* src_ptr;
    unsigned char* dst_ptr;

    unsigned long sum_r;
    unsigned long sum_g;
    unsigned long sum_b;
//    unsigned long sum_a;
    unsigned long sum_in_r;
    unsigned long sum_in_g;
    unsigned long sum_in_b;
//    unsigned long sum_in_a;
    unsigned long sum_out_r;
    unsigned long sum_out_g;
    unsigned long sum_out_b;
//    unsigned long sum_out_a;

    unsigned int wm = w - 1;
    unsigned int hm = h - 1;
    unsigned int w4 = w * 4;
    unsigned int div = (radius * 2) + 1;
    unsigned int mul_sum = stackblur_mul[radius];
    unsigned char shr_sum = stackblur_shr[radius];


    if (step == 1)
    {
        int minY = core * h / cores;
        int maxY = (core + 1) * h / cores;

        for(y = minY; y < maxY; y++)
        {
            sum_r = sum_g = sum_b =
            sum_in_r = sum_in_g = sum_in_b =
            sum_out_r = sum_out_g = sum_out_b = 0;

            src_ptr = src + w4 * y; // start of line (0,y)

            for(i = 0; i <= radius; i++)
            {
                stack_ptr    = &stack[ 4 * i ];
                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                stack_ptr[3] = src_ptr[3];
                sum_r += src_ptr[0] * (i + 1);
                sum_g += src_ptr[1] * (i + 1);
                sum_b += src_ptr[2] * (i + 1);
//                sum_a += src_ptr[3] * (i + 1);
                sum_out_r += src_ptr[0];
                sum_out_g += src_ptr[1];
                sum_out_b += src_ptr[2];
//                sum_out_a += src_ptr[3];
            }


            for(i = 1; i <= radius; i++)
            {
                if (i <= wm) src_ptr += 4;
                stack_ptr = &stack[ 4 * (i + radius) ];
                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                stack_ptr[3] = src_ptr[3];
                sum_r += src_ptr[0] * (radius + 1 - i);
                sum_g += src_ptr[1] * (radius + 1 - i);
                sum_b += src_ptr[2] * (radius + 1 - i);
//                sum_a += src_ptr[3] * (radius + 1 - i);
                sum_in_r += src_ptr[0];
                sum_in_g += src_ptr[1];
                sum_in_b += src_ptr[2];
//                sum_in_a += src_ptr[3];
            }


            sp = radius;
            xp = radius;
            if (xp > wm) xp = wm;
            src_ptr = src + 4 * (xp + y * w); //   img.pix_ptr(xp, y);
            dst_ptr = src + y * w4; // img.pix_ptr(0, y);
            for(x = 0; x < w; x++)
            {
                dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
                dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
                dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
                dst_ptr[3] = 0;
                dst_ptr += 4;

                sum_r -= sum_out_r;
                sum_g -= sum_out_g;
                sum_b -= sum_out_b;
//                sum_a -= sum_out_a;

                stack_start = sp + div - radius;
                if (stack_start >= div) stack_start -= div;
                stack_ptr = &stack[4 * stack_start];

                sum_out_r -= stack_ptr[0];
                sum_out_g -= stack_ptr[1];
                sum_out_b -= stack_ptr[2];
//                sum_out_a -= stack_ptr[3];

                if(xp < wm)
                {
                    src_ptr += 4;
                    ++xp;
                }

                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                stack_ptr[3] = src_ptr[3];

                sum_in_r += src_ptr[0];
                sum_in_g += src_ptr[1];
                sum_in_b += src_ptr[2];
//                sum_in_a += src_ptr[3];
                sum_r    += sum_in_r;
                sum_g    += sum_in_g;
                sum_b    += sum_in_b;
//                sum_a    += sum_in_a;

                ++sp;
                if (sp >= div) sp = 0;
                stack_ptr = &stack[sp*4];

                sum_out_r += stack_ptr[0];
                sum_out_g += stack_ptr[1];
                sum_out_b += stack_ptr[2];
//                sum_out_a += stack_ptr[3];
                sum_in_r  -= stack_ptr[0];
                sum_in_g  -= stack_ptr[1];
                sum_in_b  -= stack_ptr[2];
//                sum_in_a  -= stack_ptr[3];


            }

        }
    }

    // step 2
    if (step == 2)
    {
        int minX = core * w / cores;
        int maxX = (core + 1) * w / cores;

        for(x = minX; x < maxX; x++)
        {
            sum_r =	sum_g =	sum_b =
            sum_in_r = sum_in_g = sum_in_b =
            sum_out_r = sum_out_g = sum_out_b = 0;

            src_ptr = src + 4 * x; // x,0
            for(i = 0; i <= radius; i++)
            {
                stack_ptr    = &stack[i * 4];
                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                stack_ptr[3] = src_ptr[3];
                sum_r           += src_ptr[0] * (i + 1);
                sum_g           += src_ptr[1] * (i + 1);
                sum_b           += src_ptr[2] * (i + 1);
//                sum_a           += src_ptr[3] * (i + 1);
                sum_out_r       += src_ptr[0];
                sum_out_g       += src_ptr[1];
                sum_out_b       += src_ptr[2];
//                sum_out_a       += src_ptr[3];
            }
            for(i = 1; i <= radius; i++)
            {
                if(i <= hm) src_ptr += w4; // +stride

                stack_ptr = &stack[4 * (i + radius)];
                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                stack_ptr[3] = src_ptr[3];
                sum_r += src_ptr[0] * (radius + 1 - i);
                sum_g += src_ptr[1] * (radius + 1 - i);
                sum_b += src_ptr[2] * (radius + 1 - i);
//                sum_a += src_ptr[3] * (radius + 1 - i);
                sum_in_r += src_ptr[0];
                sum_in_g += src_ptr[1];
                sum_in_b += src_ptr[2];
//                sum_in_a += src_ptr[3];
            }

            sp = radius;
            yp = radius;
            if (yp > hm) yp = hm;
            src_ptr = src + 4 * (x + yp * w); // img.pix_ptr(x, yp);
            dst_ptr = src + 4 * x; 			  // img.pix_ptr(x, 0);
            for(y = 0; y < h; y++)
            {
                dst_ptr[0] = (sum_r * mul_sum) >> shr_sum;
                dst_ptr[1] = (sum_g * mul_sum) >> shr_sum;
                dst_ptr[2] = (sum_b * mul_sum) >> shr_sum;
//                dst_ptr[3] = (sum_a * mul_sum) >> shr_sum;
                dst_ptr += w4;

                sum_r -= sum_out_r;
                sum_g -= sum_out_g;
                sum_b -= sum_out_b;
//                sum_a -= sum_out_a;

                stack_start = sp + div - radius;
                if(stack_start >= div) stack_start -= div;
                stack_ptr = &stack[4 * stack_start];

                sum_out_r -= stack_ptr[0];
                sum_out_g -= stack_ptr[1];
                sum_out_b -= stack_ptr[2];
//                sum_out_a -= stack_ptr[3];

                if(yp < hm)
                {
                    src_ptr += w4; // stride
                    ++yp;
                }

                stack_ptr[0] = src_ptr[0];
                stack_ptr[1] = src_ptr[1];
                stack_ptr[2] = src_ptr[2];
                stack_ptr[3] = src_ptr[3];

                sum_in_r += src_ptr[0];
                sum_in_g += src_ptr[1];
                sum_in_b += src_ptr[2];
//                sum_in_a += src_ptr[3];
                sum_r    += sum_in_r;
                sum_g    += sum_in_g;
                sum_b    += sum_in_b;
//                sum_a    += sum_in_a;

                ++sp;
                if (sp >= div) sp = 0;
                stack_ptr = &stack[sp*4];

                sum_out_r += stack_ptr[0];
                sum_out_g += stack_ptr[1];
                sum_out_b += stack_ptr[2];
//                sum_out_a += stack_ptr[3];
                sum_in_r  -= stack_ptr[0];
                sum_in_g  -= stack_ptr[1];
                sum_in_b  -= stack_ptr[2];
//                sum_in_a  -= stack_ptr[3];
            }
        }
    }

}


/// Stackblur algorithm by Mario Klingemann
/// Details here:
/// http://www.quasimondo.com/StackBlurForCanvas/StackBlurDemo.html
/// C++ implemenation base from:
/// https://gist.github.com/benjamin9999/3809142
/// http://www.antigrain.com/__code/include/agg_blur.h.html
/// This version works only with RGBA color
void 			   stackblur(unsigned char* src,				///< input image data
                             unsigned int w,					///< image width
                             unsigned int h,					///< image height
                             unsigned int radius,				///< blur intensity (should be in 2..254 range)
                             int cores						///< number of threads (1 - normal single thread)
)
{
    if (radius > 254) return;
    if (radius < 2) return;

    unsigned int div = (radius * 2) + 1;
    unsigned char* stack = new unsigned char [div * 4 * cores];
    if (stack == nullptr) {
        return;
    }

    if (cores == 1)
    {
        // no multithreading
        stackblurJob(src, w, h, radius, 1, 0, 1, stack);
        stackblurJob(src, w, h, radius, 1, 0, 2, stack);
    }
    else
    {

    }

//    unsigned int w4 = w * 4;
//    for (int j=0;j<h;++j)
//    {
//        for (int i=0;i<w;++i)
//        {
//            unsigned char* src_ptr = src + i * j * 4;
//            src_ptr[3] = 0xff;
//        }
//    }

    delete[] stack;
}

// on "init" you need to initialize your instance
bool HelloWorld::init() {
    //////////////////////////////
    // 1. super init first
    if (!Scene::init()) {
        return false;
    }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    auto winSize1 = visibleSize;
    
    // Director::getInstance()->setDisplayStats(true);
    
    {
        Sprite* pSprUpCloud = Sprite::create("CloseNormal.png");
        pSprUpCloud->setPosition(100, 100);
        addChild(pSprUpCloud);
        
        pSprUpCloud->runAction(Sequence::create(DelayTime::create(5), CallFunc::create([=](){
            pSprUpCloud->setPosition(-1000, 100);
        }), NULL));
        return true;
    }
    
    {
        for(int i=0;i<2;++i){
            auto label = Label::createWithTTF(std::string("ABAAAAAAAAAAAAAAAAAAAAAA").append(std::to_string(i)) , "fonts/Marker Felt.ttf", 24);
            label->setPosition(Vec2(origin.x + visibleSize.width/2,
                                    origin.y + visibleSize.height - label->getContentSize().height - i * 20));
            
            label->setColor(Color3B(i*20,(10 - i)*20,i*15));
            addChild(label,1);
        }
        
        auto label = Label::createWithSystemFont("The green scale9sprite is in the back.", "Arial", 15);
        addChild(label, 2);
        return true;
    }
    
    {
        
        auto m_pLayerColor1 = LayerColor::create(Color4B(255, 255, 255, 255), winSize1.width, winSize1.height);
        addChild(m_pLayerColor1);
        
        const int repeate_count = 1;
        const int draw_cnt = 1;
        for(int i=0;i<draw_cnt;i++){
            for(int j = 0; j< repeate_count; j++){
                Sprite* pSprUpCloud = Sprite::create("CloseNormal.png");
                addChild(pSprUpCloud);
                pSprUpCloud->setPosition(winSize1.width/2 - 200, winSize1.height/2);
            }
            
            for(int j = 0; j< repeate_count; j++){
                Sprite* pSprUpCloud2 = Sprite::create("boss.png");
                addChild(pSprUpCloud2);
                pSprUpCloud2->setPosition(winSize1.width/2 + 200, winSize1.height/2);
            }
        }
        
        return true;
    }
    
    {
        Director::getInstance()->setDisplayStats(true);
        m_pRT = RenderTextureFor3D::create(winSize1.width, winSize1.height);
        m_pRT->setPosition(winSize1.width/2, winSize1.height/2);
        m_pRT->setKeepMatrix(true);
        m_pRT->retain();
//        this->addChild(m_pRT);
        
        m_pModel = cocos2d::Sprite3D::create("neichenglong.c3b");
        m_pModel->setScale(22);

        // auto f = FileUtils::getInstance()->isFileExist("neichenglong_alpha.pkm");
        // CCLOGERROR("is file exist %d",f);

        // auto tex = Director::getInstance()->getTextureCache()->addImage("neichenglong.png");

        m_pModel->setTexture("neichenglong.png");
        
        
        m_pModel->setPosition(winSize1.width/2,winSize1.height/2);
        //m_pModel->setFrontFace(backend::Winding::COUNTER_CLOCK_WISE);
//        m_pModel->retain();
        addChild(m_pModel);
        m_pModel->setVisible(false);
        
       // t_pModel->setForce2DQueue(true);

        Sprite* pSprite=Sprite::createWithTexture(m_pRT->getSprite()->getTexture());
        this->addChild(pSprite);
        pSprite->setPosition(winSize1.width/2, winSize1.height/2);

        pSprite->setFlippedY(true);
        
        
        m_pModel->runAction(RepeatForever::create(RotateBy::create(1.0f,Vec3(0,-32,0))));
    
//        m_pRT->beginWithClear(1, 0, 0, 1.0f,1,0);
//
//        m_pModel->cocos2d::Node::visit(Director::getInstance()->getRenderer(),Mat4::IDENTITY,0);
//       // m_pModel->visit(Director::getInstance()->getRenderer(), Mat4::IDENTITY, 0);
//
//        m_pRT->end();



         scheduleUpdate();
        return true;
    }

    
    {

        Director::getInstance()->setDisplayStats(true);

        auto f = FileUtils::getInstance()->isFileExist("neichenglong.c3b");
        CCLOGERROR("is file exist %d",f);


        auto t_pModel1 = cocos2d::Sprite3D::create("boss.c3b");


        t_pModel1->setTexture("boss.png");
        t_pModel1->setScale(10);
        t_pModel1->runAction(RepeatForever::create(RotateBy::create(1.0f,Vec3(0,32,0))));
        this->addChild(t_pModel1);

        return true;
    }

    {
        auto rt = RenderTexture::create(winSize1.width/2, winSize1.height/2, backend::PixelFormat::RGB565);
//        auto rt = RenderTexture::create(winSize1.width/2, winSize1.height/2, backend::PixelFormat::RGBA8888);
        rt->setPosition(winSize1.width/2, winSize1.height/2);
        rt->setKeepMatrix(true);
        //this->addChild(rt);

        Sprite* pSprUpCloud = Sprite::create("CloseNormal.png");
        pSprUpCloud->setPosition(400,400);
        pSprUpCloud->setScale(4);
        rt->beginWithClear(1.0, 1.0, 1.0, 1.0);
        pSprUpCloud->visit();
        rt->end();

        auto sprite = rt->getSprite();
        auto tx = sprite->getTexture();

        auto meltSp = SpriteMelt::createWithTexture(tx, 1.0f, 0.1f, Vec3(58, 94, 218), Vec3(68, 73, 81));
        meltSp->setPosition(winSize1.width / 2, winSize1.height / 2);
//        meltSp->setFlipY(true);
        addChild(meltSp);

        return true;
    }

//    {
//
//        auto m_pLayerColor1 = LayerColor::create(Color4B(255, 255, 255, 255), winSize1.width, winSize1.height);
//        addChild(m_pLayerColor1);
//
//        auto m_pLayerColor = LayerColor::create(Color4B(0, 0, 0, 100), winSize1.width/2, winSize1.height);
//        addChild(m_pLayerColor);
//        auto func = m_pLayerColor->getBlendFunc();
//        m_pLayerColor->setBlendFunc(BlendFunc::ALPHA_PREMULTIPLIED);
//
//    }

//    {
//        auto rt = RenderTexture::create(winSize1.width/2, winSize1.height/2, backend::PixelFormat::RGBA8888, backend::PixelFormat::D24S8);
//        rt->setPosition(winSize1.width/2, winSize1.height/2);
//        rt->setKeepMatrix(true);
//        this->addChild(rt);
//
//        Sprite* pSprUpCloud = Sprite::create("CloseNormal.png");
//        pSprUpCloud->setPosition(400,400);
//        pSprUpCloud->setScale(4);
//        rt->begin();
//        pSprUpCloud->visit();
//        rt->end();
//    }

//    auto tp = Sprite::create("CloseNormal.png");
//
//    tp->setScale(2);
//
//    auto stencil = DrawNode::create();
//    Vec2 rectangle[4];
//    rectangle[0] = Vec2(0, 0);
//    rectangle[1] = Vec2(100, 0);
//    rectangle[2] = Vec2(100, 100);
//    rectangle[3] = Vec2(0, 100);
//
//    Color4F white(1, 1, 1, 1);
//    stencil->drawPolygon(rectangle, 4, white, 1, white);
//
//
//
//    auto clipNode = ClippingNode::create();
//    clipNode->setStencil(stencil);
//
//    clipNode->addChild(tp);
//
//    addChild(clipNode);
//
//    clipNode->setPosition(100, 100);

//    {
//
////        auto clipNode = ClippingNode::create(100, 100);
////        clipNode->setPosition(200,200);
//
//        auto sprite = Sprite::create("boss.png");
//        sprite->setScale(30);
//        addChild(sprite);
//
////        clipNode->addChild(sprite);
////        clipNode->setPosition(100,100);
////
////        addChild(clipNode);
//
//
//        return true;
//    }

    {
        auto clipNode = ClippingNode::create(100, 100);

        auto sprite = Sprite::create("CloseNormal.png");

        clipNode->setPosition(100,100);
        clipNode->addChild(sprite);

        addChild(clipNode);


        runAction(Sequence::create(DelayTime::create(5),CallFunc::create([clipNode](){
            clipNode->removeFromParentAndCleanup(true);
        }),NULL));
        return true;
    }
    {
        auto fileIns = FileUtils::getInstance();
        auto l1 = fileIns->getFileSize("bang.png");
        auto f = fileIns->isDirectoryExist("fonts");
        auto f1 = fileIns->isFileExist("bang.png");
        return true;
    }

    {
        auto m_pLayerColor1 = LayerColor::create(Color4B(255, 0, 0, 255), winSize1.width, winSize1.height);
        addChild(m_pLayerColor1);
        return true;
    }

    {
        auto tp3 = Sprite::create("boss.png");
        tp3->setScale(3);
        tp3->setPosition(800,1800);
        addChild(tp3);

        {
            auto tp3 = Sprite::create("boss.png");
            tp3->setScale(3);
            tp3->setPosition(1800,1800);
            addChild(tp3);
        }

        auto tp2 = Sprite::create("CloseNormal.png");
        tp2->setPosition(200,200);
        addChild(tp2);

//        utils::captureNode(this,[this](Image*img){
//
//            stackblur(img->getData(), img->getWidth(), img->getHeight(), 3, 1);
//
//            auto tx = new Texture2D;
//            tx->initWithImage(img, Texture2D::getDefaultAlphaPixelFormat());
//            img->release();
//
//            auto sp2 = Sprite::createWithTexture(tx);
//            sp2->setPosition(100,3000);
//            this->addChild(sp2);
//        });
    }


//    auto clipNode = ClippingNode::create(100, 100);
//
//    auto sprite = Sprite::create("CloseNormal.png");
//    addChild(sprite);
//    clipNode->setPosition(100,100);
//
//    addChild(clipNode);

//    auto tlayer = LayerColor::create(Color4B(44, 44, 44, 255), winSize1.width, winSize1.height);
//    addChild(tlayer, -1);
//
//    auto pRect = Rect(629, 429, 407, 268);
//    bool isRotate = true;
//
//    std::vector<std::string> resArray = {
//            "_alpha_sk_halo_iceworld_face_front.pkm",
//            "_alpha_sk_halo_iceworld_face_front_nopma.pkm",
//            "_alpha_sk_halo_iceworld_face_front.png",
//            "_alpha_sk_halo_iceworld_face_front_nopma.png",
//            "_alpha_sk_halo_iceworld_face_front.pvr",
//
//    };
//
//    float yArr[] = {1.0f, 4 / 5.0f, 3 / 5.0f, 2 / 5.0f, 1 / 5.0f};
//
//    for (auto i = 0; i < resArray.size(); i++) {
//        auto pTex = Director::getInstance()->getTextureCache()->addImage(
//                resArray[i]);
//        auto sp = Sprite::createWithTexture(pTex,  pRect, isRotate);
//        sp->setPosition(winSize1.width / 2, winSize1.height * yArr[i] - 400);
//        sp->setScale(3);
//        addChild(sp);
//        sp->setFlippedX(i < resArray.size() - 1);
//        sp->setFlippedY(i < resArray.size() - 1);
//    }

//    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("Imperial_35.plist");
//    float width = 13619.2f;//1024*3.5f*3.8f;
//    float texHeight = 256;
//    float moveUp = 200;
//
//    cocos2d::backend::SamplerDescriptor texParams(backend::SamplerFilter::LINEAR, backend::SamplerFilter::LINEAR, backend::SamplerAddressMode::REPEAT, backend::SamplerAddressMode::REPEAT);
////    cocos2d::backend::SamplerDescriptor texParams(backend::SamplerFilter::LINEAR, backend::SamplerFilter::LINEAR, backend::SamplerAddressMode::REPEAT, backend::SamplerAddressMode::REPEAT);
//
//
//
//    Sprite* pSprUpCloud = Sprite::createWithSpriteFrameName("up_cloud.png");
//    pSprUpCloud->setPosition(winSize1.width/2,winSize1.height/2);
//    addChild(pSprUpCloud);
//    pSprUpCloud->setScale(1.5);
//
//    Texture2D* pTex = pSprUpCloud->getTexture();
//
//    pTex->setTexParameters(texParams);
//
//
//    pSprUpCloud->setTextureRect(Rect(0,-moveUp,1024 ,texHeight + moveUp ));
//
//    Sprite* pSprCloudCopy = Sprite::createWithTexture(pSprUpCloud->getTexture(), pSprUpCloud->getTextureRect());
////    pSprCloudCopy->setScaleY(3);
//    pSprCloudCopy->setOpacity(128);
//    pSprCloudCopy->setScale(4);
//
//
//    float ty = winSize1.height/2;
//    pSprCloudCopy->setPosition(winSize1.width/2,ty);
//
////    pSprCloudCopy->runAction(MoveTo::create(10,Vec2(-1000,ty)));
//
//    addChild(pSprCloudCopy);

//    auto sprite = Sprite::create("CloseNormal.png");
//    sprite->setPosition(Vec2(winSize1.width/2, winSize1.height/2));
//    sprite->retain();
//    tsp = sprite;
//
//    m_pModel = cocos2d::Sprite3D::create("boss.c3b");
//    m_pModel->setScale(10);
//    m_pModel->setTexture("boss.png");
//    m_pModel->setPosition(winSize1.width/2,winSize1.height/2);
//    m_pModel->retain();
//    m_pModel->setForce2DQueue(false);
//
//    m_pRT = RenderTextureFor3D::create(winSize1.width/2, winSize1.height/2);
//    m_pRT->setPosition(winSize1.width/2, winSize1.height/2);
//    m_pRT->setKeepMatrix(true);
//    this->addChild(m_pRT);

//    rt = RenderTexture::create(winSize1.width/2, winSize1.height/2, backend::PixelFormat::RGBA8888, backend::PixelFormat::D24S8);
//    rt->setPosition(winSize1.width/2, winSize1.height/2);
//    rt->setKeepMatrix(true);
//    this->addChild(rt);
//
//    m_pModel = cocos2d::Sprite3D::create("boss.c3b");
//    m_pModel->setScale(5);
//    m_pModel->setTexture("boss.png");
//    m_pModel->setPosition(winSize1.width/2,winSize1.height/2 + 200);
//    m_pModel->retain();
//    m_pModel->setForce2DQueue(true);
//
//
//
//    m_pRT->beginWithClear(1, 0, 0, 1.0f,1,0);
//
//    m_pModel->cocos2d::Node::visit(Director::getInstance()->getRenderer(),Mat4::IDENTITY,0);
//    sprite->visit(Director::getInstance()->getRenderer(), Mat4::IDENTITY, 0);
//
//    m_pRT->end();



//    scheduleUpdate();
    return true;
}

void HelloWorld::menuCloseCallback(Ref* sender)
{
    Director::getInstance()->end();
}

void HelloWorld::update(float ftime)
{
    auto tex = m_pRT;
    tex->beginWithClear(0, 0, 0, 0, 1, 0);
//    m_pModel->setPositionX(m_pModel->getPositionX() + 1);
    m_pModel->setVisible(true);
    m_pModel->cocos2d::Node::visit(Director::getInstance()->getRenderer(),Mat4::IDENTITY,0);
//    tsp->cocos2d::Node::visit(Director::getInstance()->getRenderer(),Mat4::IDENTITY,0);
    tex->end();
    m_pModel->setVisible(false);
}
