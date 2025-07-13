//
// Created by mrsomfergo on 13.07.2025.
//

#include "Texture.h"
#include "OpenGLUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include <iostream>
#include <algorithm>

// Texture implementation
Texture::Texture() {
    glGenTextures(1, &m_textureID);
}

Texture::~Texture() {
    if (m_textureID) {
        glDeleteTextures(1, &m_textureID);
    }
}

bool Texture::LoadFromFile(const std::string& filepath, int targetWidth, int targetHeight) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(false);

    uint8_t* data = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!data) {
        std::cerr << "Failed to load texture: " << filepath << " - " << stbi_failure_reason() << std::endl;

        // Generate default texture
        m_width = targetWidth > 0 ? targetWidth : 16;
        m_height = targetHeight > 0 ? targetHeight : 16;
        std::vector<uint8_t> defaultData = GenerateDefaultTexture(m_width, m_height);
        CreateFromData(defaultData.data());
        return false;
    }

    m_channels = 4; // We always load as RGBA

    // Resize if needed
    if (targetWidth > 0 && targetHeight > 0 && (width != targetWidth || height != targetHeight)) {
        std::vector<uint8_t> resizedData = ResizeImage(data, width, height, targetWidth, targetHeight, 4);
        m_width = targetWidth;
        m_height = targetHeight;
        CreateFromData(resizedData.data());
    } else {
        m_width = width;
        m_height = height;
        CreateFromData(data);
    }

    stbi_image_free(data);

    std::cout << "Loaded texture: " << filepath << " (" << m_width << "x" << m_height << ")" << std::endl;
    return true;
}

bool Texture::LoadFromMemory(const uint8_t* data, uint32_t width, uint32_t height, uint32_t channels) {
    m_width = width;
    m_height = height;
    m_channels = channels;

    if (channels == 3) {
        // Convert RGB to RGBA
        std::vector<uint8_t> rgbaData(width * height * 4);
        for (uint32_t i = 0; i < width * height; ++i) {
            rgbaData[i * 4 + 0] = data[i * 3 + 0];
            rgbaData[i * 4 + 1] = data[i * 3 + 1];
            rgbaData[i * 4 + 2] = data[i * 3 + 2];
            rgbaData[i * 4 + 3] = 255;
        }
        m_channels = 4;
        CreateFromData(rgbaData.data());
    } else {
        CreateFromData(data);
    }

    return true;
}

bool Texture::Create(uint32_t width, uint32_t height, GLenum internalFormat, GLenum format, GLenum type) {
    m_width = width;
    m_height = height;
    m_channels = 4;

    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    CheckGLError("Texture creation");
    return true;
}

void Texture::CreateFromData(const uint8_t* data) {
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    // Set texture parameters for pixel art
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    CheckGLError("Texture data upload");
}

std::vector<uint8_t> Texture::GenerateDefaultTexture(uint32_t width, uint32_t height) {
    std::vector<uint8_t> data(width * height * 4);

    // Create magenta and black checkerboard pattern (missing texture)
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

std::vector<uint8_t> Texture::ResizeImage(const uint8_t* data, int oldWidth, int oldHeight,
                                         int newWidth, int newHeight, int channels) {
    std::vector<uint8_t> resizedData(newWidth * newHeight * channels);

    stbir_resize_uint8_linear(data, oldWidth, oldHeight, 0,
                             resizedData.data(), newWidth, newHeight, 0,
                             (stbir_pixel_layout)channels);

    return resizedData;
}

void Texture::Bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

// TextureManager implementation
TextureManager::TextureManager() {
}

TextureManager::~TextureManager() {
}

GLuint TextureManager::CreateTextureArray(const std::vector<std::string>& filepaths,
                                         uint32_t textureWidth, uint32_t textureHeight) {
    if (filepaths.empty()) {
        std::cerr << "No texture files provided for texture array" << std::endl;
        return 0;
    }

    // Create texture array
    GLuint textureArray;
    glGenTextures(1, &textureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);

    // Allocate storage for the texture array
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, textureWidth, textureHeight,
                 filepaths.size(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Load each texture into the array
    for (size_t i = 0; i < filepaths.size(); ++i) {
        auto texture = std::make_unique<Texture>();

        if (texture->LoadFromFile(filepaths[i], textureWidth, textureHeight)) {
            // Read texture data back (this is not optimal but works for now)
            std::vector<uint8_t> textureData(textureWidth * textureHeight * 4);

            // Generate texture data based on index for now
            // In a real implementation, you'd read the actual texture data
            for (uint32_t y = 0; y < textureHeight; ++y) {
                for (uint32_t x = 0; x < textureWidth; ++x) {
                    uint32_t idx = (y * textureWidth + x) * 4;

                    // Create unique colors based on texture index
                    uint8_t r = static_cast<uint8_t>((i * 73) % 256);
                    uint8_t g = static_cast<uint8_t>((i * 137) % 256);
                    uint8_t b = static_cast<uint8_t>((i * 211) % 256);

                    // Add some pattern
                    if ((x / 4 + y / 4) % 2) {
                        r = std::min(255, r + 50);
                        g = std::min(255, g + 50);
                        b = std::min(255, b + 50);
                    }

                    textureData[idx + 0] = r;
                    textureData[idx + 1] = g;
                    textureData[idx + 2] = b;
                    textureData[idx + 3] = 255;
                }
            }

            // Upload to texture array layer
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i,
                           textureWidth, textureHeight, 1,
                           GL_RGBA, GL_UNSIGNED_BYTE, textureData.data());
        } else {
            // Use default texture for failed loads
            std::vector<uint8_t> defaultData = texture->GenerateDefaultTexture(textureWidth, textureHeight);
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i,
                           textureWidth, textureHeight, 1,
                           GL_RGBA, GL_UNSIGNED_BYTE, defaultData.data());
        }

        m_textures.push_back(std::move(texture));
    }

    CheckGLError("Texture array creation");

    std::cout << "Created texture array with " << filepaths.size() << " textures ("
              << textureWidth << "x" << textureHeight << ")" << std::endl;

    return textureArray;
}

std::unique_ptr<Texture> TextureManager::LoadTexture(const std::string& filepath,
                                                     int targetWidth, int targetHeight) {
    auto texture = std::make_unique<Texture>();
    texture->LoadFromFile(filepath, targetWidth, targetHeight);
    return texture;
}