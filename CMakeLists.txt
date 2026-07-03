cmake_minimum_required(VERSION 3.10)
project(ClothSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

# glad vendorizado — evita o bug do GLEW 2.2.0 com Mesa moderno
add_library(glad STATIC external/glad/src/gl.c)
target_include_directories(glad PUBLIC external/glad/include)

add_executable(cloth_sim
    src/main.cpp
    src/Cloth.cpp
    src/Shader.cpp
)

target_link_libraries(cloth_sim
    glad
    OpenGL::GL
    glfw
)

file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})
