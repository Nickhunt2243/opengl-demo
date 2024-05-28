//
// Created by admin on 5/27/2024.
//

#include "coordinate.hpp"

namespace Craft
{
    Coordinate::Coordinate(float x, float y, float z)
        : x(x)
        , y(y)
        , z(z)
    {}

    float Coordinate::getX() const { return x; }
    float Coordinate::getY() const { return y; }
    float Coordinate::getZ() const { return z; }

    Coordinate Coordinate::add(float dX, float dY, float dZ) const
    {
        return Coordinate{x + dX, y + dY, z + dZ};
    }

    Coordinate::operator float*()
    {
        return &x;
    }

    bool Coordinate::operator==(const Coordinate& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
}