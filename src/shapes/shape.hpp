#ifndef OPENGLDEMO_SHAPE_HPP
#define OPENGLDEMO_SHAPE_HPP

#include <vector>

namespace Engine
{
    class Shape
    {
    public:
        Shape() = default;
        ~Shape() = default;
        /**
         * A virtual function for drawing specific shapes.
         *
         * Implemented within child classes.
         */
        virtual void draw() {};
        /**
         * Adds the following coordinates (representing a triangle) to the vertices, bary, and indices vector's
         *
         * @param x0: First X coordinate
         * @param y0: First Y coordinate
         * @param z0: First Z coordinate
         * @param x1: Second X coordinate
         * @param y1: Second Y coordinate
         * @param z1: Second Z coordinate
         * @param x2: Third X coordinate
         * @param y2: Third Y coordinate
         * @param z2: Third Z coordinate
         */
        void addTriangle(
                float x0, float y0, float z0,
                float x1, float y1, float z1,
                float x2, float y2, float z2
            );
        /**
         * Adds three normalized coordinates (representing a triangle) to the normals vector.
         *
         * @param x0: First X coordinate
         * @param y0: First Y coordinate
         * @param z0: First Z coordinate
         * @param x1: Second X coordinate
         * @param y1: Second Y coordinate
         * @param z1: Second Z coordinate
         * @param x2: Third X coordinate
         * @param y2: Third Y coordinate
         * @param z2: Third Z coordinate
         */
        void addNormal(
                float x0, float y0, float z0,
                float x1, float y1, float z1,
                float x2, float y2, float z2
        );
        /**
         * Adds three UV coordinate (representing a triangle) to the uv vector.
         *
         * @param u0: First u point
         * @param v0: First v point
         * @param u1: Second u point
         * @param v1: Second v point
         * @param u2: Third u point
         * @param v2: Third v point
         */
        void addUV(
                float u0, float v0,
                float u1, float v1,
                float u2, float v2
        );
        /**
         * Retrieve the Vertices Vector.
         *
         * @return A vector of floats containing the vertices.
         */
        std::vector<float> getVertices() const;
        /**
         * Retrieve the Indices Vector.
         *
         * @return A vector of floats containing the indices.
         */
        std::vector<unsigned int> getIndices() const;
        /**
         * Retrieve the Normal Vector.
         *
         * @return A vector of floats containing the normals.
         */
        std::vector<float> getNormals() const;
        /**
         * Retrieve the UV Vector.
         *
         * @return A vector of floats containing the UVs.
         */
        std::vector<float> getUVs() const;
    private:
        /// A vector of vertices.
        std::vector<float> vertices{};
        /// A vector of bary coordinates for use within shaders.
        std::vector<float> bary{};
        /// A vector of indices for rendering.
        std::vector<unsigned int> indices{};
        /// A vector of normals for use within shaders.
        std::vector<float> normals{};
        /// A vector of UVs for use within shaders.
        std::vector<float> uv{};
    };
}

#endif //OPENGLDEMO_SHAPE_HPP
