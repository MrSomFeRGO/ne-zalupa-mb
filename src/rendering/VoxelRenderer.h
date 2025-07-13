//
// Created by mrsomfergo on 13.07.2025.
//

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>

#include "../core/Camera.h"
#include "../world/ChunkManager.h"
#include "Shader.h"
#include "Texture.h"

struct UniformBufferData {
    glm::mat4 viewMatrix;
    glm::mat4 projMatrix;
    glm::vec4 lightDirection;
    glm::vec4 viewPosition;
    glm::vec4 fogColor;
    float fogStart;
    float fogEnd;
    float padding[2];
};

class VoxelRenderer {
public:
    VoxelRenderer();
    ~VoxelRenderer();

    void Initialize(uint32_t width, uint32_t height, ChunkManager* chunkManager);
    void UpdateCamera(Camera* camera);
    void Render();
    void OnResize(uint32_t width, uint32_t height);

    // Statistics
    uint32_t GetRenderedChunks() const { return m_renderedChunks; }
    uint32_t GetRenderedTriangles() const { return m_renderedTriangles; }

private:
    void CreateShaders();
    void CreateTextures();
    void CreateUniformBuffer();
    void UpdateUniformBuffer();
    void SetupBlocks();

    // Rendering
    void RenderChunks(const std::vector<Chunk*>& chunks);

    // Frustum culling
    bool IsChunkInFrustum(const Chunk* chunk) const;
    void UpdateFrustumPlanes();

    // Shaders
    std::unique_ptr<Shader> m_shader;

    // Textures
    std::unique_ptr<TextureManager> m_textureManager;
    GLuint m_textureArray = 0;

    // Uniform buffer
    GLuint m_uniformBuffer = 0;
    UniformBufferData m_uniformData;

    // References
    Camera* m_camera = nullptr;
    ChunkManager* m_chunkManager = nullptr;

    // Viewport
    uint32_t m_width = 1280;
    uint32_t m_height = 720;

    // Frustum culling
    glm::vec4 m_frustumPlanes[6];

    // Statistics
    uint32_t m_renderedChunks = 0;
    uint32_t m_renderedTriangles = 0;

    // Rendering settings
    bool m_wireframe = false;
    bool m_frustumCulling = true;
    float m_renderDistance = 128.0f;
};