# OpenGL Demo

Welcome to the OpenGL Demo project! This project is a learning exercise inspired by the tutorials on [Learn OpenGL](https://learnopengl.com/). As I progress through the tutorials, I will be updating and expanding this repository.

## Getting Started

### Prerequisites

Before you can build and run this project, you need to have the following libraries installed:

- **OpenGL**: Version 4.6
- **GLM**: Version 1.0.1
- **GLFW**: Version 3.4
- **GLAD**: Generated version 0.1.36 from [GLAD generator](https://glad.dav1d.de/)

### Building the Project

1. **Clone the Repository**

   ```bash
   git clone https://github.com/your-username/OpenGLDemo.git
   cd OpenGLDemo
   ```

2. **Install Dependencies**

   Ensure that you have the required libraries installed on your system. You may need to set the paths for GLM, GLFW, and GLAD in your development environment.

3. **Generate Build Files**

   Use CMake to generate the build files for your preferred build system. For example, to generate Visual Studio project files:

   ```bash
   mkdir build
   cd build
   cmake .. -G "Visual Studio 16 2019"
   ```

4. **Build the Project**

   Open the generated project in Visual Studio (or your preferred IDE) and build the project.

   Alternatively, you can build from the command line:

   ```bash
   cmake --build .
   ```

### Running the Project

After successfully building the project, you can run the executable. The application should open a window and render the scene as described in the code.

### Project Structure

The project is organized into the following main components:

- **helpers**: Helper functions to be used globally.
- **setup**: Code for setting up and running the application (may change name in future).
- **shaders**: The GLSL Shaders used. May add additional folders to group various shaders as I move through chapters.
- **shapes**: Code for generating various shapes.

### Example Usage

```cpp
#include "app.hpp"

int main() {
    Engine::Application app;
    if (!app.initialize()) {
        return -1;
    }
    app.run();
    return 0;
}
```

### Contact

For any questions or inquiries, please contact nickrhunt@yahoo.com.

Thank you for checking out this project! Happy coding!
