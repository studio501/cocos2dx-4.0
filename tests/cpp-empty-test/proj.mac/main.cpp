/****************************************************************************
 Copyright (c) 2010 cocos2d-x.org
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

#include "../Classes/AppDelegate.h"
#include "../Classes/PolygonTool.h"

#include "../Classes/args.hxx"

#include <iostream>

USING_NS_CC;

int main(int argc, char *argv[])
{
//    AppDelegate app;
//    return Application::getInstance()->run();
//    SpriteFrameCache::getInstance()->addSpriteFramesWithFile("World_1.plist");
    
    
//    auto pt1 = new ConvertPlistWorker("/Users/mac/Downloads/111/eight_kingdom_map_1.plist", "/Users/mac/Downloads/111/World_1", 0.99, 256);
//
//    pt1->convert();
    
    
    
    args::ArgumentParser parser("This is a program convert texturepacker cocos2d-plist to polygon cocos2d-plist");
    args::HelpFlag help(parser, "HELP", "Show this help menu.", {'h', "help"});
    args::ValueFlag<std::string> plist_arg(parser, "plist", "Specify plist file path.", {'p',"plist"});
    args::ValueFlag<std::string> imags_arg(parser, "images", "Specify raw small images directory.", {'i',"images"});
    args::ValueFlag<float> uv_scale(parser, "uvscale", "Specify uv scale of texture default is 1.0.", {'s',"uvscale"});
    
    args::ValueFlag<int> max_verts(parser, "maxverts", "Specify max verts of one polygon defalut is 256.", {'m',"maxverts"});
    
    try
    {
        parser.ParseCLI(argc, argv);
        
        if(plist_arg && imags_arg){
            
            std::string plist = args::get(plist_arg);
            
            std::string images = args::get(imags_arg);
            
            float default_uv_scale = 1.0f;
            
            if(uv_scale){
                default_uv_scale = args::get(uv_scale);
            }
            
            int default_max_verts = 256;
            if(max_verts){
                default_max_verts = args::get(max_verts);
            }
            
            std::cout<<"start convert " << plist << "......\n";
            
            std::cout<<"use uv scale of " << default_uv_scale << "\n";
            
            std::cout<<"max verts count is " << default_max_verts << "\n";
            
            
            auto pt1 = new ConvertPlistWorker(plist, images, default_uv_scale, default_max_verts);
        
            if(pt1->convert()){
                std::cout<<"convert " << plist << " success." << "\n";
            }
        }
    }
    catch (args::Help)
    {
        std::cout << parser;
        return 0;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (args::ValidationError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    
    return 0;
}

