#pragma once

#include <unordered_map>

struct ImageInfo {
    unsigned short Width;
    unsigned short Height;
    short HotspotX;
    short HotspotY;
    short ActionPointX;
    short ActionPointY;
    int TransparentColor;

    ImageInfo(unsigned short width, unsigned short height, short hotspotX, short hotspotY, short actionPointX, short actionPointY, int transparentColor)
        : Width(width), Height(height), HotspotX(hotspotX), HotspotY(hotspotY), ActionPointX(actionPointX), ActionPointY(actionPointY), TransparentColor(transparentColor) {}
};

class ImageBank {
public:
    static ImageBank& Instance() {
        static ImageBank instance;
        return instance;
    }
    
    const ImageInfo* GetImage(unsigned int handle) const {
        auto it = Images.find(handle);
        if (it != Images.end()) {
            return &it->second;
        }
        return nullptr;
    }
    
private:
    ImageBank();
    
    std::unordered_map<unsigned int, ImageInfo> Images;
};
