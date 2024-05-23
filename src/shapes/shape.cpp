#include "shape.hpp"

namespace Engine
{
    void Shape::addTriangle(
            float x0, float y0, float z0,
            float x1, float y1, float z1,
            float x2, float y2, float z2
        )
    {
        unsigned int nVerts = Shape::vertices.size() / 3;

        // First verts
        Shape::vertices.push_back(x0); Shape::bary.push_back(1.0);
        Shape::vertices.push_back(y0); Shape::bary.push_back(0.0);
        Shape::vertices.push_back(z0); Shape::bary.push_back(0.0);
        Shape::indices.push_back(nVerts);

        // Second verts
        Shape::vertices.push_back(x1); Shape::bary.push_back(0.0);
        Shape::vertices.push_back(y1); Shape::bary.push_back(1.0);
        Shape::vertices.push_back(z1); Shape::bary.push_back(0.0);
        Shape::indices.push_back(nVerts+1);

        // Third verts
        Shape::vertices.push_back(x2); Shape::bary.push_back(0.0);
        Shape::vertices.push_back(y2); Shape::bary.push_back(0.0);
        Shape::vertices.push_back(z2); Shape::bary.push_back(1.0);
        Shape::indices.push_back(nVerts+2);
    }
    void Shape::addNormal(
            float x0, float y0, float z0,
            float x1, float y1, float z1,
            float x2, float y2, float z2
        )
    {
        // First normals
        Shape::normals.push_back(x0);
        Shape::normals.push_back(y0);
        Shape::normals.push_back(z0);
        // Second normals
        Shape::normals.push_back(x1);
        Shape::normals.push_back(y1);
        Shape::normals.push_back(z1);
        // Third normals
        Shape::normals.push_back(x2);
        Shape::normals.push_back(y2);
        Shape::normals.push_back(z2);

    }
    void Shape::addUV(
            float u0, float v0,
            float u1, float v1,
            float u2, float v2
        )
    {
        // First UV
        Shape::uv.push_back(u0);
        Shape::uv.push_back(v0);
        // Second UV
        Shape::uv.push_back(u0);
        Shape::uv.push_back(v0);
        // Third UV
        Shape::uv.push_back(u0);
        Shape::uv.push_back(v0);
    }
    std::vector<float> Shape::getVertices() const
    {
        return Shape::vertices;
    }
    std::vector<unsigned int> Shape::getIndices() const
    {
        return Shape::indices;
    }
    std::vector<float> Shape::getNormals() const
    {
        return Shape::normals;
    }

    std::vector<float> Shape::getUVs() const
    {
        return Shape::uv;
    }
}
