/****************************************************************************
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2011      Zynga Inc.
Copyright (c) 2013-2016 Chukong Technologies Inc.
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

#ifndef __CCLABEL_PACKER_H__
#define __CCLABEL_PACKER_H__

#include "base/CCRef.h"
#include "renderer/CCTexture2D.h"
#include "platform/CCImage.h"
#include "2d/CCLabel.h"

#include "framepacker.hpp"


NS_CC_BEGIN

namespace fp = framepacker;

namespace ns_labelpacker {

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
            //std::free(color_buffer);
            color_buffer = nullptr;
            _label = nullptr;
            w = 0;
            h = 0;
        }
    
        void release() {
            std::free(color_buffer);
            color_buffer = nullptr;
            _label = nullptr;
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
            auto* tmp = (color_type*)std::malloc(_w * _h * 4);
            if (!tmp) {
                return false;
            }
            std::memset(tmp, 0, _w * _h * 4);
            color_buffer = tmp;
            w = _w;
            h = _h;

            return true;
        }

        bool load_file(const char* p, cocos2d::Image *pImg, cocos2d::Label *pLabel) noexcept {
            w = size_type(pImg->getWidth());
            h = size_type(pImg->getHeight());
            
            color_type* tmp = pImg->getData();
            _label = pLabel;
            
//            std::free(color_buffer);
            color_buffer = tmp;

            path = p;
            
            
//            save_png(nullptr);
            return true;
        }

        bool save_png(const char* p) noexcept {
            //return stbi_write_png(p, w, h, 4, color_buffer, w * 4) != 0;
//            auto out_image = new (std::nothrow) Image();
//            auto tlen = sizeof(color_buffer);
//            out_image->initWithRawData(color_buffer, 0, w, h, 8);
//
//            auto out_texture = new (std::nothrow) Texture2D();
//            out_texture->initWithImage(out_image);
//            auto ts = Sprite::createWithTexture(out_texture);
//            ts->setPosition(200,200);
//            pHello->addChild(ts,1000);
//            pHello->setScale(0.5);
//            int a = 0;
            return true;
        }
    
    Texture2D* getTexture()
    {
        auto out_image = new (std::nothrow) Image();
        auto tlen = sizeof(color_buffer);
        out_image->initWithRawData(color_buffer, 0, w, h, 8);
        
        auto out_texture = new (std::nothrow) Texture2D();
        out_texture->initWithImage(out_image);
        return out_texture;
    }
    
    cocos2d::Label * getLabel() const {
        return _label;
    }
    
private:
    std::string path;
    color_type* color_buffer = nullptr;
    size_type w = 0;
    size_type h = 0;
    cocos2d::Label * _label = nullptr;
};



};



NS_CC_END

#endif //__CCLABEL_PACKER_H__

