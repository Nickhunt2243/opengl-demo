#ifndef OPENGLDEMO_CUBE_HPP
#define OPENGLDEMO_CUBE_HPP

#include "./shape.hpp"

namespace Engine
{
    class Cube: public Shape
    {
    public:
        /**
         * Instantiate a Cube object.
         *
         * @param subdivisions: The number of subdivisions.
         */
        Cube(unsigned int subdivisions);
        ///Initialize the Cubes vertices, normals, UVs, and indices.
        void draw() override;
    private:
        /// The number of subdivisions to draw the cube with.
        unsigned int subdivisions;
    };
}

#endif //OPENGLDEMO_CUBE_HPP
