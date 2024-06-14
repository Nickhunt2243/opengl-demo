//
// Created by admin on 6/6/2024.
//

#ifndef OPENGLDEMO_COORDINATE_HPP
#define OPENGLDEMO_COORDINATE_HPP
#include <functional>
#include <glm/glm.hpp>

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

        inline static size_t compute_hash(float x, float z, float y) {
            size_t hx = std::hash<float>()(x);
            size_t hy = std::hash<float>()(y);
            size_t hz = std::hash<float>()(z);
            return hx ^ (hy << 1) ^ (hz << 2); // Combine the hashes
        }
    };
    /// A struct containing coordinate information for a given x and z position.
    struct Coordinate2D
    {
        Coordinate2D(float x, float z) : x{x}, z{z} {}
        /// The x position of the object.
        float x;
        /// The z position of the object.
        float z;

        /**
         * A helper function for finding the hashed value of a coordinate with the offset of dX, dZ.
         *
         * Used for finding neighbors efficiently.
         *
         * @param dX: The X offset.
         * @param dZ: The Z offset.
         * @return
         */
        [[nodiscard]] size_t add(float dX, float dZ) const
        {
            size_t hx = std::hash<float>()(x+dX);
            size_t hz = std::hash<float>()(z+dZ);
            return hx ^ (hz << 1); // Combine the hashes
        }
        /**
         * The subtraction operator for a 2D Coordinate.
         *
         * @param rhs: The right hand coordinate to subtract.
         * @return:    A 2D coordinate with the subtracted values.
         */
        inline Coordinate2D operator -(Coordinate2D rhs) const {
            return Coordinate2D{x - rhs.x, z - rhs.z};
        }
        /**
         * The subtraction operator for 2D Coordinates that allows you to subtract both elements by a single value.
         *
         * @param diff: The value to be subtracted.
         * @return:     A 2D coordinate with the subtracted values.
         */
        inline Coordinate2D operator -(float diff) const {
            return Coordinate2D{x - diff, z - diff};
        }
        /**
         * The multiplication operator for a 2D Coordinate.
         *
         * @param scalar: The scalar to be used in the multiplication.
         * @return:       A 2D coordinate with the values scaled.
         */
        inline Coordinate2D operator *(float scalar) const {
            return Coordinate2D{scalar * x, scalar *  z};
        }
        /**
         * The << operator for a 2D coordinate. Used for debugging purposes.
         *
         * @param os:    The stream to to be inserted into.
         * @param coord: The coordinate to input into the stream.
         * @return:      The stream with the updated coordinate string.
         */
        friend std::ostream& operator <<(std::ostream& os, const Coordinate2D& coord)
        {
            return os << "Coord2D(X: " << coord.x << ", Z: " << coord.z << ");";
        }
        /**
         * A glm::vec2 cast function for ease of sending data to openGL.
         *
         * @return: A glm::vec2 representing the 2D coordinate.
         */
        operator glm::vec2() const {
            return {x, z};
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

    /**
     * A function for determining the hash of a Coordinate2D pointer.
     */
    template<>
    struct hash<Craft::Coordinate2D>
    {
        size_t operator()(const Craft::Coordinate2D coord) const
        {
            size_t hx = hash<float>()(coord.x);
            size_t hz = hash<float>()(coord.z);
            return hx ^ (hz << 2); // Combine the hashes
        }
    };
    /**
     * A function for determining equality of coordinate2D pointers.
     */
    template<>
    struct equal_to<Craft::Coordinate2D>
    {
        bool operator()(const Craft::Coordinate2D lhs, const Craft::Coordinate2D rhs) const
        {
//            if (lhs == nullptr || rhs == nullptr) return lhs == rhs;
            return lhs.x == rhs.x &&
                   lhs.z == rhs.z;
        }
    };
}

#endif //OPENGLDEMO_COORDINATE_HPP
