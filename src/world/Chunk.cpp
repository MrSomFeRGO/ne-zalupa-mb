//
// Created by mrsomfergo on 12.07.2025.
//

#include "Chunk.h"
#include <algorithm>

Chunk::Chunk(const glm::ivec3& position)
    : m_position(position)
    , m_worldPosition(position.x * SIZE, position.y * HEIGHT, position.z * SIZE) {
    // Инициализируем все блоки как воздух
    std::fill(m_blocks.begin(), m_blocks.end(), BlockType::Air);
}

Chunk::~Chunk() {
}

BlockType Chunk::GetBlock(int x, int y, int z) const {
    if (!IsValidPosition(x, y, z)) {
        return BlockType::Air;
    }
    return m_blocks[GetBlockIndex(x, y, z)];
}

void Chunk::SetBlock(int x, int y, int z, BlockType type) {
    if (!IsValidPosition(x, y, z)) {
        return;
    }

    m_blocks[GetBlockIndex(x, y, z)] = type;
    m_meshDirty = true;

    // Если блок на границе чанка, помечаем соседний чанк для обновления
    if (x == 0 && m_neighbors[0]) m_neighbors[0]->m_meshDirty = true;
    if (x == SIZE - 1 && m_neighbors[1]) m_neighbors[1]->m_meshDirty = true;
    if (y == 0 && m_neighbors[2]) m_neighbors[2]->m_meshDirty = true;
    if (y == HEIGHT - 1 && m_neighbors[3]) m_neighbors[3]->m_meshDirty = true;
    if (z == 0 && m_neighbors[4]) m_neighbors[4]->m_meshDirty = true;
    if (z == SIZE - 1 && m_neighbors[5]) m_neighbors[5]->m_meshDirty = true;
}

void Chunk::GenerateMesh(nvrhi::IDevice* device, nvrhi::ICommandList* commandList) {
    if (!m_meshDirty) {
        return;
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    m_isEmpty = true;

    // Генерируем геометрию для каждого блока
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
        // Создаем буферы
        nvrhi::BufferDesc vertexBufferDesc;
        vertexBufferDesc.byteSize = vertices.size() * sizeof(Vertex);
        vertexBufferDesc.isVertexBuffer = true;
        vertexBufferDesc.debugName = "ChunkVertexBuffer";
        m_vertexBuffer = device->createBuffer(vertexBufferDesc);

        nvrhi::BufferDesc indexBufferDesc;
        indexBufferDesc.byteSize = indices.size() * sizeof(uint32_t);
        indexBufferDesc.isIndexBuffer = true;
        indexBufferDesc.debugName = "ChunkIndexBuffer";
        m_indexBuffer = device->createBuffer(indexBufferDesc);

        // Загружаем данные
        commandList->writeBuffer(m_vertexBuffer, vertices.data(), vertices.size() * sizeof(Vertex));
        commandList->writeBuffer(m_indexBuffer, indices.data(), indices.size() * sizeof(uint32_t));
    } else {
        m_vertexBuffer = nullptr;
        m_indexBuffer = nullptr;
    }

    m_meshDirty = false;
}

void Chunk::SetNeighbor(int direction, Chunk* neighbor) {
    m_neighbors[direction] = neighbor;
}

bool Chunk::IsValidPosition(int x, int y, int z) const {
    return x >= 0 && x < SIZE && y >= 0 && y < HEIGHT && z >= 0 && z < SIZE;
}

int Chunk::GetBlockIndex(int x, int y, int z) const {
    return y * SIZE * SIZE + z * SIZE + x;
}

bool Chunk::ShouldRenderFace(int x, int y, int z, int nx, int ny, int nz) const {
    // Проверяем блок в соседней позиции
    if (IsValidPosition(nx, ny, nz)) {
        BlockType neighbor = GetBlock(nx, ny, nz);
        return Block::IsTransparent(neighbor);
    }

    // Проверяем соседний чанк
    int dx = 0, dy = 0, dz = 0;
    if (nx < 0) { dx = -1; nx = SIZE - 1; }
    else if (nx >= SIZE) { dx = 1; nx = 0; }
    if (ny < 0) { dy = -1; ny = HEIGHT - 1; }
    else if (ny >= HEIGHT) { dy = 1; ny = 0; }
    if (nz < 0) { dz = -1; nz = SIZE - 1; }
    else if (nz >= SIZE) { dz = 1; nz = 0; }

    // Определяем индекс соседа
    int neighborIndex = -1;
    if (dx == -1) neighborIndex = 0;
    else if (dx == 1) neighborIndex = 1;
    else if (dy == -1) neighborIndex = 2;
    else if (dy == 1) neighborIndex = 3;
    else if (dz == -1) neighborIndex = 4;
    else if (dz == 1) neighborIndex = 5;

    if (neighborIndex >= 0 && m_neighbors[neighborIndex]) {
        BlockType neighbor = m_neighbors[neighborIndex]->GetBlock(nx, ny, nz);
        return Block::IsTransparent(neighbor);
    }

    // Если нет соседнего чанка, рендерим грань
    return true;
}

