//
// Created by mrsomfergo on 12.07.2025.
//

#include "VoxelRenderer.h"
#include <glm/gtc/matrix_transform.hpp>

VoxelRenderer::VoxelRenderer(nvrhi::IDevice* device, nvrhi::ICommandList* commandList)
    : m_device(device)
    , m_commandList(commandList)
    , m_width(1280)
    , m_height(720) {
}

VoxelRenderer::~VoxelRenderer() {
}

void VoxelRenderer::Initialize(uint32_t width, uint32_t height, ChunkManager* chunkManager) {
    m_width = width;
    m_height = height;
    m_chunkManager = chunkManager;

    m_textureManager = std::make_unique<TextureManager>(m_device.Get(), m_commandList.Get());

    CreateTextures();
    CreateUniformBuffer();
    CreatePipeline();
}

void VoxelRenderer::CreateTextures() {
    // Загружаем текстуры блоков
    std::vector<std::string> textureFiles = {
        // Stone (3 текстуры: top, side, bottom)
        "assets/textures/stone.png",
        "assets/textures/stone.png",
        "assets/textures/stone.png",

        // Dirt
        "assets/textures/dirt.png",
        "assets/textures/dirt.png",
        "assets/textures/dirt.png",

        // Grass
        "assets/textures/grass_top.png",
        "assets/textures/grass_side.png",
        "assets/textures/grass_side.png",

        // Sand
        "assets/textures/sand.png",
        "assets/textures/sand.png",
        "assets/textures/sand.png",

        // Wood
        "assets/textures/wood_top.png",
        "assets/textures/wood_side.png",
        "assets/textures/wood_top.png",

        // Leaves
        "assets/textures/leaves.png",
        "assets/textures/leaves.png",
        "assets/textures/leaves.png",

        // Water
        "assets/textures/water.png",
        "assets/textures/water.png",
        "assets/textures/water.png",
    };

    // Создаем массив текстур
    m_textureArray = m_textureManager->CreateTextureArray(textureFiles, 16, 16);

    // Создаем сэмплер
    nvrhi::SamplerDesc samplerDesc;
    // samplerDesc.minFilter = nvrhi::SamplerFilter::Nearest;
    // samplerDesc.magFilter = nvrhi::SamplerFilter::Nearest;
    // samplerDesc.mipFilter = nvrhi::SamplerFilter::Nearest;
    samplerDesc.addressU = nvrhi::SamplerAddressMode::Repeat;
    samplerDesc.addressV = nvrhi::SamplerAddressMode::Repeat;
    samplerDesc.addressW = nvrhi::SamplerAddressMode::Repeat;
    m_sampler = m_device->createSampler(samplerDesc);
}

void VoxelRenderer::CreateUniformBuffer() {
    nvrhi::BufferDesc bufferDesc;
    bufferDesc.byteSize = sizeof(UniformBufferData);
    bufferDesc.isConstantBuffer = true;
    bufferDesc.debugName = "VoxelUniformBuffer";
    m_uniformBuffer = m_device->createBuffer(bufferDesc);

    // Инициализация данных
    m_uniformData.lightDirection = glm::normalize(glm::vec4(0.3f, -1.0f, 0.5f, 0.0f));
    m_uniformData.fogColor = glm::vec4(0.529f, 0.808f, 0.922f, 1.0f); // Sky blue
    m_uniformData.fogStart = 50.0f;
    m_uniformData.fogEnd = 150.0f;
}

