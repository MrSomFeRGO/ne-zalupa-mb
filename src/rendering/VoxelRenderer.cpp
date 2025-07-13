//
// Created by mrsomfergo on 13.07.2025.
//

#include "VoxelRenderer.h"
#include "OpenGLUtils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

VoxelRenderer::VoxelRenderer() {
}

VoxelRenderer::~VoxelRenderer() {
    if (m_uniformBuffer) {
        glDeleteBuffers(1, &m_uniformBuffer);
    }
    if (m_textureArray) {
        glDeleteTextures(1, &m_textureArray);
    }
}

void VoxelRenderer::Initialize(uint32_t width, uint32_t height, ChunkManager* chunkManager) {
    m_width = width;
    m_height = height;
    m_chunkManager = chunkManager;

    CreateShaders();
    CreateTextures();
    CreateUniformBuffer();
    SetupBlocks();

    // Initialize uniform data
    m_uniformData.lightDirection = glm::normalize(glm::vec4(0.3f, -1.0f, 0.5f, 0.0f));
    m_uniformData.fogColor = glm::vec4(0.529f, 0.808f, 0.922f, 1.0f); // Sky blue
    m_uniformData.fogStart = 50.0f;
    m_uniformData.fogEnd = 150.0f;

    std::cout << "VoxelRenderer initialized" << std::endl;
    CheckGLError("VoxelRenderer initialization");
}

void VoxelRenderer::CreateShaders() {
    m_shader = std::make_unique<Shader>();

    // Check OpenGL version to use appropriate shader version
    GLint majorVersion, minorVersion;
    glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
    glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

    std::string shaderVersion;
    if (majorVersion >= 4 && minorVersion >= 5) {
        shaderVersion = "#version 450 core";
    } else if (majorVersion >= 4 && minorVersion >= 3) {
        shaderVersion = "#version 430 core";
    } else if (majorVersion >= 4 && minorVersion >= 1) {
        shaderVersion = "#version 410 core";
    } else {
        shaderVersion = "#version 330 core";
    }

    const std::string vertexSource = shaderVersion + R"(

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in uint aTextureIndex;

layout(std140, binding = 0) uniform UniformBuffer {
    mat4 viewMatrix;
    mat4 projMatrix;
    vec4 lightDirection;
    vec4 viewPosition;
    vec4 fogColor;
    float fogStart;
    float fogEnd;
    float padding[2];
} ubo;

out vec3 fragWorldPos;
out vec3 fragNormal;
out vec2 fragTexCoord;
flat out uint fragTextureIndex;
out float fragFogFactor;

void main() {
    vec4 worldPos = vec4(aPosition, 1.0);
    vec4 viewPos = ubo.viewMatrix * worldPos;
    gl_Position = ubo.projMatrix * viewPos;

    fragWorldPos = aPosition;
    fragNormal = aNormal;
    fragTexCoord = aTexCoord;
    fragTextureIndex = aTextureIndex;

    // Calculate fog factor
    float distance = length(viewPos.xyz);
    fragFogFactor = clamp((ubo.fogEnd - distance) / (ubo.fogEnd - ubo.fogStart), 0.0, 1.0);
}
)";

    const std::string fragmentSource = shaderVersion + R"(

in vec3 fragWorldPos;
in vec3 fragNormal;
in vec2 fragTexCoord;
flat in uint fragTextureIndex;
in float fragFogFactor;

layout(std140, binding = 0) uniform UniformBuffer {
    mat4 viewMatrix;
    mat4 projMatrix;
    vec4 lightDirection;
    vec4 viewPosition;
    vec4 fogColor;
    float fogStart;
    float fogEnd;
    float padding[2];
} ubo;

uniform sampler2DArray texArray;

out vec4 fragColor;

