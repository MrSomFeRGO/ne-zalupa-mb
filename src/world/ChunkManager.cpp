//
// Created by mrsomfergo on 12.07.2025.
//

#include "ChunkManager.h"
#include <algorithm>
#include <cmath>

ChunkManager::ChunkManager() {
    m_worldGenerator = std::make_unique<WorldGenerator>();
}

ChunkManager::~ChunkManager() {
    m_shouldStop = true;
    if (m_generationThread.joinable()) {
        m_generationThread.join();
    }
}

void ChunkManager::Initialize() {
    Block::Initialize();
    m_worldGenerator->Initialize();

    // Запускаем поток генерации
    m_generationThread = std::thread(&ChunkManager::GenerationThreadFunc, this);

    // Загружаем начальные чанки вокруг (0,0,0)
    LoadChunksAroundPosition(glm::ivec3(0, 0, 0));
}

void ChunkManager::Update(const glm::vec3& viewerPosition, float deltaTime) {
    glm::ivec3 newChunkPosition = WorldToChunkPosition(viewerPosition);

    // Если позиция изменилась, обновляем чанки
    if (newChunkPosition != m_currentChunkPosition) {
        m_currentChunkPosition = newChunkPosition;
        LoadChunksAroundPosition(m_currentChunkPosition);
        UnloadDistantChunks(m_currentChunkPosition);
    }
}

void ChunkManager::GenerateChunkMeshes(nvrhi::IDevice* device, nvrhi::ICommandList* commandList) {
    std::lock_guard<std::mutex> lock(m_chunksMutex);

    for (auto& [pos, chunk] : m_chunks) {
        if (chunk->NeedsMeshUpdate()) {
            chunk->GenerateMesh(device, commandList);
        }
    }
}

Chunk* ChunkManager::GetChunk(const glm::ivec3& position) {
    std::lock_guard<std::mutex> lock(m_chunksMutex);

    auto it = m_chunks.find(position);
    if (it != m_chunks.end()) {
        return it->second.get();
    }
    return nullptr;
}

std::vector<Chunk*> ChunkManager::GetVisibleChunks() const {
    std::vector<Chunk*> visibleChunks;

    std::lock_guard<std::mutex> lock(m_chunksMutex);

    for (const auto& [pos, chunk] : m_chunks) {
        // Проверяем расстояние до чанка
        glm::vec3 chunkCenter = glm::vec3(pos) + glm::vec3(0.5f);
        float distance = glm::length(chunkCenter - glm::vec3(m_currentChunkPosition));

        if (distance <= RENDER_DISTANCE && !chunk->IsEmpty()) {
            visibleChunks.push_back(chunk.get());
        }
    }

    return visibleChunks;
}

BlockType ChunkManager::GetBlock(int x, int y, int z) {
    glm::ivec3 chunkPos = GetChunkPositionFromBlock(x, y, z);
    glm::ivec3 blockPos = WorldToBlockPosition(x, y, z);

    Chunk* chunk = GetChunk(chunkPos);
    if (chunk) {
        return chunk->GetBlock(blockPos.x, blockPos.y, blockPos.z);
    }

    return BlockType::Air;
}

void ChunkManager::SetBlock(int x, int y, int z, BlockType type) {
    glm::ivec3 chunkPos = GetChunkPositionFromBlock(x, y, z);
    glm::ivec3 blockPos = WorldToBlockPosition(x, y, z);

    Chunk* chunk = GetChunk(chunkPos);
    if (chunk) {
        chunk->SetBlock(blockPos.x, blockPos.y, blockPos.z, type);
    }
}

glm::ivec3 ChunkManager::WorldToChunkPosition(const glm::vec3& worldPos) const {
    return glm::ivec3(
        std::floor(worldPos.x / Chunk::SIZE),
        std::floor(worldPos.y / Chunk::HEIGHT),
        std::floor(worldPos.z / Chunk::SIZE)
    );
}

glm::ivec3 ChunkManager::WorldToBlockPosition(int x, int y, int z) const {
    return glm::ivec3(
        ((x % Chunk::SIZE) + Chunk::SIZE) % Chunk::SIZE,
        ((y % Chunk::HEIGHT) + Chunk::HEIGHT) % Chunk::HEIGHT,
        ((z % Chunk::SIZE) + Chunk::SIZE) % Chunk::SIZE
    );
}

