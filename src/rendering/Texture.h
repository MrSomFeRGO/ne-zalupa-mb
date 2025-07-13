//
// Created by mrsomfergo on 13.07.2025.
//

#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>
#include <memory>

class Texture {
public:
    Texture();
    ~Texture();

    // Load texture from file
    bool LoadFromFile(const std::string& filepath, int targetWidth = 0, int targetHeight = 0);

    // Load texture from memory
    bool LoadFromMemory(const uint8_t* data, uint32_t width, uint32_t height, uint32_t channels);
    static std::vector<uint8_t> GenerateDefaultTexture(uint32_t width, uint32_t height);

    // Create empty texture
    bool Create(uint32_t width, uint32_t height, GLenum internalFormat = GL_RGBA8,
                GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE);

    // Getters
    GLuint GetID() const { return m_textureID; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    uint32_t GetChannels() const { return m_channels; }

    // Bind texture
    void Bind(uint32_t slot = 0) const;
    void Unbind() const;

private:
    void CreateFromData(const uint8_t* data);
    std::vector<uint8_t> ResizeImage(const uint8_t* data, int oldWidth, int oldHeight,
                                    int newWidth, int newHeight, int channels);

    GLuint m_textureID = 0;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_channels = 4;
};

class TextureManager {
public:
    TextureManager();
    ~TextureManager();

    // Create texture array from multiple files
    GLuint CreateTextureArray(const std::vector<std::string>& filepaths,
                             uint32_t textureWidth, uint32_t textureHeight);

    // Load single texture
    std::unique_ptr<Texture> LoadTexture(const std::string& filepath,
                                        int targetWidth = 0, int targetHeight = 0);

private:
    std::vector<std::unique_ptr<Texture>> m_textures;
};