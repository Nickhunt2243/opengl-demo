#cmake_minimum_required(VERSION 3.10)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)
#project(OpenGLDemo)
#
#set(VCPKG_ROOT "C:/Users/admin/vcpkg")
#
#if(DEFINED VCPKG_ROOT)
#    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
#else()
#    message(FATAL_ERROR "VCPKG_ROOT not defined")
#endif()
#
#find_package(OpenGL REQUIRED)
#find_package(glfw3 CONFIG REQUIRED)
#find_package(glm CONFIG REQUIRED)
#find_package(nlohmann_json CONFIG REQUIRED)
##find_package(stb CONFIG REQUIRED)
#
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
#
#
## Add GLAD manually
#add_library(glad STATIC ./includes/glad/src/glad.c)
#target_include_directories(glad PUBLIC ./includes/glad/include)
#
## Add stb manually by including its header path directly
#include_directories("./includes/stb/includes")  # Adjust the path as necessary
#file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)
#add_executable(OpenGLDemo ${SOURCES})
#set_target_properties(OpenGLDemo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
#target_link_libraries(OpenGLDemo glfw glad OpenGL::GL glm::glm nlohmann_json::nlohmann_json)

cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(OpenGLDemo)

include(FetchContent)

# OpenGL is typically found on the system, so you may keep this as is.
find_package(OpenGL REQUIRED)

# FetchContent for GLFW
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4  # Specify the version you want
)
FetchContent_MakeAvailable(glfw)

# FetchContent for GLM
FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1  # Specify the version you want
)
FetchContent_MakeAvailable(glm)

# FetchContent for nlohmann-json
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3  # Specify the version you want
)
FetchContent_MakeAvailable(nlohmann_json)

# FetchContent for ImGui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.87  # Specify the version you want
)
FetchContent_MakeAvailable(imgui)

# Add GLAD manually
add_library(glad STATIC includes/glad/src/glad.c)
target_include_directories(glad PUBLIC includes/glad/include)

# Add stb manually by including its header path directly
include_directories("includes/stb/includes")

# Gather your source files
file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# Create the executable
add_executable(OpenGLDemo ${SOURCES})

set_target_properties(OpenGLDemo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Set working directory for the executable (for finding assets)
set_target_properties(OpenGLDemo PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# Link libraries
target_link_libraries(OpenGLDemo glfw glad OpenGL::GL glm::glm nlohmann_json::nlohmann_json)
