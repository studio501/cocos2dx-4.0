//
//  PolygonTool.cpp
//  cpp-empty-test
//
//  Created by mac on 2021/12/29.
//

#include "cocos2d.h"

#include "PolygonTool.h"

#include <sstream>

#include <iostream>

USING_NS_CC;

std::string ConvertPlistWorker::add_last_slash(const std::string& inpath)
{
    if(inpath.find_last_of('/') != inpath.size() - 1){
        return inpath + "/";
    }
    return inpath;
}

bool ConvertPlistWorker::convert()
{
    if(!FileUtils::getInstance()->isFileExist(_plistPath)){
        std::cout<<"error: "<<_plistPath <<" is not exist." << "\n";
        return false;
    }
    
    if(!FileUtils::getInstance()->isDirectoryExist(_rawImagesPath)){
        std::cout<<"error: "<<_rawImagesPath <<" is not exist." << "\n";
        return false;
    }
    
    std::string images_path = _rawImagesPath;
//    "/Users/mac/Documents/my_projects/cok/client/CCB/IF/World/World_1/";
    
    ValueMap resDict;
    
    SpriteFrameCache::getInstance()->addSpriteFramesWithFile(_plistPath, resDict);
    
    auto& framesMap = SpriteFrameCache::getInstance()->getInnerCache().getSpriteFrames();
    
    auto framesItr = resDict.find("frames");
    
    if(framesItr == resDict.end())
    {
        std::cout<<"error: "<<_plistPath <<" is not a valid cocos2d-plist format." << "\n";
        return false;
    }
    
    ValueMap& framesDict = resDict["frames"].asValueMap();
    
    auto metaItr = resDict.find("metadata");
    if (metaItr != resDict.end())
    {
        ValueMap& metadataDict = metaItr->second.asValueMap();
        
        if(metadataDict["format"].asInt() == 4){
            std::cout<<"warning: "<<_plistPath <<" had converted and skip." << "\n";
            return false;
        }
    }
    
    
    std::ostringstream stream_vertices;
    std::ostringstream stream_uvs;
    std::ostringstream stream_indices;
    
    std::ostringstream temp_ss;
    
    for(auto& iter : framesMap){
        stream_vertices.str("");
        stream_vertices.clear();
        
        stream_uvs.str("");
        stream_uvs.clear();
        
        stream_indices.str("");
        stream_indices.clear();
        
        SpriteFrame* spriteFrame = iter.second;
        
        bool isRoate = spriteFrame->isRotated();
        
        if(!spriteFrame->getVerticesStr().empty()){
            std::cout<<"warning: "<<_plistPath <<" had converted and skip." << "\n";
            return false;
        }
        
        if(iter.first == "002.png"){
            int a = 100;
        }
        
        std::string relative_path = images_path + iter.first;
        
        spriteFrame->setRelativePngPath(images_path + iter.first);
        
        Rect sourceColor = spriteFrame->getSourceColorRect();
        
        Size originSize = spriteFrame->getOriginalSizeInPixels();
        
        auto pinfo = AutoPolygon::generatePolygon(relative_path,spriteFrame->getSourceColorRect(), 2.0f, 0.05f, _uvscale, _maxVerts);
        
        if(pinfo.triangles.vertCount == 0){
            if(pinfo.getAllTransparent()){
                auto frameItr = framesDict.find(iter.first);
                
                if(frameItr != framesDict.end()){
                    ValueMap& oneFrame = frameItr->second.asValueMap();
                    
                    auto frame_rect = spriteFrame->getRectInPixels();
                    
                    Rect new_rect = Rect(frame_rect.origin.x + frame_rect.size.width / 2,
                                         frame_rect.origin.y + frame_rect.size.height / 2,
                                         1, 1);
                    
                    temp_ss.str("");
                    temp_ss.clear();
                    
                    temp_ss << "{{" << new_rect.origin.x << "," << new_rect.origin.y <<"},{" << new_rect.size.width << "," << new_rect.size.height << "}}";
                    
                    
                    oneFrame["frame"] = temp_ss.str();
                    
                    temp_ss.str("");
                    temp_ss.clear();
                    
                    temp_ss << "{{" << 0 << "," << 0 <<"},{" << new_rect.size.width << "," << new_rect.size.height << "}}";
                    oneFrame["sourceColorRect"] = temp_ss.str();
                    
                    temp_ss.str("");
                    temp_ss.clear();
                    
                    
                    temp_ss << "{" << new_rect.size.width << "," << new_rect.size.height << "}";
                    oneFrame["sourceSize"] = temp_ss.str();
                    
                    temp_ss.str("");
                    temp_ss.clear();
                }
            }
            
            continue;
        }
        
        float fw = originSize.width;
        float fh = originSize.height;
        
        if(isRoate){
            fw = originSize.height;
            fh = originSize.width;
        }
        
        for(int i=0; i< pinfo.triangles.vertCount;i++){
            std::string end_char = (i == pinfo.triangles.vertCount - 1) ? "" : " ";
            
            float tu = pinfo.triangles.verts[i].texCoords.u;
            float tv = pinfo.triangles.verts[i].texCoords.v;
            
            float ox = sourceColor.origin.x;
            float oy = sourceColor.origin.y;
            
            
            if(isRoate){
                tu = 1 - pinfo.triangles.verts[i].texCoords.v;
                tv = pinfo.triangles.verts[i].texCoords.u;
                
                ox = originSize.height - sourceColor.size.height - sourceColor.origin.y;
                oy = sourceColor.origin.x;
            }

            int vx = std::round(pinfo.triangles.verts[i].vertices.x);
            int vy = std::round(pinfo.triangles.verts[i].vertices.y);
            
            stream_vertices << vx << " " << vy << end_char;
            
            float tw = tu * fw;
            float th = tv * fh;
            
            
            float rw = std::round( spriteFrame->getRectInPixels().origin.x - ox + tw );
            float rh = std::round( spriteFrame->getRectInPixels().origin.y - oy + th );
            
            stream_uvs << rw << " " << rh << end_char;
        }
        
        for(int i=0; i< pinfo.triangles.indexCount; i++){
            
            std::string end_char = (i == pinfo.triangles.indexCount - 1) ? "" : " ";

            stream_indices << (int)pinfo.triangles.indices[i] << end_char;
        }
        
        
        std::string vertices_str = stream_vertices.str();
        std::string uv_str = stream_uvs.str();
        std::string triangles_str = stream_indices.str();
        
        auto frameItr = framesDict.find(iter.first);
        
        if(frameItr != framesDict.end()){
            ValueMap& oneFrame = frameItr->second.asValueMap();
            
            oneFrame["triangles"] = Value(triangles_str);
            oneFrame["vertices"] = Value(vertices_str);
            oneFrame["verticesUV"] = Value(uv_str);
        }
        
        auto metaItr = resDict.find("metadata");
        if (metaItr != resDict.end())
        {
            ValueMap& metadataDict = metaItr->second.asValueMap();
            
            metadataDict["format"] = Value(4);
        }
    }
    
    FileUtils::getInstance()->writeToFile(resDict, _plistPath);
    
    return true;
}

void ConvertPlistWorker::workDone()
{
    SpriteFrameCache::getInstance()->removeSpriteFrames();
}
