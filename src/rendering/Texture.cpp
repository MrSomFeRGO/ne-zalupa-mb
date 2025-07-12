//
// Created by mrsomfergo on 12.07.2025.
//

#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include <stdexcept>
#include <cstring>

Texture::Texture(nvrhi::IDevice* device, nvrhi::ICommandList* commandList)
    : m_device(device)
    , m_commandList(commandList) {
}

Texture::~Texture() {
}

bool Texture::LoadFromFile(const std::string& filepath, int width, int height) {
    int texWidth, texHeight, texChannels;
    stbi_set_flip_vertically_on_load(false);

    uint8_t* data = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    if (!data) {
        // Если не удалось загрузить, создаем текстуру по умолчанию
        std::vector<uint8_t> defaultData = GenerateDefaultTexture(width, height);
        CreateTexture(defaultData.data());
        m_width = width;
        m_height = height;
        return false;
    }

    // Если размеры не совпадают, масштабируем
    if (texWidth != width || texHeight != height) {
        uint8_t* scaledData = new uint8_t[width * height * 4];
        stbir_resize_uint8_linear(data, texWidth, texHeight, 0,
                          scaledData, width, height, 0, STBIR_RGB);

        CreateTexture(scaledData);
        delete[] scaledData;
    } else {
        CreateTexture(data);
    }

    stbi_image_free(data);

    m_width = width;
    m_height = height;
    m_channels = 4;

    return true;
}

bool Texture::LoadFromMemory(const uint8_t* data, uint32_t width, uint32_t height, uint32_t channels) {
    m_width = width;
    m_height = height;
    m_channels = channels;

    if (channels == 3) {
        // Конвертируем RGB в RGBA
        std::vector<uint8_t> rgbaData(width * height * 4);
        for (uint32_t i = 0; i < width * height; ++i) {
            rgbaData[i * 4 + 0] = data[i * 3 + 0];
            rgbaData[i * 4 + 1] = data[i * 3 + 1];
            rgbaData[i * 4 + 2] = data[i * 3 + 2];
            rgbaData[i * 4 + 3] = 255;
        }
        CreateTexture(rgbaData.data());
    } else {
        CreateTexture(data);
    }

    return true;
}

void Texture::CreateTexture(const uint8_t* data) {
    nvrhi::TextureDesc textureDesc;
    textureDesc.width = m_width;
    textureDesc.height = m_height;
    textureDesc.format = nvrhi::Format::RGBA8_UNORM;
    textureDesc.mipLevels = 1;
    textureDesc.dimension = nvrhi::TextureDimension::Texture2D;
    textureDesc.initialState = nvrhi::ResourceStates::ShaderResource;
    textureDesc.keepInitialState = true;

    m_texture = m_device->createTexture(textureDesc);

    // Загружаем данные в текстуру
    m_commandList->writeTexture(m_texture, 0, 0, data, m_width * 4);
}

std::vector<uint8_t> Texture::GenerateDefaultTexture(uint32_t width, uint32_t height) {
    std::vector<uint8_t> data(width * height * 4);

    // Создаем шахматный паттерн розового и черного цветов (missing texture)
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            bool checker = ((x / 4) + (y / 4)) % 2 == 0;
            uint32_t idx = (y * width + x) * 4;

            if (checker) {
                data[idx + 0] = 255; // R
                data[idx + 1] = 0;   // G
                data[idx + 2] = 255; // B
                data[idx + 3] = 255; // A
            } else {
                data[idx + 0] = 0;   // R
                data[idx + 1] = 0;   // G
                data[idx + 2] = 0;   // B
                data[idx + 3] = 255; // A
            }
        }
    }

    return data;
}

// TextureManager implementation
TextureManager::TextureManager(nvrhi::IDevice* device, nvrhi::ICommandList* commandList)
    : m_device(device)
    , m_commandList(commandList) {
}

TextureManager::~TextureManager() {
}

nvrhi::TextureHandle TextureManager::CreateTextureArray(const std::vector<std::string>& filepaths,
                                                       uint32_t textureWidth, uint32_t textureHeight) {
    // Загружаем все текстуры
    std::vector<std::vector<uint8_t>> textureData;

    for (const auto& filepath : filepaths) {
        auto texture = std::make_unique<Texture>(m_device.Get(), m_commandList.Get());
        texture->LoadFromFile(filepath, textureWidth, textureHeight);

        // Читаем данные из текстуры (в реальном проекте это нужно оптимизировать)
        std::vector<uint8_t> data(textureWidth * textureHeight * 4);
        // Здесь нужен код для чтения данных из текстуры, но это зависит от API

        // Для демонстрации генерируем уникальные цвета для каждой текстуры
        for (uint32_t i = 0; i < textureWidth * textureHeight; ++i) {
            data[i * 4 + 0] = (filepaths.size() * 20) % 255;
            data[i * 4 + 1] = (filepaths.size() * 40) % 255;
            data[i * 4 + 2] = (filepaths.size() * 60) % 255;
            data[i * 4 + 3] = 255;
        }

        textureData.push_back(data);
        m_textures.push_back(std::move(texture));
    }

    // Создаем массив текстур
    nvrhi::TextureDesc arrayDesc;
    arrayDesc.width = textureWidth;
    arrayDesc.height = textureHeight;
    arrayDesc.arraySize = filepaths.size();
    arrayDesc.format = nvrhi::Format::RGBA8_UNORM;
    arrayDesc.mipLevels = 1;
    arrayDesc.dimension = nvrhi::TextureDimension::Texture2DArray;
    arrayDesc.initialState = nvrhi::ResourceStates::ShaderResource;
    arrayDesc.keepInitialState = true;

    auto textureArray = m_device->createTexture(arrayDesc);

    // Загружаем данные в массив
    for (uint32_t i = 0; i < textureData.size(); ++i) {
        m_commandList->writeTexture(textureArray, i, 0, textureData[i].data(), textureWidth * 4);
    }

    return textureArray;
}
