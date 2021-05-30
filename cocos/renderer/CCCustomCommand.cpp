/****************************************************************************
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
#include "renderer/CCCustomCommand.h"
#include "renderer/CCTextureAtlas.h"
#include "renderer/backend/Buffer.h"
#include "renderer/backend/Device.h"
#include "base/ccUtils.h"
#include <sstream>


NS_CC_BEGIN
    static std::string buffer2key(std::size_t vertexSize, std::size_t indexSize, backend::IndexFormat format, backend::BufferUsage usage)
    {
        std::stringstream ss;
        ss << vertexSize << ":" << indexSize << ":" << (int)format << (int)usage;
        return ss.str();
    }

    struct VIBucket
    {
        std::vector<backend::Buffer*> _vertexBufferPool;
        std::vector<backend::Buffer*> _indexBufferPool;
        int _currentBufferIndex = 0;

        std::size_t _vertexSize;
        std::size_t _indexSize;
        backend::IndexFormat _format;
        backend::BufferUsage _usage;

        VIBucket(std::size_t vertexSize, std::size_t indexSize, backend::IndexFormat format, backend::BufferUsage usage)
                :_vertexSize(vertexSize)
                ,_indexSize(indexSize)
                ,_format(format)
                ,_usage(usage)
        {
            init();
        }

        bool needIndex(){
            return 0 < _indexSize;
        }

        void releaseBuffer(){
            --_currentBufferIndex;
        }


        backend::Buffer* getVertexBuffer(){
            if(_currentBufferIndex >= (int)_vertexBufferPool.size()){
                createBuffer();
            }
            if(needIndex())
                return _vertexBufferPool[_currentBufferIndex];
            else
                return _vertexBufferPool[_currentBufferIndex++];
        }

        // must call after getVertexBuffer
        // because _vertexBufferPool and _indexBufferPool share _currentBufferIndex
        backend::Buffer* getIndexBuffer(){
            if(!needIndex()){
                return nullptr;
            }
            return _indexBufferPool[_currentBufferIndex++];
        }
        void init(){
            createBuffer();
        }

        std::size_t computeIndexSize() const
        {
            if (backend::IndexFormat::U_SHORT == _format)
                return sizeof(unsigned short);
            else
                return sizeof(unsigned int);
        }

        void createBuffer(){
            auto device = backend::Device::getInstance();

            backend::Buffer * indexBuffer = nullptr;
            bool isNeedIndex = needIndex();
#ifdef CC_USE_METAL
            // Metal doesn't need to update buffer to make sure it has the correct size.
        auto vertexBuffer = device->newBuffer(_vertexSize, backend::BufferType::VERTEX, _usage);
        if (!vertexBuffer)
            return;
        if(isNeedIndex){
            indexBuffer = device->newBuffer(_indexSize * computeIndexSize(), backend::BufferType::INDEX, _usage);
            if (!indexBuffer)
            {
                vertexBuffer->release();
                return;
            }
        }
#else
            auto tmpData = malloc(_vertexSize);
            if (!tmpData)
                return;

            auto vertexBuffer = device->newBuffer(_vertexSize, backend::BufferType::VERTEX, _usage);
            if (!vertexBuffer)
            {
                free(tmpData);
                return;
            }
            vertexBuffer->updateData(tmpData, _vertexSize);
            if(isNeedIndex){
                indexBuffer = device->newBuffer(_indexSize * computeIndexSize(), backend::BufferType::INDEX, _usage);
                if (! indexBuffer)
                {
                    free(tmpData);
                    vertexBuffer->release();
                    return;
                }
                indexBuffer->updateData(tmpData, _indexSize * computeIndexSize());
            }

            free(tmpData);
#endif

            _vertexBufferPool.push_back(vertexBuffer);
            if(isNeedIndex){
                _indexBufferPool.push_back(indexBuffer);
            }
        }
        ~VIBucket(){
            for (auto& vertexBuffer : _vertexBufferPool)
                vertexBuffer->release();

            for (auto& indexBuffer : _indexBufferPool)
                indexBuffer->release();

            _currentBufferIndex = 0;
        }
    };

    class VIBufferCache
    {

    public:
        static VIBufferCache* _instance;
        static VIBufferCache* getInstance()
        {
            if(!_instance)
                _instance = new VIBufferCache();

            return _instance;
        }

        VIBucket& getBucket(std::size_t vertexSize, std::size_t indexSize, backend::IndexFormat format, backend::BufferUsage usage){
            std::string key = buffer2key(vertexSize, indexSize, format, usage);
            auto iter = _pool.find(key);
            if(iter != _pool.end()){
                return *iter->second;
            }

            auto *b = new VIBucket(vertexSize, indexSize, format, usage);
            _pool.insert(std::pair<std::string, VIBucket*>(key, b));
            return *b;
        }

        void releaseBucket(std::string key){
            auto iter = _pool.find(key);
            if(iter != _pool.end()){
                iter->second->releaseBuffer();
            }
        }

    private:
        std::map<std::string, VIBucket*> _pool;
    };

    VIBufferCache* VIBufferCache::_instance = nullptr;


    CustomCommand::CustomCommand()
{
    _type = RenderCommand::Type::CUSTOM_COMMAND;
}

CustomCommand::~CustomCommand()
{
    if(_viKey.empty()){
        CC_SAFE_RELEASE(_vertexBuffer);
        CC_SAFE_RELEASE(_indexBuffer);
    }else{
        VIBufferCache::getInstance()->releaseBucket(_viKey);
    }


}

void CustomCommand::init(float depth, const cocos2d::Mat4 &modelViewTransform, unsigned int flags)
{
    RenderCommand::init(depth, modelViewTransform, flags);
}

void CustomCommand::init(float globalZOrder)
{
    _globalOrder = globalZOrder;
}

void CustomCommand::init(float globalZOrder, const BlendFunc& blendFunc)
{
    _globalOrder = globalZOrder;

    auto& blendDescriptor = _pipelineDescriptor.blendDescriptor;
    blendDescriptor.blendEnabled = true;
    blendDescriptor.sourceRGBBlendFactor = blendDescriptor.sourceAlphaBlendFactor = blendFunc.src;
    blendDescriptor.destinationRGBBlendFactor = blendDescriptor.destinationAlphaBlendFactor = blendFunc.dst;
}

void CustomCommand::createVIBuffer(std::size_t vertexSize, std::size_t capacity, std::size_t indexSize, backend::IndexFormat format, backend::BufferUsage usage)
{
    _viKey = buffer2key(vertexSize * capacity, indexSize, format, usage);
    auto& bucket = VIBufferCache::getInstance()->getBucket(vertexSize * capacity, indexSize, format, usage);

    CC_SAFE_RELEASE(_vertexBuffer);

    _vertexCapacity = capacity;
    _vertexDrawCount = capacity;


    _vertexBuffer = bucket.getVertexBuffer();

    CC_SAFE_RELEASE(_indexBuffer);

    _indexFormat = format;
    _indexSize = computeIndexSize();
    _indexCapacity = indexSize;
    _indexDrawCount = indexSize;

    _indexBuffer = bucket.getIndexBuffer();

}



    void CustomCommand::createVertexBuffer(std::size_t vertexSize, std::size_t capacity, BufferUsage usage)
{
    CC_SAFE_RELEASE(_vertexBuffer);
    
    _vertexCapacity = capacity;
    _vertexDrawCount = capacity;
    
    auto device = backend::Device::getInstance();
    _vertexBuffer = device->newBuffer(vertexSize * capacity, backend::BufferType::VERTEX, usage);
}

void CustomCommand::createIndexBuffer(IndexFormat format, std::size_t capacity, BufferUsage usage)
{
    CC_SAFE_RELEASE(_indexBuffer);
    
    _indexFormat = format;
    _indexSize = computeIndexSize();
    _indexCapacity = capacity;
    _indexDrawCount = capacity;
    
    auto device = backend::Device::getInstance();
    _indexBuffer = device->newBuffer(_indexSize * capacity, backend::BufferType::INDEX, usage);
}

void CustomCommand::updateVertexBuffer(void* data, std::size_t offset, std::size_t length)
{   
    assert(_vertexBuffer);
    _vertexBuffer->updateSubData(data, offset, length);
}

void CustomCommand::updateIndexBuffer(void* data, std::size_t offset, std::size_t length)
{
    assert(_indexBuffer);
    _indexBuffer->updateSubData(data, offset, length);
}

void CustomCommand::setVertexBuffer(backend::Buffer *vertexBuffer)
{
    if (_vertexBuffer == vertexBuffer)
        return;

    _vertexBuffer = vertexBuffer;
    CC_SAFE_RETAIN(_vertexBuffer);
}

void CustomCommand::setIndexBuffer(backend::Buffer *indexBuffer, IndexFormat format)
{
    if (_indexBuffer == indexBuffer && _indexFormat == format)
        return;

    _indexBuffer = indexBuffer;
    CC_SAFE_RETAIN(_indexBuffer);

    _indexFormat = format;
    _indexSize = computeIndexSize();
}

void CustomCommand::updateVertexBuffer(void* data, std::size_t length)
{
    assert(_vertexBuffer);
    _vertCount = length;
    _vertexStart = (V3F_C4B_T2F*)data;
    _vertexBuffer->updateData(data, length);
}

void CustomCommand::updateIndexBuffer(void* data, std::size_t length)
{
    assert(_indexBuffer);
    _indexCount = length;
    _indexStart = (unsigned short *)data;
    _indexBuffer->updateData(data, length);
}

std::size_t CustomCommand::computeIndexSize() const
{
if (IndexFormat::U_SHORT == _indexFormat)
    return sizeof(unsigned short);
else
    return sizeof(unsigned int);
}

NS_CC_END
