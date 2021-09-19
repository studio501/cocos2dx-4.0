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

#include "extensions/GUI/CCScrollView/CCScrollView.h"
#include "PrismaticSprite.hpp"
#include "framepacker.hpp"

namespace fp = framepacker;

USING_NS_CC;


static HelloWorld* pHello = nullptr;

namespace Test1 {
using size_type = std::size_t;

using color_type = unsigned char;

struct rgba final {
    color_type data[4];

    inline static constexpr rgba from_buffer(const color_type* rgba_ptr) noexcept {
        return rgba {
            rgba_ptr[0],
            rgba_ptr[1],
            rgba_ptr[2],
            rgba_ptr[3]
        };
    }

    inline color_type& operator [] (size_type i) noexcept {
        return data[i];
    }

    inline const color_type& operator [] (size_type i) const noexcept {
        return data[i];
    }

    inline constexpr bool is_transparent() const noexcept {
        return data[3] == 0;
    }
};

class image {
public:
    ~image() {
            std::free(color_buffer);
            color_buffer = nullptr;
            w = 0;
            h = 0;
        }

        inline const char* get_path(void) const noexcept {
            return path.c_str();
        }

        size_type width(void) const noexcept {
            return w;
        }
        size_type height(void) const noexcept {
            return h;
        }

        inline constexpr const color_type* data(size_type x = 0, size_type y = 0) const noexcept {
            return &color_buffer[((y * w) + x) * 4];
        }

        inline color_type* data(size_type x = 0, size_type y = 0) noexcept {
            return &color_buffer[((y * w) + x) * 4];
        }

        inline constexpr rgba pixel(size_type x, size_type y) const noexcept {
            return rgba::from_buffer(&color_buffer[((y * w) + x) * 4]);
        }

        inline void pixel(size_type x, size_type y, const rgba &c) noexcept {
            auto* dst = data(x, y);
            dst[0] = c[0];
            dst[1] = c[1];
            dst[2] = c[2];
            dst[3] = c[3];
        }

        void copy_from(
            const image &src,
            size_type src_x,
            size_type src_y,
            size_type width,
            size_type height,
            size_type dst_x,
            size_type dst_y
        ) {
            // TODO : Worth checking if 'src' is ever equal to 'this'.
            // If not, then we don't need the temporary buffer.
            color_type* tmp = (color_type*)std::malloc(width * height * 4);
            if (!tmp) {
                return;
            }

            // Copy from source onto temporary buffer.
            for (size_type y = 0; y < height; y++) {
                const color_type* src_ptr = src.data(src_x, src_y + y);

                color_type* dst_ptr = &tmp[y * width * 4];

                std::memcpy(dst_ptr, src_ptr, width * 4);
            }

            // Copy from temporary buffer to destination.
            for (size_type y = 0; y < height; y++) {
                const color_type* src_ptr = &tmp[y * width * 4];

                color_type* dst_ptr = data(dst_x, dst_y + y);

                std::memcpy(dst_ptr, src_ptr, width * 4);
            }

            std::free(tmp);
        }

        inline constexpr bool is_transparent(size_type x, size_type y) const noexcept {
            return pixel(x, y).is_transparent();
        }

        bool resize(size_type _w, size_type _h) noexcept {
            auto* tmp = (color_type*)std::realloc(color_buffer, _w * _h * 4);
            if (!tmp) {
                return false;
            }

            color_buffer = tmp;
            w = _w;
            h = _h;

            return true;
        }

        bool load_file(const char* p, cocos2d::Image *pImg) noexcept {
            w = size_type(pImg->getWidth());
            h = size_type(pImg->getHeight());
            
            color_type* tmp = pImg->getData();
            
            std::free(color_buffer);
            color_buffer = tmp;

            path = p;

            return true;
        }

        bool save_png(const char* p) noexcept {
            //return stbi_write_png(p, w, h, 4, color_buffer, w * 4) != 0;
            auto out_image = new (std::nothrow) Image();
            auto tlen = sizeof(color_buffer);
            out_image->initWithRawData(color_buffer, 0, w, h, 8);
            
            auto out_texture = new (std::nothrow) Texture2D();
            out_texture->initWithImage(out_image);
            auto ts = Sprite::createWithTexture(out_texture);
            ts->setPosition(200,200);
            pHello->addChild(ts,1000);
            int a = 0;
            return true;
        }
    
private:
    std::string path;
    color_type* color_buffer = nullptr;
    size_type w = 0;
    size_type h = 0;
};



};

