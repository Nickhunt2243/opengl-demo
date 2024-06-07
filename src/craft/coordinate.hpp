//
// Created by admin on 6/6/2024.
//

#ifndef OPENGLDEMO_COORDINATE_HPP
#define OPENGLDEMO_COORDINATE_HPP
#include <functional>

namespace Craft
{
    /// A struct containing coordinate information for a given block.
    struct Coordinate
    {
        Coordinate(
                float x, float y, float z
            ) : x{x}, y{y}, z{z}
        {}
        /// The x position of the object.
        float x;
        /// The y position of the object.
        float y;
        /// The z position of the object.
        float z;

        /**
         * A helper function for finding the hashed value of a coordinate with the offset of dX, dY, dZ.
         *
         * Used for finding neighbors efficiently.
         *
         * @param dX: The X offset.
         * @param dY: The Y offset.
         * @param dZ: The Z offset.
         * @return
         */
        [[nodiscard]] size_t add(float dX, float dY, float dZ) const
        {
            size_t hx = std::hash<float>()(x+dX);
            size_t hy = std::hash<float>()(y+dY);
            size_t hz = std::hash<float>()(z+dZ);
            return hx ^ (hy << 1) ^ (hz << 2); // Combine the hashes
        }
    };
}


namespace std
{
    /**
     * A function for determining the hash of a Coordinate pointer.
     */
    template<>
    struct hash<Craft::Coordinate*>
    {
        size_t operator()(const Craft::Coordinate* coord) const
        {
            if (coord == nullptr) return 0;
            size_t hx = hash<float>()(coord->x);
            size_t hy = hash<float>()(coord->y);
            size_t hz = hash<float>()(coord->z);
            return hx ^ (hy << 1) ^ (hz << 2); // Combine the hashes
        }
    };
    /**
     * A function for determining equality of coordinate pointers.
     */
    template<>
    struct equal_to<Craft::Coordinate*>
    {
        bool operator()(const Craft::Coordinate* lhs, const Craft::Coordinate* rhs) const
        {
            if (lhs == nullptr || rhs == nullptr) return lhs == rhs;
            return lhs->x == rhs->x &&
                   lhs->y == rhs->y &&
                   lhs->z == rhs->z;
        }
    };
}

#endif //OPENGLDEMO_COORDINATE_HPP