void VoxelRenderer::CreatePipeline() {
    // Создаем шейдеры
    m_vertexShader = std::make_unique<Shader>(m_device.Get(), "shaders/voxel.vert.spv", nvrhi::ShaderType::Vertex);
    m_pixelShader = std::make_unique<Shader>(m_device.Get(), "shaders/voxel.frag.spv", nvrhi::ShaderType::Pixel);

    // Создаем layout привязок
    nvrhi::BindingLayoutDesc layoutDesc;
    layoutDesc.visibility = nvrhi::ShaderType::All;
    layoutDesc.bindings = {
        nvrhi::BindingLayoutItem::ConstantBuffer(0),
        nvrhi::BindingLayoutItem::Texture_SRV(0),
        nvrhi::BindingLayoutItem::Sampler(0)
    };
    m_bindingLayout = m_device->createBindingLayout(layoutDesc);

    // Создаем набор привязок
    nvrhi::BindingSetDesc bindingSetDesc;
    bindingSetDesc.bindings = {
        nvrhi::BindingSetItem::ConstantBuffer(0, m_uniformBuffer),
        nvrhi::BindingSetItem::Texture_SRV(0, m_textureArray),
        nvrhi::BindingSetItem::Sampler(0, m_sampler)
    };
    m_bindingSet = m_device->createBindingSet(bindingSetDesc, m_bindingLayout);

    // Описание вершинного формата
    nvrhi::VertexAttributeDesc vertexAttributes[] = {
        nvrhi::VertexAttributeDesc()
            .setName("POSITION")
            .setFormat(nvrhi::Format::RGB32_FLOAT)
            .setOffset(offsetof(Chunk::Vertex, position))
            .setElementStride(sizeof(Chunk::Vertex)),
        nvrhi::VertexAttributeDesc()
            .setName("NORMAL")
            .setFormat(nvrhi::Format::RGB32_FLOAT)
            .setOffset(offsetof(Chunk::Vertex, normal))
            .setElementStride(sizeof(Chunk::Vertex)),
        nvrhi::VertexAttributeDesc()
            .setName("TEXCOORD")
            .setFormat(nvrhi::Format::RG32_FLOAT)
            .setOffset(offsetof(Chunk::Vertex, texCoord))
            .setElementStride(sizeof(Chunk::Vertex)),
        nvrhi::VertexAttributeDesc()
            .setName("TEXINDEX")
            .setFormat(nvrhi::Format::R32_UINT)
            .setOffset(offsetof(Chunk::Vertex, textureIndex))
            .setElementStride(sizeof(Chunk::Vertex))
    };
    nvrhi::InputLayoutHandle inputLayout = m_device->createInputLayout(
        vertexAttributes, uint32_t(std::size(vertexAttributes)), m_vertexShader->GetHandle());
    // Создаем графический пайплайн
    nvrhi::GraphicsPipelineDesc pipelineDesc;
    pipelineDesc.VS = m_vertexShader->GetHandle();
    pipelineDesc.PS = m_pixelShader->GetHandle();
    pipelineDesc.inputLayout = inputLayout;
    pipelineDesc.bindingLayouts = { m_bindingLayout };
    pipelineDesc.primType = nvrhi::PrimitiveType::TriangleList;

    // Настройки рендеринга
    pipelineDesc.renderState.depthStencilState.depthTestEnable = true;
    pipelineDesc.renderState.depthStencilState.depthWriteEnable = true;
    pipelineDesc.renderState.depthStencilState.depthFunc = nvrhi::ComparisonFunc::Less;

    pipelineDesc.renderState.rasterState.cullMode = nvrhi::RasterCullMode::Back;
    pipelineDesc.renderState.rasterState.frontCounterClockwise = false;

    // Включаем альфа-смешивание для прозрачных блоков
    pipelineDesc.renderState.blendState.targets[0].blendEnable = true;
    pipelineDesc.renderState.blendState.targets[0].srcBlend = nvrhi::BlendFactor::SrcAlpha;
    pipelineDesc.renderState.blendState.targets[0].destBlend = nvrhi::BlendFactor::InvSrcAlpha;
    auto framebufferDesc = nvrhi::FramebufferDesc();
    nvrhi::FramebufferHandle framebuffer = m_device->createFramebuffer(framebufferDesc);
    m_pipeline = m_device->createGraphicsPipeline(pipelineDesc, framebuffer);
}

void VoxelRenderer::UpdateCamera(Camera* camera) {
    m_camera = camera;
}

void VoxelRenderer::UpdateUniformBuffer() {
    if (!m_camera) return;

    m_uniformData.viewMatrix = m_camera->GetViewMatrix();
    m_uniformData.projMatrix = m_camera->GetProjectionMatrix();
    m_uniformData.viewPosition = glm::vec4(m_camera->GetPosition(), 1.0f);

    m_commandList->writeBuffer(m_uniformBuffer, &m_uniformData, sizeof(m_uniformData));
}

void VoxelRenderer::Render(nvrhi::IFramebuffer* framebuffer) {
    if (!m_camera || !m_chunkManager) return;

    // Обновляем uniform buffer
    UpdateUniformBuffer();

    // Генерируем меши для чанков
    m_chunkManager->GenerateChunkMeshes(m_device.Get(), m_commandList.Get());

    // Получаем видимые чанки
    std::vector<Chunk*> visibleChunks = m_chunkManager->GetVisibleChunks();

    // Настройка состояния рендеринга
    nvrhi::GraphicsState state;
    state.pipeline = m_pipeline;
    state.framebuffer = framebuffer;
    state.bindings = { m_bindingSet };
    state.viewport.viewports = { nvrhi::Viewport(float(m_width), float(m_height)) };
    state.viewport.scissorRects = { nvrhi::Rect(m_width, m_height) };

    // Рендерим каждый чанк
    for (Chunk* chunk : visibleChunks) {
        if (chunk->GetIndexCount() == 0) continue;

        state.vertexBuffers = { { chunk->GetVertexBuffer(), 0, 0 } };
        state.indexBuffer = { chunk->GetIndexBuffer(), nvrhi::Format::R32_UINT, 0 };

        m_commandList->setGraphicsState(state);

        nvrhi::DrawArguments drawArgs;
        drawArgs.instanceCount = 1;
        drawArgs.vertexCount = chunk->GetIndexCount();
        m_commandList->drawIndexed(drawArgs);
    }
}

void VoxelRenderer::OnResize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;

    if (m_camera) {
        m_camera->SetAspectRatio((float)width / height);
    }
}