using packer_type = fp::packer<Test1::image>;

Scene* HelloWorld::createScene()
{
    return HelloWorld::create();
}

// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}



// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Scene::init() )
    {
        return false;
    }
    
    pHello = this;

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();
    printf("Writable Path is %s.\n", FileUtils::getInstance()->getWritablePath().c_str());
    
    
//    {
//        auto bgLayer = LayerColor::create(Color4B::WHITE, 2000, 2000);
//        //addChild(bgLayer, -1000);
//
//        auto label1 = Label::createWithSystemFont("hello", "Marker Felt", 30);
//        label1->updateContent();
//        label1->setPosition(100, 100);
////        label1->setColor(Color3B::WHITE);
//        addChild(label1,100);
//        return true;
//    }
    {
        packer_type packer;
        packer.padding = 2;
        packer.output_texture_size = fp::vec2(200, 200);
        packer.allow_rotate = true;
        packer.power_of_2 = false;
        packer.alpha_trim = true;
        packer.comparer = packer_type::compare_area;
        
        
        for(int i=0;i<2;i++){
            std::string key = "abcde" + std::to_string(i);
            auto label1 = Label::createWithSystemFont(key, "Marker Felt", 30);
            label1->setPosition(100, 100);
            label1->updateContent();
            
            auto sp = label1->getTextureSprite();
            auto img = sp->getTexture()->getBackImage();
            Test1::image* img1 = new Test1::image;
            img1->load_file(key.c_str(), img);


            packer_type::texture_type texture(img1);
            packer.add(key.c_str(), texture);
            
            addChild(label1,100);
        }
        
        // Pack it.
        packer_type::texture_type result(new Test1::image);
        packer_type::texture_coll_type packed;
        packer_type::texture_coll_type failed;
        packer.pack(result, packed, failed);
        
        // Prompt.
        {
            for (auto it = packed.begin(); it != packed.end(); ++it) {
                packer_type::block_type &blk = it->second;
                printf("  %s [%d, %d] - [%d, %d]\n", it->first.c_str(), blk.fit->min_x(), blk.fit->min_y(), blk.fit->min_x() + blk.fit->width(), blk.fit->min_y() + blk.fit->height());
            }
        }
        
        std::string outpath = FileUtils::getInstance()->getWritablePath() + "outpath";
        // Serialize.
        {
            std::string out_tex = outpath + ".png";
            result->save_png(out_tex.c_str());
            printf("Texture written: %s.\n", out_tex.c_str());
        }
        {
            std::string out_meta = outpath + ".json";
            std::ofstream fs(out_meta);
            packer.write_meta(fs, packed, result, outpath.c_str());
            fs.close();
            printf("Meta data written: %s.\n", out_meta.c_str());
        }
        
        return true;
    }
    
    {
        Director::getInstance()->setDisplayStats(false);
        auto visibleSize = Director::getInstance()->getVisibleSize();
        Vec2 origin = Director::getInstance()->getVisibleOrigin();
        
        {
            float sc = Director::getInstance()->getContentScaleFactor();
            {
                auto text = Director::getInstance()->getTextureCache()->addImage("World_1.pvr");
                auto r1 = Rect(4,4,256,140);
                
                auto r2 = Rect(r1.origin.x /sc, r1.origin.y / sc, r1.size.width / sc, r1.size.height/sc);
                auto sp = PrismaticSprite::createWithTexture(text,r2,false);
                sp->setPosition(visibleSize.width /2, visibleSize.height /2 - 30);
                addChild(sp);
            }
            
            {
                auto text = Director::getInstance()->getTextureCache()->addImage("HelloWorld.png");
                auto sp = PrismaticSprite::createWithTexture(text);
                
                sp->setPosition(visibleSize.width /2, visibleSize.height /2 + 100 - 30);
                addChild(sp);
            }
            
            return true;
        }
        
//        {
//            for(int i=0;i<10;++i){
//                auto label = Label::createWithTTF(std::string("ABA").append(std::to_string(i)) , "fonts/Marker Felt.ttf", 24);
//                label->setPosition(Vec2(origin.x + 100,
//                                        origin.y + visibleSize.height - label->getContentSize().height - i * 20));
//
//                label->setColor(Color3B(i*20,(10 - i)*20,i*15));
//                addChild(label,1);
//            }
//        }
        
        
        {
            auto sp = Sprite::create("HelloWorld.png");
            utils::captureNode(sp, [](Image *p){
    
                int a = 100;
            });
    
    
            int a = 100;
            return true;
        }
        {
            
            auto sf = SpriteFrameCache::getInstance();
            sf->addSpriteFramesWithFile("Particle.plist");
            
            
            auto _emitter = ParticleSystemQuad::create("NewInnerCloud.plist");
            _emitter->setDisplayFrame(sf->getSpriteFrameByName("Effect_0169.png"));
            _emitter->setPosition(100,100);
            _emitter->setScale(10);
            addChild(_emitter, 10);
            
            
            return true;
        }
        
        {
            
            auto scroll = cocos2d::extension::ScrollView::create(Size(100,100));
            scroll->setDirection(cocos2d::extension::ScrollView::Direction::VERTICAL);
            scroll->setClippingToBounds(true);
            scroll->setContentSize(Size(100,900));
//            addChild(scroll);
            scroll->setPosition(200, 0);
            
            
            for(int i=0;i<10;++i){
                auto label = Label::createWithTTF(std::string("ABA").append(std::to_string(i)) , "fonts/Marker Felt.ttf", 24);
//                label->setSystemFontSize(10 + i*2);
                label->setPosition(Vec2(0 + 50,
                                         300 - label->getContentSize().height - i * 20));
                
                label->setColor(Color3B(i*20,(10 - i)*20,i*15));
//                scroll->addChild(label,1);
                this->addChild(label,1);
            }
            
            
            for(int i=0;i<10;++i){
                auto label = Label::createWithTTF(std::string("ABA").append(std::to_string(i)) , "fonts/Marker Felt.ttf", 12);
//                label->setSystemFontSize(10 + i*2);
                label->setPosition(Vec2(0 + 150,
                                         300 - label->getContentSize().height - i * 20));
                
                label->setColor(Color3B(i*20,(10 - i)*20,i*15));
//                scroll->addChild(label,1);
                this->addChild(label,1);
            }
        }
        
        
        return true;
    }

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));

    if (closeItem == nullptr ||
        closeItem->getContentSize().width <= 0 ||
        closeItem->getContentSize().height <= 0)
    {
        problemLoading("'CloseNormal.png' and 'CloseSelected.png'");
    }
    else
    {
        float x = origin.x + visibleSize.width - closeItem->getContentSize().width/2;
        float y = origin.y + closeItem->getContentSize().height/2;
        closeItem->setPosition(Vec2(x,y));
    }

    // create menu, it's an autorelease object
    auto menu = Menu::create(closeItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

    /////////////////////////////
    // 3. add your codes below...

    // add a label shows "Hello World"
    // create and initialize a label

    auto label = Label::createWithTTF("Hello World", "fonts/Marker Felt.ttf", 24);
    if (label == nullptr)
    {
        problemLoading("'fonts/Marker Felt.ttf'");
    }
    else
    {
        // position the label on the center of the screen
        label->setPosition(Vec2(origin.x + visibleSize.width/2,
                                origin.y + visibleSize.height - label->getContentSize().height));

        // add the label as a child to this layer
        this->addChild(label, 1);
    }

    // add "HelloWorld" splash screen"
    auto sprite = Sprite::create("HelloWorld.png");
    if (sprite == nullptr)
    {
        problemLoading("'HelloWorld.png'");
    }
    else
    {
        // position the sprite on the center of the screen
        sprite->setPosition(Vec2(visibleSize.width/2 + origin.x, visibleSize.height/2 + origin.y));

        // add the sprite as a child to this layer
        this->addChild(sprite, 0);
    }
    return true;
}


void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() as given above,instead trigger a custom event created in RootViewController.mm as below*/

    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);


}
