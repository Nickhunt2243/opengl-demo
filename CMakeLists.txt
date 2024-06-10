include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
cmake_minimum_required(VERSION 3.10)
project(OpenGLDemo)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(OpenGL REQUIRED)

include_directories(
        ${OPENGL_INCLUDE_DIRS}
        "${GLFW_PATH}/includes"
        ${GLM_PATH}
        "${GLAD_PATH}/include"
        "${STB_PATH}/includes"
        "${JSON_PATH}/includes"
)
link_directories("${GLFW_PATH}/lib-vc2022")

include_directories(external)

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

message(STATUS ${SOURCES})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
# Add GLAD source files
set(GLAD_SOURCES ${GLAD_PATH}/src/glad.c
        src/craft/world.hpp
        src/craft/world.cpp
        src/craft/player.hpp
        src/craft/player.cpp)

add_executable(OpenGLDemo ${SOURCES} ${GLAD_SOURCES})

# Set specific output directory for the executable to bin/
#set_target_properties(OpenGLDemo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Link the GLFW library
target_link_libraries(OpenGLDemo glfw3 ${OPENGL_LIBRARIES})

if (MSVC)
    # Ignore specific default libraries
    set_target_properties(OpenGLDemo PROPERTIES LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
endif()

add_custom_command(TARGET OpenGLDemo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:OpenGLDemo> ${CMAKE_SOURCE_DIR}/bin/$<TARGET_FILE_NAME:OpenGLDemo>
)

