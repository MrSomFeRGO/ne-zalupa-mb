cmake_minimum_required(VERSION 3.20)
project(VoxelEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glad CONFIG REQUIRED)
# GLM
include(FetchContent)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG bf71a834948186f4097caa076cd2663c69a10e1e
)

FetchContent_MakeAvailable(glm)

# GLAD
#FetchContent_Declare(
#        glad
#        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
#        GIT_TAG v0.1.36
#)
#
#FetchContent_MakeAvailable(glad)

# Source files
set(SOURCES
        src/main.cpp
        src/core/Application.cpp
        src/core/Camera.cpp
        src/core/Input.cpp
        src/rendering/VoxelRenderer.cpp
        src/rendering/Shader.cpp
        src/rendering/Texture.cpp
        src/world/Block.cpp
        src/world/Chunk.cpp
        src/world/ChunkManager.cpp
        src/world/WorldGenerator.cpp
)

set(HEADERS
        src/core/Application.h
        src/core/Camera.h
        src/core/Input.h
        src/rendering/VoxelRenderer.h
        src/rendering/Shader.h
        src/rendering/Texture.h
        src/rendering/OpenGLUtils.h
        src/world/Block.h
        src/world/Chunk.h
        src/world/ChunkManager.h
        src/world/WorldGenerator.h
        src/utils/Math.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        external/FastNoiseLite
        external/stb
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        SDL3::SDL3
        OpenGL::GL
        glad::glad
        glm::glm
)

# Copy shaders
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)

# Copy textures
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets/textures
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets/textures
)