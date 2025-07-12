//
// Created by mrsomfergo on 12.07.2025.
//

#pragma once

#include <nvrhi/nvrhi.h>
#include <string>
#include <vector>
#include <memory>

class Texture {
public:
    Texture(nvrhi::IDevice* device, nvrhi::ICommandList* commandList);
    ~Texture();

    bool LoadFromFile(const std::string& filepath, int width, int height);
    bool LoadFromMemory(const uint8_t* data, uint32_t width, uint32_t height, uint32_t channels);

    nvrhi::TextureHandle GetHandle() const { return m_texture; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }

private:
    void CreateTexture(const uint8_t* data);
    std::vector<uint8_t> GenerateDefaultTexture(uint32_t width, uint32_t height);

    nvrhi::DeviceHandle m_device;
    nvrhi::CommandListHandle m_commandList;
    nvrhi::TextureHandle m_texture;

    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_channels = 4;
};

class TextureManager {
public:
    TextureManager(nvrhi::IDevice* device, nvrhi::ICommandList* commandList);
    ~TextureManager();

    nvrhi::TextureHandle CreateTextureArray(const std::vector<std::string>& filepaths,
                                           uint32_t textureWidth, uint32_t textureHeight);

private:
    nvrhi::DeviceHandle m_device;
    nvrhi::CommandListHandle m_commandList;
    std::vector<std::unique_ptr<Texture>> m_textures;
};
