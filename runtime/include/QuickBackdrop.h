#pragma once

#include <vector>
#include <memory>

#include "ObjectInstance.h"
#include "Shape.h"

class QuickBackdrop : public ObjectInstance {
public:
	QuickBackdrop(unsigned int objectInfoHandle, int type, std::string name)
		: ObjectInstance(objectInfoHandle, type, name) {}

	unsigned int ObstacleType;
	unsigned int CollisionType;
	int Width;
    int Height;
	Shape shape;

	void GetImagesUsed(std::vector<unsigned int>& imagesUsed) override {
		imagesUsed.push_back(shape.Image);
	}
};