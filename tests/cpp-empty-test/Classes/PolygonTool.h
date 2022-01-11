//
//  PolygonTool.hpp
//  cpp-empty-test
//
//  Created by mac on 2021/12/29.
//

#ifndef PolygonTool_h
#define PolygonTool_h

#include "cocos2d.h"

#include <stdio.h>

#include <string>

class ConvertPlistWorker
{
public:
    ConvertPlistWorker(const std::string& plist, const std::string& rawImages, float uvscale = 1.0f, int maxVerts = 256 )
    :_plistPath(plist)
    ,_rawImagesPath(rawImages)
    ,_uvscale(uvscale)
    ,_maxVerts(maxVerts)
    {
        std::string plistPathDir = _plistPath.substr(0, _plistPath.find_last_of('/'))+"/";
        std::string rawImagesPath = _rawImagesPath.substr(0, _rawImagesPath.find_last_of('/'))+"/";
        
        
        _rawImagesPath = add_last_slash(_rawImagesPath);
        
        cocos2d::FileUtils::getInstance()->addSearchPath(plistPathDir);
        cocos2d::FileUtils::getInstance()->addSearchPath(rawImagesPath);
        
    }
    
    bool convert();
    
    
    void workDone();
    
    
private:
    std::string _plistPath;
    std::string _rawImagesPath;
    float _uvscale;
    int _maxVerts = 256;
    
    std::string add_last_slash(const std::string& inpath);
};

#endif /* PolygonTool_h */
