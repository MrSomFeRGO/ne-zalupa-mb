//
// Created by mrsomfergo on 12.07.2025.
//

#pragma once

#include "Chunk.h"
#include "WorldGenerator.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>

// Hash function для glm::ivec3
struct ivec3Hash {
    std::size_t operator()(const glm::ivec3& v) const {
        return std::hash<int>()(v.x) ^
               (std::hash<int>()(v.y) << 1) ^
               (std::hash<int>()(v.z) << 2);
    }
};

class ChunkManager {
public:
    static constexpr int RENDER_DISTANCE = 8;
    static constexpr int LOAD_DISTANCE = RENDER_DISTANCE + 2;
    static constexpr int UNLOAD_DISTANCE = LOAD_DISTANCE + 2;

    ChunkManager();
    ~ChunkManager();

    void Initialize();
    void Update(const glm::vec3& viewerPosition, float deltaTime);
    void GenerateChunkMeshes(nvrhi::IDevice* device, nvrhi::ICommandList* commandList);

    // Доступ к чанкам
    Chunk* GetChunk(const glm::ivec3& position);
    std::vector<Chunk*> GetVisibleChunks() const;

    // Доступ к блокам через мировые координаты
    BlockType GetBlock(int x, int y, int z);
    void SetBlock(int x, int y, int z, BlockType type);

private:
    // Преобразование координат
    glm::ivec3 WorldToChunkPosition(const glm::vec3& worldPos) const;
    glm::ivec3 WorldToBlockPosition(int x, int y, int z) const;
    glm::ivec3 GetChunkPositionFromBlock(int x, int y, int z) const;

    // Управление чанками
    void LoadChunksAroundPosition(const glm::ivec3& centerChunk);
    void UnloadDistantChunks(const glm::ivec3& centerChunk);
    void LoadChunk(const glm::ivec3& position);
    void UnloadChunk(const glm::ivec3& position);
    void UpdateChunkNeighbors(const glm::ivec3& position);

    // Генерация в отдельном потоке
    void GenerationThreadFunc();

    // Хранилище чанков
    std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, ivec3Hash> m_chunks;
    mutable std::mutex m_chunksMutex;

    // Генератор мира
    std::unique_ptr<WorldGenerator> m_worldGenerator;

    // Очередь для генерации
    std::queue<glm::ivec3> m_generationQueue;
    std::mutex m_queueMutex;

    // Поток генерации
    std::thread m_generationThread;
    bool m_shouldStop = false;

    // Текущая позиция наблюдателя в чанковых координатах
    glm::ivec3 m_currentChunkPosition;
};
