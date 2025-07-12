//
// Created by mrsomfergo on 12.07.2025.
//

#pragma once

#include <nvrhi/nvrhi.h>
#include <string>
#include <vector>

class Shader {
public:
    Shader(nvrhi::IDevice* device, const std::string& filepath, nvrhi::ShaderType type);
    ~Shader();

    nvrhi::ShaderHandle GetHandle() const { return m_shader; }
    nvrhi::ShaderType GetType() const { return m_type; }

private:
    std::vector<uint8_t> LoadShaderFile(const std::string& filepath);

    nvrhi::ShaderHandle m_shader;
    nvrhi::ShaderType m_type;
};
