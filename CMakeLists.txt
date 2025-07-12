cmake_minimum_required(VERSION 3.20)
project(VoxelEngine)
set(NVRHI_BUILD_SHARED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(NVRHI_WITH_VULKAN TRUE CACHE BOOL "" FORCE)
# Поиск пакетов
find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED)

# NVRHI
add_subdirectory(external/nvrhi)

# GLM
include(FetchContent)

FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	bf71a834948186f4097caa076cd2663c69a10e1e #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm)
#FetchContent_Declare(
#        volk
#        GIT_REPOSITORY https://github.com/zeux/volk.git
#        GIT_TAG        master # или зафиксируй конкретную версию
#)
#
#FetchContent_MakeAvailable(volk)
# STB для загрузки изображений
#add_subdirectory()

file(GLOB NVRHI_UTILS_SRC CONFIGURE_DEPENDS
        external/nvrhi/src/utils/*.cpp)
# Исходные файлы
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
        ${NVRHI_UTILS_SRC}
)

set(HEADERS
        src/core/Application.h
        src/core/Camera.h
        src/core/Input.h
        src/rendering/VoxelRenderer.h
        src/rendering/Shader.h
        src/rendering/Texture.h
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
#        ${volk_SOURCE_DIR}/volk
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Vulkan::Vulkan
#        volk
        SDL3::SDL3
        nvrhi
#        nvrhi_vk
        glm::glm
)

# Копирование шейдеров
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)

# Копирование текстур
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets/textures
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets/textures
)