void main() {
    // Sample texture from array
    vec4 texColor = texture(texArray, vec3(fragTexCoord, float(fragTextureIndex)));

    // Discard transparent pixels
    if (texColor.a < 0.5) {
        discard;
    }

    // Calculate lighting
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(-ubo.lightDirection.xyz);

    // Ambient lighting
    float ambient = 0.4;

    // Diffuse lighting
    float diffuse = max(dot(normal, lightDir), 0.0);

    // Add extra brightness to top faces
    if (normal.y > 0.9) {
        diffuse += 0.1;
    }

    // Calculate shadow for bottom faces
    float shadow = 1.0;
    if (normal.y < -0.9) {
        shadow = 0.5;
    }

    // Combine lighting
    vec3 lighting = vec3(ambient + diffuse * 0.6) * shadow;
    vec3 finalColor = texColor.rgb * lighting;

    // Apply fog
    finalColor = mix(ubo.fogColor.rgb, finalColor, fragFogFactor);

    fragColor = vec4(finalColor, texColor.a);
}
)";

    if (!m_shader->LoadFromSource(vertexSource, fragmentSource)) {
        throw std::runtime_error("Failed to create voxel shader");
    }

    std::cout << "Voxel shader compiled with " << shaderVersion << std::endl;
    CheckGLError("Shader creation");
}

void VoxelRenderer::CreateTextures() {
    m_textureManager = std::make_unique<TextureManager>();

    // Load block textures
    std::vector<std::string> textureFiles = {
        // Stone (3 textures: top, side, bottom)
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

    m_textureArray = m_textureManager->CreateTextureArray(textureFiles, 16, 16);

    CheckGLError("Texture creation");
}

void VoxelRenderer::CreateUniformBuffer() {
    glGenBuffers(1, &m_uniformBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformBufferData), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniformBuffer);

    CheckGLError("Uniform buffer creation");
}

void VoxelRenderer::SetupBlocks() {
    // This can be expanded for per-block rendering settings
    std::cout << "Block rendering setup complete" << std::endl;
}

void VoxelRenderer::UpdateCamera(Camera* camera) {
    m_camera = camera;
    if (camera) {
        UpdateFrustumPlanes();
    }
}

void VoxelRenderer::UpdateUniformBuffer() {
    if (!m_camera) return;

    m_uniformData.viewMatrix = m_camera->GetViewMatrix();
    m_uniformData.projMatrix = m_camera->GetProjectionMatrix();
    m_uniformData.viewPosition = glm::vec4(m_camera->GetPosition(), 1.0f);

    glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UniformBufferData), &m_uniformData);
}

void VoxelRenderer::UpdateFrustumPlanes() {
    if (!m_camera) return;

    glm::mat4 viewProjection = m_camera->GetProjectionMatrix() * m_camera->GetViewMatrix();

    // Extract frustum planes from view-projection matrix
    // Left plane
    m_frustumPlanes[0] = glm::vec4(
        viewProjection[0][3] + viewProjection[0][0],
        viewProjection[1][3] + viewProjection[1][0],
        viewProjection[2][3] + viewProjection[2][0],
        viewProjection[3][3] + viewProjection[3][0]
    );

    // Right plane
    m_frustumPlanes[1] = glm::vec4(
        viewProjection[0][3] - viewProjection[0][0],
        viewProjection[1][3] - viewProjection[1][0],
        viewProjection[2][3] - viewProjection[2][0],
        viewProjection[3][3] - viewProjection[3][0]
    );

    // Bottom plane
    m_frustumPlanes[2] = glm::vec4(
        viewProjection[0][3] + viewProjection[0][1],
        viewProjection[1][3] + viewProjection[1][1],
        viewProjection[2][3] + viewProjection[2][1],
        viewProjection[3][3] + viewProjection[3][1]
    );

    // Top plane
    m_frustumPlanes[3] = glm::vec4(
        viewProjection[0][3] - viewProjection[0][1],
        viewProjection[1][3] - viewProjection[1][1],
        viewProjection[2][3] - viewProjection[2][1],
        viewProjection[3][3] - viewProjection[3][1]
    );

    // Near plane
    m_frustumPlanes[4] = glm::vec4(
        viewProjection[0][3] + viewProjection[0][2],
        viewProjection[1][3] + viewProjection[1][2],
        viewProjection[2][3] + viewProjection[2][2],
        viewProjection[3][3] + viewProjection[3][2]
    );

    // Far plane
    m_frustumPlanes[5] = glm::vec4(
        viewProjection[0][3] - viewProjection[0][2],
        viewProjection[1][3] - viewProjection[1][2],
        viewProjection[2][3] - viewProjection[2][2],
        viewProjection[3][3] - viewProjection[3][2]
    );

    // Normalize planes
    for (int i = 0; i < 6; ++i) {
        float length = glm::length(glm::vec3(m_frustumPlanes[i]));
        m_frustumPlanes[i] /= length;
    }
}

