//
// Created by mrsomfergo on 12.07.2025.
//

#pragma once

#include <nvrhi/nvrhi.h>
#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "../core/Camera.h"
#include "../world/ChunkManager.h"
#include "Texture.h"
#include "Shader.h"

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
    VoxelRenderer(nvrhi::IDevice* device, nvrhi::ICommandList* commandList);
    ~VoxelRenderer();

    void Initialize(uint32_t width, uint32_t height, ChunkManager* chunkManager);
    void UpdateCamera(Camera* camera);
    void Render(nvrhi::IFramebuffer* framebuffer);
    void OnResize(uint32_t width, uint32_t height);

private:
    void CreatePipeline();
    void CreatePipelineDeferred(nvrhi::IFramebuffer* framebuffer);
    void CreateTextures();
    void CreateUniformBuffer();
    void UpdateUniformBuffer();

    nvrhi::DeviceHandle m_device;
    nvrhi::CommandListHandle m_commandList;

    // Rendering pipeline
    std::unique_ptr<Shader> m_vertexShader;
    std::unique_ptr<Shader> m_pixelShader;
    nvrhi::InputLayoutHandle m_inputLayout;
    nvrhi::GraphicsPipelineHandle m_pipeline;
    nvrhi::BindingLayoutHandle m_bindingLayout;
    nvrhi::BindingSetHandle m_bindingSet;

    // Textures
    std::unique_ptr<TextureManager> m_textureManager;
    nvrhi::TextureHandle m_textureArray;
    nvrhi::SamplerHandle m_sampler;

    // Uniform buffer
    nvrhi::BufferHandle m_uniformBuffer;
    UniformBufferData m_uniformData;

    // References
    Camera* m_camera = nullptr;
    ChunkManager* m_chunkManager = nullptr;

    // Viewport
    uint32_t m_width;
    uint32_t m_height;
};