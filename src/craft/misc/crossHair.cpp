#include "crossHair.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "../../helpers/helpers.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"

namespace Craft
{
    CrossHair::CrossHair(Engine::Program* orthoProgram, Engine::Window* window)
            : program{orthoProgram}, window{window}, VAO(0), VBO(0)
    {}

    CrossHair::~CrossHair()
    {
        if (VAO != 0) {
            glDeleteVertexArrays(1, &VAO);
        }
        if (VBO != 0) {
            glDeleteBuffers(1, &VBO);
        }
    }

    void CrossHair::initCrossHair()
    {

        auto halfWindowWidth = (float) window->getWidth() / 2.0f;
        auto halfWindowHeight = (float) window->getHeight() / 2.0f;
        ortho = glm::ortho(-halfWindowWidth, halfWindowWidth, -halfWindowHeight, halfWindowHeight);
        program->useProgram();
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr) (crossHairVertices.size() * sizeof(float)), crossHairVertices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void CrossHair::setUniforms()
    {
        setMat4(program->getProgram(), "u_projT", ortho);
        setMat4(program->getProgram(), "u_modelT", identity);

        auto windowWidth = (float) window->getWidth();
        auto windowHeight = (float) window->getHeight();
        glm::vec2 screenDim{windowWidth, windowHeight};
        setVec2(program->getProgram(), "u_screenDimensions", screenDim);
    }

    void CrossHair::drawCrossHair(GLuint frameTexture)
    {
        program->useProgram();
        setUniforms();
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, (GLint) crossHairVertices.size() / 2);
        glBindVertexArray(0);
    }
}