glm::ivec3 ChunkManager::GetChunkPositionFromBlock(int x, int y, int z) const {
    return glm::ivec3(
        std::floor((float)x / Chunk::SIZE),
        std::floor((float)y / Chunk::HEIGHT),
        std::floor((float)z / Chunk::SIZE)
    );
}

void ChunkManager::LoadChunksAroundPosition(const glm::ivec3& centerChunk) {
    // Загружаем все чанки в радиусе LOAD_DISTANCE
    for (int x = -LOAD_DISTANCE; x <= LOAD_DISTANCE; ++x) {
        for (int z = -LOAD_DISTANCE; z <= LOAD_DISTANCE; ++z) {
            for (int y = -2; y <= 2; ++y) { // Ограничиваем высоту
                glm::ivec3 chunkPos = centerChunk + glm::ivec3(x, y, z);

                if (glm::length(glm::vec3(x, y * 2, z)) <= LOAD_DISTANCE) {
                    LoadChunk(chunkPos);
                }
            }
        }
    }
}

void ChunkManager::UnloadDistantChunks(const glm::ivec3& centerChunk) {
    std::lock_guard<std::mutex> lock(m_chunksMutex);

    std::vector<glm::ivec3> chunksToUnload;

    for (const auto& [pos, chunk] : m_chunks) {
        glm::vec3 diff = glm::vec3(pos - centerChunk);
        float distance = glm::length(diff);

        if (distance > UNLOAD_DISTANCE) {
            chunksToUnload.push_back(pos);
        }
    }

    for (const auto& pos : chunksToUnload) {
        m_chunks.erase(pos);
    }
}

void ChunkManager::LoadChunk(const glm::ivec3& position) {
    // Проверяем, не загружен ли уже чанк
    {
        std::lock_guard<std::mutex> lock(m_chunksMutex);
        if (m_chunks.find(position) != m_chunks.end()) {
            return;
        }
    }

    // Добавляем в очередь генерации
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_generationQueue.push(position);
    }
}

void ChunkManager::UpdateChunkNeighbors(const glm::ivec3& position) {
    Chunk* chunk = GetChunk(position);
    if (!chunk) return;

    // Устанавливаем соседей
    chunk->SetNeighbor(0, GetChunk(position + glm::ivec3(-1, 0, 0))); // -X
    chunk->SetNeighbor(1, GetChunk(position + glm::ivec3(1, 0, 0)));  // +X
    chunk->SetNeighbor(2, GetChunk(position + glm::ivec3(0, -1, 0))); // -Y
    chunk->SetNeighbor(3, GetChunk(position + glm::ivec3(0, 1, 0)));  // +Y
    chunk->SetNeighbor(4, GetChunk(position + glm::ivec3(0, 0, -1))); // -Z
    chunk->SetNeighbor(5, GetChunk(position + glm::ivec3(0, 0, 1)));  // +Z

    // Обновляем соседей этого чанка
    for (int i = 0; i < 6; ++i) {
        glm::ivec3 offset(0);
        int oppositeDir = 0;

        switch (i) {
            case 0: offset.x = -1; oppositeDir = 1; break;
            case 1: offset.x = 1; oppositeDir = 0; break;
            case 2: offset.y = -1; oppositeDir = 3; break;
            case 3: offset.y = 1; oppositeDir = 2; break;
            case 4: offset.z = -1; oppositeDir = 5; break;
            case 5: offset.z = 1; oppositeDir = 4; break;
        }

        Chunk* neighbor = GetChunk(position + offset);
        if (neighbor) {
            neighbor->SetNeighbor(oppositeDir, chunk);
        }
    }
}

void ChunkManager::GenerationThreadFunc() {
    while (!m_shouldStop) {
        glm::ivec3 chunkPos;
        bool hasWork = false;

        // Получаем следующий чанк для генерации
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            if (!m_generationQueue.empty()) {
                chunkPos = m_generationQueue.front();
                m_generationQueue.pop();
                hasWork = true;
            }
        }

        if (hasWork) {
            // Создаем и генерируем чанк
            auto chunk = std::make_unique<Chunk>(chunkPos);
            m_worldGenerator->GenerateChunk(chunk.get());

            // Добавляем в хранилище
            {
                std::lock_guard<std::mutex> lock(m_chunksMutex);
                m_chunks[chunkPos] = std::move(chunk);
                UpdateChunkNeighbors(chunkPos);
            }
        } else {
            // Если нет работы, немного ждем
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}
