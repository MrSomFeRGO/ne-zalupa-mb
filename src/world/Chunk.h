//
// Created by mrsomfergo on 13.07.2025.
//

#pragma once

#include "Block.h"
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <memory>

class Chunk {
public:
    static constexpr int SIZE = 16;
    static constexpr int HEIGHT = 16;
    static constexpr int TOTAL_BLOCKS = SIZE * SIZE * HEIGHT;

    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
        uint32_t textureIndex;
    };

    Chunk(const glm::ivec3& position);
    ~Chunk();

    // Block access using palette
    BlockType GetBlock(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, BlockType type);

    // Mesh generation
    void GenerateMesh();
    void CreateOpenGLObjects(); // Create VAO/VBO/EBO (main thread only!)
    bool NeedsMeshUpdate() const { return m_meshDirty; }
    void MarkDirty() { m_meshDirty = true; }

    // Getters
    const glm::ivec3& GetPosition() const { return m_position; }
    const glm::vec3& GetWorldPosition() const { return m_worldPosition; }
    uint32_t GetVAO() const { return m_vao; }
    uint32_t GetVBO() const { return m_vbo; }
    uint32_t GetEBO() const { return m_ebo; }
    uint32_t GetIndexCount() const { return m_indexCount; }
    bool IsEmpty() const { return m_isEmpty; }

    // Neighbors for mesh optimization
    void SetNeighbor(int direction, Chunk* neighbor);
    Chunk* GetNeighbor(int direction) const { return m_neighbors[direction]; }

    // Palette info for debugging
    size_t GetPaletteSize() const { return m_palette.size(); }
    size_t GetMemoryUsage() const;

private:
    // Coordinate helpers
    bool IsValidPosition(int x, int y, int z) const;
    int GetBlockIndex(int x, int y, int z) const;

    // Palette management
    uint8_t GetPaletteIndex(BlockType type);
    void OptimizePalette();

    // Mesh generation
    void AddBlockFaces(int x, int y, int z, BlockType type,
                      std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    bool ShouldRenderFace(int x, int y, int z, int nx, int ny, int nz) const;
    void UpdateOpenGLBuffers(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    // Position
    glm::ivec3 m_position;
    glm::vec3 m_worldPosition;

    // Palette system for memory efficiency
    std::vector<BlockType> m_palette;           // Unique block types in this chunk
    std::array<uint8_t, TOTAL_BLOCKS> m_blocks; // Indices into palette (1 byte per block)

    // OpenGL buffers
    uint32_t m_vao = 0;
    uint32_t m_vbo = 0;
    uint32_t m_ebo = 0;
    uint32_t m_indexCount = 0;
    bool m_meshDirty = true;
    bool m_isEmpty = true;

    // Neighbors for optimization (6 directions: -X, +X, -Y, +Y, -Z, +Z)
    std::array<Chunk*, 6> m_neighbors = { nullptr };
};