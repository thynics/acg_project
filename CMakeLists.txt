cmake_minimum_required(VERSION 3.10)
project(OpenGLProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


add_subdirectory(external/glfw)

add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

add_library(tinyobjloader STATIC external/tinyobjloader/tiny_obj_loader.cc)
target_include_directories(tinyobjloader PUBLIC external/tinyobjloader)

add_library(stb_image STATIC src/stb_image_wrapper.cpp)
target_include_directories(stb_image PUBLIC external/stb)

# Include dirs
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    external/glad/include
    external/glfw/include
)


file(GLOB SRC_FILES src/*.cpp)

add_executable(OpenGLApp ${SRC_FILES})

target_include_directories(OpenGLApp PUBLIC external/glm)
target_link_libraries(OpenGLApp glad glfw ${CMAKE_DL_LIBS})
target_link_libraries(OpenGLApp glad glfw tinyobjloader)
target_link_libraries(OpenGLApp glad glfw tinyobjloader stb_image)
