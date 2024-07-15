#ifndef OPENGLDEMO_CROSSHAIR_HPP
#define OPENGLDEMO_CROSSHAIR_HPP
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "../../setup/program.hpp"
#include "../../setup/window.hpp"

namespace Craft
{
    class CrossHair
    {
    public:
        CrossHair(Engine::Program* worldProgram, Engine::Window* window);
        ~CrossHair();
        void initCrossHair();
        void drawCrossHair(GLuint frameTexture);
    private:
        Engine::Program* program;
        Engine::Window* window;
        GLuint VAO, VBO;
        glm::mat4 identity {1.0f};
        glm::mat4 ortho{};
        glm::mat4 modelT{1.0f};
        glm::vec4 crossHairColor {0.8f, 0.8f, 0.8f, 0.5f};
        float scale = 100.0f;
        float crossHairWidth   = 0.015f * scale,
              crossHairHeight  = 0.14f * scale;

        std::vector<float> crossHairVertices {
            // Vertical rectangle (two triangles)
            crossHairWidth, crossHairHeight,
            -crossHairWidth, crossHairHeight,
            -crossHairWidth, -crossHairHeight,

            -crossHairWidth, -crossHairHeight,
            crossHairWidth, -crossHairHeight,
            crossHairWidth,  crossHairHeight,

            // Horizontal rectangle (two triangles)
            crossHairHeight,  crossHairWidth,
            -crossHairHeight,  crossHairWidth,
            -crossHairHeight, -crossHairWidth,

            -crossHairHeight, -crossHairWidth,
            crossHairHeight, -crossHairWidth,
            crossHairHeight,  crossHairWidth
        };

        void setUniforms();
    };

}
#endif //OPENGLDEMO_CROSSHAIR_HPP
