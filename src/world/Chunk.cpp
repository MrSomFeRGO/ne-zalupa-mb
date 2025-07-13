//
// Created by mrsomfergo on 13.07.2025.
//

#include "Chunk.h"
#include "../rendering/OpenGLUtils.h"
#include <algorithm>
#include <unordered_map>

Chunk::Chunk(const glm::ivec3& position)
    : m_position(position)
    , m_worldPosition(position.x * SIZE, position.y * HEIGHT, position.z * SIZE) {

    // Initialize palette with Air
    m_palette.push_back(BlockType::Air);

    // Initialize all blocks as Air (index 0)
    std::fill(m_blocks.begin(), m_blocks.end(), 0);

    // DON'T create OpenGL objects here - this runs in background thread!
    // OpenGL objects will be created later in main thread
}

Chunk::~Chunk() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
}

BlockType Chunk::GetBlock(int x, int y, int z) const {
    if (!IsValidPosition(x, y, z)) {
        return BlockType::Air;
    }

    uint8_t paletteIndex = m_blocks[GetBlockIndex(x, y, z)];
    if (paletteIndex >= m_palette.size()) {
        return BlockType::Air;
    }

    return m_palette[paletteIndex];
}

void Chunk::SetBlock(int x, int y, int z, BlockType type) {
    if (!IsValidPosition(x, y, z)) {
        return;
    }

    uint8_t paletteIndex = GetPaletteIndex(type);
    m_blocks[GetBlockIndex(x, y, z)] = paletteIndex;
    m_meshDirty = true;

    // Mark neighbor chunks dirty if block is on boundary
    if (x == 0 && m_neighbors[0]) m_neighbors[0]->MarkDirty();
    if (x == SIZE - 1 && m_neighbors[1]) m_neighbors[1]->MarkDirty();
    if (y == 0 && m_neighbors[2]) m_neighbors[2]->MarkDirty();
    if (y == HEIGHT - 1 && m_neighbors[3]) m_neighbors[3]->MarkDirty();
    if (z == 0 && m_neighbors[4]) m_neighbors[4]->MarkDirty();
    if (z == SIZE - 1 && m_neighbors[5]) m_neighbors[5]->MarkDirty();
}

uint8_t Chunk::GetPaletteIndex(BlockType type) {
    // Find existing palette entry
    for (size_t i = 0; i < m_palette.size(); ++i) {
        if (m_palette[i] == type) {
            return static_cast<uint8_t>(i);
        }
    }

    // Add new entry to palette
    if (m_palette.size() < 255) { // Reserve 255 for special cases
        m_palette.push_back(type);
        return static_cast<uint8_t>(m_palette.size() - 1);
    }

    // Palette full, return Air index (should rarely happen)
    return 0;
}

void Chunk::OptimizePalette() {
    // Count usage of each palette entry
    std::vector<uint32_t> usage(m_palette.size(), 0);

    for (uint8_t blockIndex : m_blocks) {
        if (blockIndex < usage.size()) {
            usage[blockIndex]++;
        }
    }

    // Build new palette without unused entries
    std::vector<BlockType> newPalette;
    std::vector<uint8_t> remapping(m_palette.size());

    for (size_t i = 0; i < m_palette.size(); ++i) {
        if (usage[i] > 0) {
            remapping[i] = static_cast<uint8_t>(newPalette.size());
            newPalette.push_back(m_palette[i]);
        } else {
            remapping[i] = 0; // Map to Air
        }
    }

    // Remap block indices
    for (uint8_t& blockIndex : m_blocks) {
        if (blockIndex < remapping.size()) {
            blockIndex = remapping[blockIndex];
        } else {
            blockIndex = 0; // Safety fallback to Air
        }
    }

    m_palette = std::move(newPalette);
}

void Chunk::CreateOpenGLObjects() {
    if (m_vao == 0) {
        m_vao = CreateVertexArray();
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);
        CheckGLError("Chunk OpenGL objects creation");
    }
}

void Chunk::GenerateMesh() {
    if (!m_meshDirty) {
        return;
    }

    // Create OpenGL objects if not created yet (main thread only!)
    CreateOpenGLObjects();

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    m_isEmpty = true;

    // Generate geometry for each block
    for (int y = 0; y < HEIGHT; ++y) {
        for (int z = 0; z < SIZE; ++z) {
            for (int x = 0; x < SIZE; ++x) {
                BlockType type = GetBlock(x, y, z);
                if (type != BlockType::Air) {
                    m_isEmpty = false;
                    AddBlockFaces(x, y, z, type, vertices, indices);
                }
            }
        }
    }

    m_indexCount = indices.size();

    if (m_indexCount > 0) {
        UpdateOpenGLBuffers(vertices, indices);
    }

    m_meshDirty = false;

    // Optimize palette after major changes
    if (m_palette.size() > 16) {
        OptimizePalette();
    }
}

void Chunk::UpdateOpenGLBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
    glBindVertexArray(m_vao);

    // Update vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Update index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    // Set up vertex attributes
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // Texture coordinates
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    // Texture index
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, textureIndex));

    glBindVertexArray(0);

    CheckGLError("Chunk mesh update");
}

void Chunk::SetNeighbor(int direction, Chunk* neighbor) {
    if (direction >= 0 && direction < 6) {
        m_neighbors[direction] = neighbor;
    }
}

bool Chunk::IsValidPosition(int x, int y, int z) const {
    return x >= 0 && x < SIZE && y >= 0 && y < HEIGHT && z >= 0 && z < SIZE;
}

