#ifndef OPENGLDEMO_CAMERA_HPP
#define OPENGLDEMO_CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "./window.hpp"
#include "./program.hpp"

namespace Engine
{
    class Camera
    {
    public:
        /**
         * Instantiate a Camera object that will handle to View Matrix for shaders.
         *
         * @param window:  A pointer to the Window object.
         * @param program: A pointer to the Program object.
         * @param worldProgram: A pointer to the Program object.
         * @param width:        The width of the window.
         * @param height:       The height of the window.
         * @param x:            The x position of the player.
         * @param y:            The y position of the player.
         * @param z:            The z position of the player.
         */
        Camera(
            Window* window,
            Program* program,
            Program* worldProgram,
            uint32_t width,
            uint32_t height,
            float x, float y, float z
        );
        ~Camera() = default;
        /**
         * Initialize the camera object's fields.
         *
         * @return true if successful else false.
         */
        bool initCamera();
        /**
         * Update the Camera's view vector to reflect the user's inputs.
         *
         * Handles:
         *  - W: Move forward
         *  - S: Move backward
         *  - A: Move left
         *  - D: Move right
         *
         *  @return True if camera updated else false
         */
        bool updateCamera(glm::vec3 cameraPos, glm::vec3 cameraUp);
        /**
         * Callback for handling mouse movement for looking around.
         *
         * @param window: A pointer to the GLFW window.
         * @param xPos:   The current X position of the mouse.
         * @param yPos:   The current Y position of the mouse.
         */
        static void mouse_movement_callback(GLFWwindow* window, double xPos, double yPos);

        glm::vec3 getCameraFront();
    private:
        /// A pointer to a window object.
        Window* window;
        /// A pointer to a program object for block rendering.
        Program* program;
        /// A pointer to a program object for generic rendering.
        Program* worldProgram;
        /// The sensitivity of the camera.
        float sensitivity{0.075f};
        /// The height and width of the current window.
        uint32_t windowWidth,
                     windowHeight;
        /// The 3x1 vector describing the cameras front direction.
        glm::vec3 cameraFront{glm::vec3(-1.0f, -0.35f, 0.0f)};
        /// The 4x4 matrix describing the View transformation.
        glm::mat4 view{};//{glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp)};
        /// A Boolean that acts as a buffer for initial calls to mouse_movement_callback.
        static bool firstMouse;
        /// The previous X value of the mouse.
        static float lastX;
        /// The previous Y value of the mouse.
        static float lastY;
        /// The current angle in degrees of the camera for looking left and right.
        static float yaw;
        /// The current angle in degrees of the camera for looking up and down.
        static float pitch;
    };
}

#endif //OPENGLDEMO_CAMERA_HPP
