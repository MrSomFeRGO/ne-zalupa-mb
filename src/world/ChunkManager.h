//
// Created by mrsomfergo on 13.07.2025.
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
#include <atomic>

// Hash function for glm::ivec3
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

    // Chunk access
    Chunk* GetChunk(const glm::ivec3& position);
    std::vector<Chunk*> GetVisibleChunks() const;

    // Block access through world coordinates
    BlockType GetBlock(int x, int y, int z);
    void SetBlock(int x, int y, int z, BlockType type);

    // Statistics
    size_t GetLoadedChunkCount() const;
    size_t GetTotalMemoryUsage() const;

    // Generation status
    bool IsGenerationComplete() const { return m_generationQueue.empty(); }
    size_t GetGenerationQueueSize() const;

private:
    // Coordinate conversion
    glm::ivec3 WorldToChunkPosition(const glm::vec3& worldPos) const;
    glm::ivec3 WorldToBlockPosition(int x, int y, int z) const;
    glm::ivec3 GetChunkPositionFromBlock(int x, int y, int z) const;

    // Chunk management
    void LoadChunksAroundPosition(const glm::ivec3& centerChunk);
    void UnloadDistantChunks(const glm::ivec3& centerChunk);
    void LoadChunk(const glm::ivec3& position);
    void UpdateChunkNeighbors(const glm::ivec3& position);

    // Mesh generation in main thread
    void UpdateChunkMeshes();

    // Generation thread
    void GenerationThreadFunc();
    void RequestChunkGeneration(const glm::ivec3& position);

    // Chunk storage
    std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, ivec3Hash> m_chunks;
    mutable std::mutex m_chunksMutex;

    // World generator
    std::unique_ptr<WorldGenerator> m_worldGenerator;

    // Generation queue and thread
    std::queue<glm::ivec3> m_generationQueue;
    std::queue<std::unique_ptr<Chunk>> m_generatedChunks;
    mutable std::mutex m_queueMutex;
    std::mutex m_generatedMutex;
    std::thread m_generationThread;
    std::atomic<bool> m_shouldStop{false};

    // Current viewer position
    glm::ivec3 m_currentChunkPosition;
    glm::vec3 m_lastViewerPosition;

    // Performance tracking
    float m_updateTimer = 0.0f;
    static constexpr float UPDATE_INTERVAL = 0.1f; // Update chunks every 100ms

    // Chunk loading priority
    struct ChunkLoadRequest {
        glm::ivec3 position;
        float distance;

        bool operator<(const ChunkLoadRequest& other) const {
            return distance > other.distance; // Min heap (closest first)
        }
    };
};