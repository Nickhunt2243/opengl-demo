#include "sun.hpp"

#include "glm/ext/matrix_transform.hpp"
#include "../../helpers/helpers.hpp"

namespace Craft {

    Sun::Sun(Engine::Program *worldProgram) : worldProgram{worldProgram} {};
    Sun::~Sun()
    {
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
    }
    void Sun::initSun()
    {
        clockTimer.resetTimer();
        // Initialize the time, sky color, and angle of sun/moon
        calcNewTime();
        calcSkyColor();
        calcNewAngle();

        glGenBuffers(1, &VBO);
        glGenVertexArrays(1, &VAO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        auto size = (GLsizeiptr) (solarObjectVertices.size() * sizeof(float));
        glBufferData(GL_ARRAY_BUFFER, size, solarObjectVertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    void Sun::updateModel(float objectX, float objectY, float objectZ, float angle)
    {
        auto modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(objectX, objectY, objectZ));
        modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), glm::vec3(0, 0, 1));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(SUN_DISTANCE, 0, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2, SUN_WIDTH, SUN_WIDTH));
        setMat4(worldProgram->getProgram(), "u_modelT", modelMatrix);
    }
    void Sun::updateSun()
    {
        calcNewTime();
        calcSkyColor();
        calcNewAngle();
    }
    void Sun::calcNewTime()
    {
        float minecraftTimeDiff = TIME_CONVERSION * clockTimer.getTimeSpan();
        gameTime.seconds += minecraftTimeDiff;
        if (gameTime.seconds > 60)
        {
            gameTime.minutes += (int) gameTime.seconds / 60;
            gameTime.seconds = fmod(gameTime.seconds, 60.0f);
        }

        if (gameTime.minutes > 60)
        {
            gameTime.hours += gameTime.minutes / 60;
            gameTime.minutes = gameTime.minutes % 60;
        }

        if (gameTime.hours > 24)
        {
            gameTime.hours = 0 + (gameTime.hours % 24);
        }
    }
    void Sun::calcNewAngle()
    {
        // Moon will last from 17:00 to 07:00
        if ((gameTime.hours >= 17 || gameTime.hours <= 7) && !drawMoon)
        {
            drawMoon = true;
        }
        else if (gameTime.hours > 7 && gameTime.hours < 17 && drawMoon)
        {
            drawMoon = false;
        }
        // Sun will last from 05:00 to 19:00
        if (gameTime.hours >= 5 && gameTime.hours <= 19 && !drawSun)
        {
            drawSun = true;
        }
        else if ((gameTime.hours <= 5 || gameTime.hours >= 19) && drawSun)
        {
            drawSun = false;
        }
        float slope = 330.0f / 24.0f;
        if (drawSun)
        {
            sunAngle = slope * (gameTime.getFullTime() - 5.455f);
        }
        if (drawMoon)
        {
            if (gameTime.hours >= 0 && gameTime.hours <= 7)
            {
                moonAngle = slope * (gameTime.getFullTime() + 6.85f);
            }
            else if (gameTime.hours >= 17 && gameTime.hours < 24)
            {
                moonAngle = slope * (gameTime.getFullTime() - 17.15f);
            }
        }
    }
    void Sun::calcSkyColor()
    {
        float x = gameTime.getFullTime();
        float xScalar = (M_PI / 12.0f);
        if (x >= 6 && x <= 18)
        {
            skyColor.r = 45 * sin((x - 6) * xScalar) + 76;
            skyColor.g = 62 * sin((x - 6) * xScalar) + 107;
            skyColor.b = 92 * sin((x - 6) * xScalar) + 162;
        }
        else
        {
            skyColor.r = 76 * sin((x - 6) * xScalar) + 76;
            skyColor.g = 107 * sin((x - 6) * xScalar) + 107;
            skyColor.b = 162 * sin((x - 6) * xScalar) + 162;
        }
    }
    void Sun::drawLight(float playerX, float playerY, float playerZ)
    {
        // Set the atmosphere color (will change to use cube maps to add more of a gradient for the sky as it switches)
        // Day time:      rgb(121, 169, 254) @ 12   / 90
        // Dusk/twilight: rgb(76,  107, 162) @ 6|18 / 0|180
        // Night time:    rgb(0,   0,   0)   @ 0    / 270
        glClearColor(skyColor.r / 255, skyColor.g / 255, skyColor.b / 255, 1.0f);
        worldProgram->useProgram();
        glBindVertexArray(VAO);
        if (drawSun)
        {
            updateModel(playerX, playerY, playerZ, sunAngle);
            setVec4(worldProgram->getProgram(), "u_colorMapping", sunColor);
            glDrawArrays(GL_TRIANGLES, 0, (GLint) solarObjectVertices.size() / 3);
        }
        if (drawMoon)
        {
            updateModel(playerX, playerY, playerZ, moonAngle);
            setVec4(worldProgram->getProgram(), "u_colorMapping", moonColor);
            glDrawArrays(GL_TRIANGLES, 0, (GLint) solarObjectVertices.size() / 3);
        }
        glBindVertexArray(0);
    }
}