bool VoxelRenderer::IsChunkInFrustum(const Chunk* chunk) const {
    if (!m_frustumCulling) return true;

    glm::vec3 chunkMin = chunk->GetWorldPosition();
    glm::vec3 chunkMax = chunkMin + glm::vec3(Chunk::SIZE, Chunk::HEIGHT, Chunk::SIZE);
    glm::vec3 center = (chunkMin + chunkMax) * 0.5f;
    glm::vec3 extent = chunkMax - center;

    // Test chunk AABB against all frustum planes
    for (int i = 0; i < 6; ++i) {
        glm::vec3 planeNormal = glm::vec3(m_frustumPlanes[i]);
        float planeDistance = m_frustumPlanes[i].w;

        // Calculate the distance from center to plane
        float distance = glm::dot(center, planeNormal) + planeDistance;

        // Calculate the effective radius of the AABB in the direction of the plane normal
        float radius = glm::dot(extent, glm::abs(planeNormal));

        // If the chunk is completely outside this plane, it's not visible
        if (distance + radius < 0) {
            return false;
        }
    }

    return true;
}

void VoxelRenderer::Render() {
    if (!m_camera || !m_chunkManager) return;

    // Update uniforms
    UpdateUniformBuffer();
    UpdateFrustumPlanes();

    // Get visible chunks
    std::vector<Chunk*> visibleChunks = m_chunkManager->GetVisibleChunks();

    // Frustum culling
    std::vector<Chunk*> culledChunks;
    culledChunks.reserve(visibleChunks.size());

    for (Chunk* chunk : visibleChunks) {
        if (IsChunkInFrustum(chunk)) {
            culledChunks.push_back(chunk);
        }
    }

    // Render chunks
    RenderChunks(culledChunks);

    // Update statistics
    m_renderedChunks = culledChunks.size();

    CheckGLError("VoxelRenderer::Render");
}

void VoxelRenderer::RenderChunks(const std::vector<Chunk*>& chunks) {
    if (chunks.empty()) return;

    // Use shader
    m_shader->Use();

    // Bind texture array
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_textureArray);
    glUniform1i(glGetUniformLocation(m_shader->GetProgram(), "texArray"), 0);

    // Set wireframe mode if enabled
    if (m_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    m_renderedTriangles = 0;

    // Render each chunk
    for (Chunk* chunk : chunks) {
        if (chunk->GetIndexCount() == 0 || chunk->GetVAO() == 0) {
            continue; // Skip chunks without mesh or OpenGL objects
        }

        // Bind VAO and draw
        glBindVertexArray(chunk->GetVAO());
        glDrawElements(GL_TRIANGLES, chunk->GetIndexCount(), GL_UNSIGNED_INT, 0);

        m_renderedTriangles += chunk->GetIndexCount() / 3;
    }

    // Reset wireframe mode
    if (m_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glBindVertexArray(0);
}

void VoxelRenderer::OnResize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;

    if (m_camera) {
        m_camera->SetAspectRatio((float)width / height);
    }
}