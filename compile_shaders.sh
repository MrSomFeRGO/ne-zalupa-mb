#!/bin/bash

echo "Compiling shaders..."

# Проверяем наличие glslc
if ! command -v glslc &> /dev/null; then
    echo "Error: glslc not found. Please install Vulkan SDK."
    exit 1
fi

# Создаем директорию для скомпилированных шейдеров если её нет
mkdir -p shaders

# Компилируем вершинный шейдер
echo "Compiling vertex shader..."
glslc -fshader-stage=vert shaders/voxel.vert -o shaders/voxel.vert.spv
if [ $? -ne 0 ]; then
    echo "Failed to compile vertex shader!"
    exit 1
fi

# Компилируем фрагментный шейдер
echo "Compiling fragment shader..."
glslc -fshader-stage=frag shaders/voxel.frag -o shaders/voxel.frag.spv
if [ $? -ne 0 ]; then
    echo "Failed to compile fragment shader!"
    exit 1
fi

echo "Shaders compiled successfully!"