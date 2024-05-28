//
// Created by admin on 5/27/2024.
//

#ifndef OPENGLDEMO_COORDINATE_HPP
#define OPENGLDEMO_COORDINATE_HPP
#include <functional>

namespace Craft
{
    class Coordinate
    {
    public:

        Coordinate(float x, float y, float z);
        explicit operator float*();
        [[nodiscard]] float getX() const;
        [[nodiscard]] float getY() const;
        [[nodiscard]] float getZ() const;
        Coordinate add(float dX, float dY, float dZ) const;
        bool operator==(const Coordinate& other) const;
    private:
        float x, y, z;
    };
}

namespace std {
    template<>
    struct hash<Craft::Coordinate> {
        size_t operator()(const Craft::Coordinate &coord) const {
            size_t hx = hash<float>()(coord.getX());
            size_t hy = hash<float>()(coord.getY());
            size_t hz = hash<float>()(coord.getZ());
            return hx ^ (hy << 1) ^ (hz << 2); // Combine the hashes
        }
    };
}

#endif //OPENGLDEMO_COORDINATE_HPP