int Chunk::GetBlockIndex(int x, int y, int z) const {
    return y * SIZE * SIZE + z * SIZE + x;
}

bool Chunk::ShouldRenderFace(int x, int y, int z, int nx, int ny, int nz) const {
    // Check neighbor block in same chunk
    if (IsValidPosition(nx, ny, nz)) {
        BlockType neighbor = GetBlock(nx, ny, nz);
        return Block::IsTransparent(neighbor);
    }

    // Check neighbor chunk
    int dx = 0, dy = 0, dz = 0;
    if (nx < 0) { dx = -1; nx = SIZE - 1; }
    else if (nx >= SIZE) { dx = 1; nx = 0; }
    if (ny < 0) { dy = -1; ny = HEIGHT - 1; }
    else if (ny >= HEIGHT) { dy = 1; ny = 0; }
    if (nz < 0) { dz = -1; nz = SIZE - 1; }
    else if (nz >= SIZE) { dz = 1; nz = 0; }

    // Determine neighbor direction
    int neighborIndex = -1;
    if (dx == -1) neighborIndex = 0;      // -X
    else if (dx == 1) neighborIndex = 1;  // +X
    else if (dy == -1) neighborIndex = 2; // -Y
    else if (dy == 1) neighborIndex = 3;  // +Y
    else if (dz == -1) neighborIndex = 4; // -Z
    else if (dz == 1) neighborIndex = 5;  // +Z

    if (neighborIndex >= 0 && m_neighbors[neighborIndex]) {
        BlockType neighbor = m_neighbors[neighborIndex]->GetBlock(nx, ny, nz);
        return Block::IsTransparent(neighbor);
    }

    // No neighbor chunk = render face (chunk boundary)
    return true;
}

void Chunk::AddBlockFaces(int x, int y, int z, BlockType type,
                         std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    const BlockInfo& info = Block::GetBlockInfo(type);
    glm::vec3 blockPos = m_worldPosition + glm::vec3(x, y, z);

    // Face data: normal, vertices, UVs, texture type (0=top, 1=side, 2=bottom)
    struct Face {
        glm::vec3 normal;
        glm::vec3 vertices[4];
        glm::vec2 uvs[4];
        int textureType;
    };

    Face faces[6] = {
        // Front (+Z)
        { glm::vec3(0, 0, 1),
          { glm::vec3(0, 0, 1), glm::vec3(1, 0, 1), glm::vec3(1, 1, 1), glm::vec3(0, 1, 1) },
          { glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0) }, 1 },

        // Back (-Z)
        { glm::vec3(0, 0, -1),
          { glm::vec3(1, 0, 0), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(1, 1, 0) },
          { glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0) }, 1 },

        // Right (+X)
        { glm::vec3(1, 0, 0),
          { glm::vec3(1, 0, 1), glm::vec3(1, 0, 0), glm::vec3(1, 1, 0), glm::vec3(1, 1, 1) },
          { glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0) }, 1 },

        // Left (-X)
        { glm::vec3(-1, 0, 0),
          { glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 1), glm::vec3(0, 1, 0) },
          { glm::vec2(0, 1), glm::vec2(1, 1), glm::vec2(1, 0), glm::vec2(0, 0) }, 1 },

        // Top (+Y)
        { glm::vec3(0, 1, 0),
          { glm::vec3(0, 1, 1), glm::vec3(1, 1, 1), glm::vec3(1, 1, 0), glm::vec3(0, 1, 0) },
          { glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1) }, 0 },

        // Bottom (-Y)
        { glm::vec3(0, -1, 0),
          { glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), glm::vec3(1, 0, 1), glm::vec3(0, 0, 1) },
          { glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1), glm::vec2(0, 1) }, 2 }
    };

    // Neighbor check directions
    glm::ivec3 directions[6] = {
        glm::ivec3(0, 0, 1),   // +Z
        glm::ivec3(0, 0, -1),  // -Z
        glm::ivec3(1, 0, 0),   // +X
        glm::ivec3(-1, 0, 0),  // -X
        glm::ivec3(0, 1, 0),   // +Y
        glm::ivec3(0, -1, 0)   // -Y
    };

    // Generate faces
    for (int i = 0; i < 6; ++i) {
        glm::ivec3 neighborPos = glm::ivec3(x, y, z) + directions[i];

        if (ShouldRenderFace(x, y, z, neighborPos.x, neighborPos.y, neighborPos.z)) {
            uint32_t baseIndex = vertices.size();
            const Face& face = faces[i];

            // Calculate texture index
            uint32_t textureIndex = static_cast<uint32_t>(type) * 3;
            textureIndex += face.textureType; // 0=top, 1=side, 2=bottom

            // Add vertices
            for (int v = 0; v < 4; ++v) {
                Vertex vertex;
                vertex.position = blockPos + face.vertices[v];
                vertex.normal = face.normal;
                vertex.texCoord = face.uvs[v];
                vertex.textureIndex = textureIndex;
                vertices.push_back(vertex);
            }

            // Add indices (two triangles per face)
            indices.push_back(baseIndex + 0);
            indices.push_back(baseIndex + 1);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 0);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 3);
        }
    }
}

size_t Chunk::GetMemoryUsage() const {
    size_t paletteSize = m_palette.size() * sizeof(BlockType);
    size_t blocksSize = m_blocks.size() * sizeof(uint8_t);
    return paletteSize + blocksSize;
}