//
//  PrismaticSprite.cpp
//  HelloCpp
//
//  Created by mac on 2021/6/18.
//

#include "PrismaticSprite.hpp"
#include "base/CCDirector.h"

PrismaticSprite* PrismaticSprite::createWithTexture(Texture2D *texture)
{
    PrismaticSprite *sprite = new (std::nothrow) PrismaticSprite();
    if (sprite && sprite->initWithTexture(texture))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

PrismaticSprite* PrismaticSprite::createWithTexture(Texture2D *texture, const Rect& rect, bool rotated)
{
    PrismaticSprite *sprite = new (std::nothrow) PrismaticSprite();
    if (sprite && sprite->initWithTexture(texture, rect, rotated))
    {
        sprite->autorelease();
        return sprite;
    }
    CC_SAFE_DELETE(sprite);
    return nullptr;
}

void PrismaticSprite::setTextureCoords(const Rect& rectInPoints, V3F_C4B_T2F_Quad* outQuad)
{
    Texture2D *tex = (_renderMode == RenderMode::QUAD_BATCHNODE) ? _textureAtlas->getTexture() : _texture;
    
    if (tex == nullptr)
        return;

    const auto rectInPixels = CC_RECT_POINTS_TO_PIXELS(rectInPoints);

    const float atlasWidth = (float)tex->getPixelsWide();
    const float atlasHeight = (float)tex->getPixelsHigh();
   
    float rw = rectInPixels.size.width;
    float rh = rectInPixels.size.height;

    // if the rect is rotated, it means that the frame is rotated 90 degrees (clockwise) and:
    //  - rectInpoints: origin will be the bottom-left of the frame (and not the top-right)
    //  - size: represents the unrotated texture size
    //
    // so what we have to do is:
    //  - swap texture width and height
    //  - take into account the origin
    //  - flip X instead of Y when flipY is enabled
    //  - flip Y instead of X when flipX is enabled

    if (_rectRotated)
        std::swap(rw, rh);

#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
    float left    = (2*rectInPixels.origin.x+1) / (2*atlasWidth);
    float right   = left+(rw*2-2) / (2*atlasWidth);
    float top     = (2*rectInPixels.origin.y+1) / (2*atlasHeight);
    float bottom  = top+(rh*2-2) / (2*atlasHeight);
#else
    float left    = rectInPixels.origin.x / atlasWidth;
    float right   = (rectInPixels.origin.x + rw) / atlasWidth;
    float top     = rectInPixels.origin.y / atlasHeight;
    float bottom  = (rectInPixels.origin.y + rh) / atlasHeight;
#endif // CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
    
    float half_right = right / 2.0f;
    float half_bottom = bottom / 2.0f;


    if ((!_rectRotated && _flippedX) || (_rectRotated && _flippedY))
        std::swap(left, right);

    if ((!_rectRotated && _flippedY) || (_rectRotated && _flippedX))
        std::swap(top, bottom);

    if (_rectRotated)
    {
        outQuad->bl.texCoords.u = left;
        outQuad->bl.texCoords.v = top;
        outQuad->br.texCoords.u = left;
        outQuad->br.texCoords.v = bottom;
        outQuad->tl.texCoords.u = right;
        outQuad->tl.texCoords.v = top;
        outQuad->tr.texCoords.u = right;
        outQuad->tr.texCoords.v = bottom;
    }
    else
    {
        outQuad->bl.texCoords.u = left;
        outQuad->bl.texCoords.v = half_bottom;
        
        outQuad->br.texCoords.u = half_right;
        outQuad->br.texCoords.v = bottom;
        
        outQuad->tl.texCoords.u = half_right;
        outQuad->tl.texCoords.v = top;
        
        outQuad->tr.texCoords.u = right;
        outQuad->tr.texCoords.v = half_bottom;
    }
}


void PrismaticSprite::setVertexCoords(const Rect& rect, V3F_C4B_T2F_Quad* outQuad)
{
    if(true){
        Sprite::setVertexCoords(rect, outQuad);
        return;
    }
    float relativeOffsetX = _unflippedOffsetPositionFromCenter.x;
    float relativeOffsetY = _unflippedOffsetPositionFromCenter.y;

    // issue #732
    if (_flippedX)
        relativeOffsetX = -relativeOffsetX;
    if (_flippedY)
        relativeOffsetY = -relativeOffsetY;

    _offsetPosition.x = relativeOffsetX + (_originalContentSize.width - _rect.size.width) / 2;
    _offsetPosition.y = relativeOffsetY + (_originalContentSize.height - _rect.size.height) / 2;

    // FIXME: Stretching should be applied to the "offset" as well
    // but probably it should be calculated in the caller function. It will be tidier
    if (_renderMode == RenderMode::QUAD)
    {
        _offsetPosition.x *= _stretchFactor.x;
        _offsetPosition.y *= _stretchFactor.y;
    }

    // rendering using batch node
    if (_renderMode == RenderMode::QUAD_BATCHNODE)
    {
        // update dirty_, don't update recursiveDirty_
        setDirty(true);
    }
    else
    {
        // self rendering

        // Atlas: Vertex
        const float x1 = 0.0f + _offsetPosition.x + rect.origin.x;
        const float y1 = 0.0f + _offsetPosition.y + rect.origin.y;
        const float x2 = x1 + rect.size.width;
        const float y2 = y1 + rect.size.height;
        const float half_x2 = x2 / 2;
        const float half_y2 = y2 / 2;

        // Don't update Z.
        outQuad->bl.vertices.set(x1, half_y2, 0.0f);
        outQuad->br.vertices.set(half_x2, y2, 0.0f);
        outQuad->tl.vertices.set(half_x2, y1, 0.0f);
        outQuad->tr.vertices.set(x2, half_y2, 0.0f);
    }
}

void PrismaticSprite::updatePoly()
{
    // center rect
    float cx1 = _centerRectNormalized.origin.x;
    float cy1 = _centerRectNormalized.origin.y;
    float cx2 = _centerRectNormalized.origin.x + _centerRectNormalized.size.width;
    float cy2 = _centerRectNormalized.origin.y + _centerRectNormalized.size.height;

    // "O"riginal rect
    const float oox = _rect.origin.x;
    const float ooy = _rect.origin.y;
    float osw = _rect.size.width;
    float osh = _rect.size.height;

    if (_rectRotated)
    {
        std::swap(cx1, cy1);
        std::swap(cx2, cy2);

        // when the texture is rotated, then the centerRect starts from the "bottom" (left)
        // but when it is not rotated, it starts from the top, so invert it
        cy2 = 1 - cy2;
        cy1 = 1 - cy1;
        std::swap(cy1, cy2);
        std::swap(osw, osh);
    }

    //
    // textCoords Data: Y must be inverted.
    //
    const float w0 = osw * cx1;
    const float w1 = osw * (cx2-cx1);
    const float w2 = osw * (1-cx2);
    const float h0 = osh * cy1;
    const float h1 = osh * (cy2-cy1);
    const float h2 = osh * (1-cy2);

    const float u0 = oox;
    const float u1 = u0 + w0;
    const float u2 = u1 + w1;
    const float v2 = ooy;
    const float v1 = v2 + h2;
    const float v0 = v1 + h1;


    const Rect texRects_normal[9] = {
        Rect(u0, v0,    w0, h0),   // bottom-left
        Rect(u1, v0,    w1, h0),   // bottom
        Rect(u2, v0,    w2, h0),   // bottom-right

        Rect(u0, v1,    w0, h1),   // left
        Rect(u1, v1,    w1, h1),   // center
        Rect(u2, v1,    w2, h1),   // right

        Rect(u0, v2,    w0, h2),   // top-left
        Rect(u1, v2,    w1, h2),   // top
        Rect(u2, v2,    w2, h2),   // top-right
    };

    // swap width and height because setTextureCoords()
    // will expects the hight and width to be swapped
    const Rect texRects_rotated[9] = {
        Rect(u0, v2,    h2, w0),        // top-left
        Rect(u0, v1,    h1, w0),        // left
        Rect(u0, v0,    h0, w0),        // bottom-left

        Rect(u1, v2,    h2, w1),        // top
        Rect(u1, v1,    h1, w1),        // center
        Rect(u1, v0,    h0, w1),        // bottom

        Rect(u2, v2,    h2, w2),        // top-right
        Rect(u2, v1,    h1, w2),        // right
        Rect(u2, v0,    h0, w2),        // bottom-right
    };

    const Rect* texRects = _rectRotated ? texRects_rotated : texRects_normal;

    //
    // vertex Data.
    //

    // reset center rect since it is altered when when the texture
    // is rotated
    cx1 = _centerRectNormalized.origin.x;
    cy1 = _centerRectNormalized.origin.y;
    cx2 = _centerRectNormalized.origin.x + _centerRectNormalized.size.width;
    cy2 = _centerRectNormalized.origin.y + _centerRectNormalized.size.height;
    if (_rectRotated)
        std::swap(osw, osh);

    // sizes
    float x0_s = osw * cx1;
    float x1_s = osw * (cx2-cx1) * _stretchFactor.x;
    float x2_s = osw * (1-cx2);
    float y0_s = osh * cy1;
    float y1_s = osh * (cy2-cy1) * _stretchFactor.y;
    float y2_s = osh * (1-cy2);


    // avoid negative size:
    if (_contentSize.width < x0_s + x2_s)
        x2_s = x0_s = _contentSize.width / 2;

    if  (_contentSize.height < y0_s + y2_s)
        y2_s = y0_s = _contentSize.height / 2;


    // is it flipped?
    // swap sizes to calculate offset correctly
    if (_flippedX)
        std::swap(x0_s, x2_s);
    if (_flippedY)
        std::swap(y0_s, y2_s);

    // origins
    float x0 = 0;
    float x1 = x0 + x0_s;
    float x2 = x1 + x1_s;
    float y0 = 0;
    float y1 = y0 + y0_s;
    float y2 = y1 + y1_s;

    // swap origin, but restore size to its original value
    if (_flippedX)
    {
        std::swap(x0, x2);
        std::swap(x0_s, x2_s);
    }
    if (_flippedY)
    {
        std::swap(y0, y2);
        std::swap(y0_s, y2_s);
    }

    const Rect verticesRects[9] = {
        Rect(x0, y0,  x0_s, y0_s),      // bottom-left
        Rect(x1, y0,  x1_s, y0_s),      // bottom
        Rect(x2, y0,  x2_s, y0_s),      // bottom-right

        Rect(x0, y1,  x0_s, y1_s),      // left
        Rect(x1, y1,  x1_s, y1_s),      // center
        Rect(x2, y1,  x2_s, y1_s),      // right

        Rect(x0, y2,  x0_s, y2_s),      // top-left
        Rect(x1, y2,  x1_s, y2_s),      // top
        Rect(x2, y2,  x2_s, y2_s),      // top-right
    };

    // needed in order to get color from "_quad"
    V3F_C4B_T2F_Quad tmpQuad = _quad;

    for (int i=0; i<9; ++i)
    {
        setTextureCoords(texRects[i], &tmpQuad);
        setVertexCoords(verticesRects[i], &tmpQuad);
        populateTriangle(i, tmpQuad);
    }
    TrianglesCommand::Triangles triangles;
    triangles.verts = _trianglesVertex;
    triangles.vertCount = 16;
    triangles.indices = _trianglesIndex;
    triangles.indexCount = 6 * 9;   // 9 quads, each needs 6 vertices

    // probably we can update the _trianglesCommand directly
    // to avoid memcpy'ing stuff
    _polyInfo.setTriangles(triangles);
}
