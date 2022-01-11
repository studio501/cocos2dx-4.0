//
//  main.cpp
//  PolygonTool
//
//  Created by mac on 2021/12/28.
//

#include <iostream>

#include "cocos2d.h"


int main(int argc, const char * argv[]) {
    // insert code here...
    
    cocos2d::SpriteFrameCache::getInstance()->addSpriteFramesWithFile("World_1.plist");
    
    std::cout << "Hello, World!\n";
    return 0;
}

