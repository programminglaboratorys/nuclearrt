#pragma once

struct CollisionInstanceBounds {
    static constexpr unsigned int NO_IMAGE = 0xFFFFFFFFu;

    short minX, minY, maxX, maxY;
    short width, height;
    short centerX, centerY;
    unsigned short angle;
    unsigned int imageId = NO_IMAGE;
    short hotspotX, hotspotY;
    short scrollX, scrollY;
    short maskWidth, maskHeight;
    float scaleX, scaleY;
    float cosInv = 1.0f, sinInv = 0.0f;
};