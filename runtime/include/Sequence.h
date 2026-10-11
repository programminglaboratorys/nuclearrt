#pragma once

#include <utility>
#include "Direction.h"
#include <vector>

class Sequence {
public:
    Sequence(std::unordered_map<int, Direction*> directions)
        : Directions(std::move(directions)) {}

    std::unordered_map<int, Direction*> Directions;
}; 