void Chunk::AddBlockFaces(int x, int y, int z, BlockType type,
                         std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    const BlockInfo& info = Block::GetBlockInfo(type);
    glm::vec3 blockPos = m_worldPosition + glm::vec3(x, y, z);

    // Структура для описания граней куба
    struct Face {
        glm::vec3 normal;
        glm::vec3 vertices[4];
        glm::vec2 uvs[4];
        int textureType; // 0 - top, 1 - side, 2 - bottom
    };

    Face faces[6] = {
        // Передняя грань (+Z)
        {
            glm::vec3(0, 0, 1),
            {
                glm::vec3(0, 0, 1), glm::vec3(1, 0, 1),
                glm::vec3(1, 1, 1), glm::vec3(0, 1, 1)
            },
            {
                glm::vec2(0, 1), glm::vec2(1, 1),
                glm::vec2(1, 0), glm::vec2(0, 0)
            },
            1 // side
        },
        // Задняя грань (-Z)
        {
            glm::vec3(0, 0, -1),
            {
                glm::vec3(1, 0, 0), glm::vec3(0, 0, 0),
                glm::vec3(0, 1, 0), glm::vec3(1, 1, 0)
            },
            {
                glm::vec2(0, 1), glm::vec2(1, 1),
                glm::vec2(1, 0), glm::vec2(0, 0)
            },
            1 // side
        },
        // Правая грань (+X)
        {
            glm::vec3(1, 0, 0),
            {
                glm::vec3(1, 0, 1), glm::vec3(1, 0, 0),
                glm::vec3(1, 1, 0), glm::vec3(1, 1, 1)
            },
            {
                glm::vec2(0, 1), glm::vec2(1, 1),
                glm::vec2(1, 0), glm::vec2(0, 0)
            },
            1 // side
        },
        // Левая грань (-X)
        {
            glm::vec3(-1, 0, 0),
            {
                glm::vec3(0, 0, 0), glm::vec3(0, 0, 1),
                glm::vec3(0, 1, 1), glm::vec3(0, 1, 0)
            },
            {
                glm::vec2(0, 1), glm::vec2(1, 1),
                glm::vec2(1, 0), glm::vec2(0, 0)
            },
            1 // side
        },
        // Верхняя грань (+Y)
        {
            glm::vec3(0, 1, 0),
            {
                glm::vec3(0, 1, 1), glm::vec3(1, 1, 1),
                glm::vec3(1, 1, 0), glm::vec3(0, 1, 0)
            },
            {
                glm::vec2(0, 0), glm::vec2(1, 0),
                glm::vec2(1, 1), glm::vec2(0, 1)
            },
            0 // top
        },
        // Нижняя грань (-Y)
        {
            glm::vec3(0, -1, 0),
            {
                glm::vec3(0, 0, 0), glm::vec3(1, 0, 0),
                glm::vec3(1, 0, 1), glm::vec3(0, 0, 1)
            },
            {
                glm::vec2(0, 0), glm::vec2(1, 0),
                glm::vec2(1, 1), glm::vec2(0, 1)
            },
            2 // bottom
        }
    };

    // Направления для проверки соседей
    glm::ivec3 directions[6] = {
        glm::ivec3(0, 0, 1),   // +Z
        glm::ivec3(0, 0, -1),  // -Z
        glm::ivec3(1, 0, 0),   // +X
        glm::ivec3(-1, 0, 0),  // -X
        glm::ivec3(0, 1, 0),   // +Y
        glm::ivec3(0, -1, 0)   // -Y
    };

    // Добавляем грани
    for (int i = 0; i < 6; ++i) {
        glm::ivec3 neighborPos = glm::ivec3(x, y, z) + directions[i];

        if (ShouldRenderFace(x, y, z, neighborPos.x, neighborPos.y, neighborPos.z)) {
            uint32_t baseIndex = vertices.size();
            const Face& face = faces[i];

            // Определяем индекс текстуры
            uint32_t textureIndex = static_cast<uint32_t>(type) * 3;
            if (face.textureType == 0) { // top
                textureIndex += 0;
            } else if (face.textureType == 1) { // side
                textureIndex += 1;
            } else { // bottom
                textureIndex += 2;
            }

            // Добавляем вершины
            for (int v = 0; v < 4; ++v) {
                Vertex vertex;
                vertex.position = blockPos + face.vertices[v];
                vertex.normal = face.normal;
                vertex.texCoord = face.uvs[v];
                vertex.textureIndex = textureIndex;
                vertices.push_back(vertex);
            }

            // Добавляем индексы
            indices.push_back(baseIndex + 0);
            indices.push_back(baseIndex + 1);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 0);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 3);
        }
    }
}
