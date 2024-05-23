#include <iostream>

#include "cube.hpp"

namespace Engine
{
    Cube::Cube(unsigned int subdivisions)
        : Shape()
        , subdivisions(subdivisions)
    {
        Cube::draw();
    };
    void Cube::draw()
    {
        float d=-0.5f;
        float s=-0.5f;
        float inc= 1.0f / (float) subdivisions;
        float r, n;
        for (unsigned int i=0; i<subdivisions; i++)
        {
            r = i * inc;
            for (unsigned int h=0; h<subdivisions; h++)
            {
                n = h * inc;
                // Back
                Shape::addTriangle(d + n,s + r,0.5f, d + n + inc,s + r + inc,0.5f, d + n + inc,s + r,0.5f);
                Shape::addNormal(0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1);
                Shape::addUV(n,r,n+inc,r,n+inc,r+inc);

                Shape::addTriangle(d + n + inc,s + r + inc,0.5f,d + n,s + r + inc,0.5f,d + n,s + r,0.5f);
                Shape::addNormal(0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1);
                Shape::addUV(n + inc,r + inc,n,r + inc,n,r);

                // Front
                Shape::addTriangle(d + n,s + r,-0.5f,d + n + inc,s + r + inc,-0.5f,d + n + inc,s + r,-0.5f);
                Shape::addNormal(0.0f,0.0f,-1.0f,0.0f,0.0f,-1.0f,0.0f,0.0f,-1);
                Shape::addUV(n,r,n + inc,r + inc,n + inc,r);

                Shape::addTriangle(d + n,s + r,-0.5f,d + n,s + r + inc,-0.5f,d + n + inc,s + r + inc,-0.5f);
                Shape::addNormal(0.0f,0.0f,-1.0f,0.0f,0.0f,-1.0f,0.0f,0.0f,-1);
                Shape::addUV(n,r,n,r + inc,n + inc,r + inc);
                // Left
                Shape::addTriangle(-0.5f,s + r,d + n,-0.5f,s + r,d + n + inc,-0.5f,s + r + inc,d + n + inc);
                Shape::addNormal(-1.0f,0.0f,0.0f,-1.0f,0.0f,0.0f,-1.0f,0.0f,0);
                Shape::addUV(n,r,n + inc,r,n + inc,r + inc);

                Shape::addTriangle(-0.5f,s + r + inc,d + n + inc,-0.5f,s + r + inc,d + n,-0.5f,s + r,d + n);
                Shape::addNormal(-1.0f,0.0f,0.0f,-1.0f,0.0f,0.0f,-1.0f,0.0f,0);
                Shape::addUV(n + inc,r + inc,n,r + inc,n,r);
                // Right
                Shape::addTriangle(0.5f,s + r,d + n + inc,0.5f,s + r,d + n,0.5f,s + r + inc,d + n);
                Shape::addNormal(1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0);
                Shape::addUV(n + inc,r,n,r,n,r + inc);

                Shape::addTriangle(0.5f,s + r + inc,d + n,0.5f,s + r + inc,d + n + inc,0.5f,s + r,d + n + inc);
                Shape::addNormal(1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0);
                Shape::addUV(n,r + inc,n + inc,r + inc,n + inc,r);
                // Top
                Shape::addTriangle(d + n,0.5f,s + r + inc,d + n + inc,0.5f,s + r + inc,d + n,0.5f,s + r);
                Shape::addNormal(0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0);
                Shape::addUV(n,r + inc,n + inc,r + inc,n,r);

                Shape::addTriangle(d + n,0.5f,s + r,d + n + inc,0.5f,s + r + inc,d + n + inc,0.5f,s + r);
                Shape::addNormal(0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0);
                Shape::addUV(n,r,n + inc,r + inc,n + inc,r);
                // Bottom
                Shape::addTriangle(d + n,-0.5f,s + r,d + n + inc,-0.5f,s + r + inc,d + n,-0.5f,s + r + inc);
                Shape::addNormal(0.0f,-1.0f,0.0f,0.0f,-1.0f,0.0f,0.0f,-1.0f,0);
                Shape::addUV(n,r,n + inc,r + inc,n,r + inc);

                Shape::addTriangle(d + n,-0.5f,s + r,d + n + inc,-0.5f,s + r,d + n + inc,-0.5f,s + r + inc);
                Shape::addNormal(0.0f,-1.0f,0.0f,0.0f,-1.0f,0.0f,0.0f,-1.0f,0);
                Shape::addUV(n,r,n + inc,r,n + inc,r + inc);
            }
        }
    }
}