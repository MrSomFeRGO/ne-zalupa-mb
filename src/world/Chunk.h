//
// Created by mrsomfergo on 12.07.2025.
//

#pragma once

#include "Block.h"
#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <memory>
#include <nvrhi/nvrhi.h>

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

    // Доступ к блокам
    BlockType GetBlock(int x, int y, int z) const;
    void SetBlock(int x, int y, int z, BlockType type);

    // Генерация меша
    void GenerateMesh(nvrhi::IDevice* device, nvrhi::ICommandList* commandList);
    bool NeedsMeshUpdate() const { return m_meshDirty; }

    // Getters
    const glm::ivec3& GetPosition() const { return m_position; }
    const glm::vec3& GetWorldPosition() const { return m_worldPosition; }
    nvrhi::IBuffer* GetVertexBuffer() const { return m_vertexBuffer.Get(); }
    nvrhi::IBuffer* GetIndexBuffer() const { return m_indexBuffer.Get(); }
    uint32_t GetIndexCount() const { return m_indexCount; }
    bool IsEmpty() const { return m_isEmpty; }

    // Соседи для оптимизации меша
    void SetNeighbor(int direction, Chunk* neighbor);

private:
    // Проверка границ
    bool IsValidPosition(int x, int y, int z) const;
    int GetBlockIndex(int x, int y, int z) const;

    // Генерация геометрии для блока
    void AddBlockFaces(int x, int y, int z, BlockType type,
                      std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
    bool ShouldRenderFace(int x, int y, int z, int nx, int ny, int nz) const;

    // Позиция чанка в чанковых координатах
    glm::ivec3 m_position;
    glm::vec3 m_worldPosition;

    // Данные блоков
    std::array<BlockType, TOTAL_BLOCKS> m_blocks;

    // Меш
    nvrhi::BufferHandle m_vertexBuffer;
    nvrhi::BufferHandle m_indexBuffer;
    uint32_t m_indexCount = 0;
    bool m_meshDirty = true;
    bool m_isEmpty = true;

    // Соседние чанки для оптимизации
    std::array<Chunk*, 6> m_neighbors = { nullptr };
};