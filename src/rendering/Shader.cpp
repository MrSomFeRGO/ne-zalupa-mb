//
// Created by mrsomfergo on 12.07.2025.
//

#include "Shader.h"
#include <fstream>
#include <stdexcept>

Shader::Shader(nvrhi::IDevice* device, const std::string& filepath, nvrhi::ShaderType type)
    : m_type(type) {

    // Загружаем SPIR-V байткод
    std::vector<uint8_t> bytecode = LoadShaderFile(filepath);

    if (bytecode.empty()) {
        throw std::runtime_error("Failed to load shader: " + filepath);
    }

    // Создаем шейдер
    nvrhi::ShaderDesc shaderDesc;
    shaderDesc.shaderType = type;
    shaderDesc.debugName = filepath.c_str();

    m_shader = device->createShader(shaderDesc, bytecode.data(), bytecode.size());

    if (!m_shader) {
        throw std::runtime_error("Failed to create shader: " + filepath);
    }
}

Shader::~Shader() {
}

std::vector<uint8_t> Shader::LoadShaderFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        return {};
    }

    size_t fileSize = file.tellg();
    std::vector<uint8_t> buffer(fileSize);

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();

    return buffer;
}
