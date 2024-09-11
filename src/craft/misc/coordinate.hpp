//
// Created by admin on 6/6/2024.
//

#ifndef OPENGLDEMO_COORDINATE_HPP
#define OPENGLDEMO_COORDINATE_HPP
#include <type_traits>
#include <functional>
#include <glm/glm.hpp>
#include <ostream>

namespace Craft
{
    /// An typename for Coordinate2D to ensure it only holds numerical types.
    template<typename T>
    using is_numeric = std::integral_constant<bool,
            std::is_integral<T>::value ||
            std::is_floating_point<T>::value>;

    /// A struct containing coordinate information for a given block.
    template<typename T, typename = std::enable_if_t<is_numeric<T>::value>>
    struct Coordinate
    {
        Coordinate() :x{0}, y{0}, z{0} {}
        Coordinate(
                T x, T y, T z
            ) : x{x}, y{y}, z{z}
        {}
        /// The x position of the object.
        T x;
        /// The y position of the object.
        T y;
        /// The z position of the object.
        T z;


        /**
         * A glm::vec2 cast function for ease of sending data to openGL.
         *
         * @return: A glm::vec2 representing the 2D coordinate.
         */
        operator glm::vec3() const {
            return {x, y, z};
        }
        operator glm::ivec3() const {
            return {(int) x, (int) y, (int) z};
        }
        /**
         * The << operator for a 2D coordinate. Used for debugging purposes.
         *
         * @param os:    The stream to to be inserted into.
         * @param coord: The coordinate to input into the stream.
         * @return:      The stream with the updated coordinate string.
         */
        friend std::ostream& operator <<(std::ostream& os, const Coordinate<T>& coord)
        {
            return os << "Coord(X: " << coord.x  << ", Y: " << coord.y << ", Z: " << coord.z << ");";
        }
        /**
         * The addition operator for a Coordinate.
         *
         * @param rhs: The right hand coordinate to add.
         * @return:    A coordinate with the added values.
         */
        inline Coordinate<T> operator +(Coordinate<T> rhs) const {
            return Coordinate<T>{x + rhs.x, y + rhs.y, z + rhs.z};
        }
        /**
         * The equality operator for a Coordinate.
         *
         * @param other: The right hand coordinate to compare.
         * @return:      A Boolean of whether the coordinates are equal.
         */
        inline bool operator==(const Coordinate<T>& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
    };

    /// A struct containing coordinate information for a given x and z position.
    /// I chose X and Z since I mostly use this struct for x and z coordinates within the world.
    template<typename T, typename = std::enable_if_t<is_numeric<T>::value>>
    struct Coordinate2D
    {
        Coordinate2D(T x, T z) : x{x}, z{z} {}
        Coordinate2D() :x{0}, z{0} {}
        /// The x position of the object.
        T x;
        /// The z position of the object.
        T z;

        /**
         * A helper function for finding the hashed value of a coordinate with the offset of dX, dZ.
         *
         * Used for finding neighbors efficiently.
         *
         * @param dX: The X offset.
         * @param dZ: The Z offset.
         * @return
         */
        [[nodiscard]] size_t add(T dX, T dZ) const
        {
            size_t hx = std::hash<float>()(x + dX);
            size_t hz = std::hash<float>()(z + dZ);
            return hx ^ (hz << 1); // Combine the hashes
        }
        /**
         * The subtraction operator for a 2D Coordinate.
         *
         * @param rhs: The right hand coordinate to subtract.
         * @return:    A 2D coordinate with the subtracted values.
         */
        inline Coordinate2D<T> operator -(Coordinate2D<T> rhs) const {
            return Coordinate2D<T>{x - rhs.x, z - rhs.z};
        }
        /**
         * The addition operator for a 2D Coordinate.
         *
         * @param rhs: The right hand coordinate to add.
         * @return:    A 2D coordinate with the added values.
         */
        inline Coordinate2D<T> operator +(Coordinate2D<T> rhs) const {
            return Coordinate2D<T>{x + rhs.x, z + rhs.z};
        }
        /**
         * The subtraction operator for 2D Coordinates that allows you to subtract both elements by a single value.
         *
         * @param diff: The value to be subtracted.
         * @return:     A 2D coordinate with the subtracted values.
         */
        inline Coordinate2D<T> operator -(T diff) const {
            return Coordinate2D<T>{x - diff, z - diff};
        }
        /**
         * The multiplication operator for a 2D Coordinate.
         *
         * @param scalar: The scalar to be used in the multiplication.
         * @return:       A 2D coordinate with the values scaled.
         */
        inline Coordinate2D<T> operator *(T scalar) const {
            return Coordinate2D<T>{scalar * x, scalar *  z};
        }
        /**
         * The equality operator for a 2D Coordinate.
         *
         * @param other: The other coordinate used within the comparison.
         * @return:      True if the coordinates x and z values are equal.
         */
        inline bool operator ==(Coordinate2D<T> other) const {
            return x == other.x && z == other.z;
        }
        /**
         * The << operator for a 2D coordinate. Used for debugging purposes.
         *
         * @param os:    The stream to to be inserted into.
         * @param coord: The coordinate to input into the stream.
         * @return:      The stream with the updated coordinate string.
         */
        friend std::ostream& operator <<(std::ostream& os, const Coordinate2D<T>& coord)
        {
            return os << "Coord2D(X: " << coord.x << ", Z: " << coord.z << ");";
        }
        /**
         * A glm::vec2 cast function for ease of sending data to openGL.
         *
         * @return: A glm::vec2 representing the 2D coordinate.
         */
        operator glm::vec2() const {
            return {(float) x, (float) z};
        }

        operator glm::ivec2() const {
            return {(int) x, (int) z};
        }
    };
}


namespace std
{

/**
 * A function for determining the hash of a Coordinate pointer.
 */
template<typename T>
struct hash<Craft::Coordinate<T>>
{
    size_t operator()(const Craft::Coordinate<T> coord) const
    {
        size_t hx = hash<T>()(coord.x);
        size_t hy = hash<T>()(coord.y);
        size_t hz = hash<T>()(coord.z);
        return hx ^ (hy << 1) ^ (hz << 2); // Combine the hashes
    }
};
/**
 * A function for determining equality of coordinate pointers.
 */
template<typename T>
struct equal_to<Craft::Coordinate<T>>
{
    bool operator()(const Craft::Coordinate<T> lhs, const Craft::Coordinate<T> rhs) const
    {
        return lhs.x == rhs.x &&
               lhs.y == rhs.y &&
               lhs.z == rhs.z;
    }
};

    /**
     * A function for determining the hash of a Coordinate2D pointer.
     */
    template<typename T>
    struct hash<Craft::Coordinate2D<T>>
    {
        size_t operator()(const Craft::Coordinate2D<T> coord) const
        {
            size_t hx = hash<T>()(coord.x);
            size_t hz = hash<T>()(coord.z);
            return hx ^ (hz << 2); // Combine the hashes
        }
    };
    /**
     * A function for determining equality of coordinate2D pointers.
     */
    template<typename T>
    struct equal_to<Craft::Coordinate2D<T>>
    {
        bool operator()(const Craft::Coordinate2D<T> lhs, const Craft::Coordinate2D<T> rhs) const
        {
//            if (lhs == nullptr || rhs == nullptr) return lhs == rhs;
            return lhs.x == rhs.x &&
                   lhs.z == rhs.z;
        }
    };
}

#endif //OPENGLDEMO_COORDINATE_HPP
