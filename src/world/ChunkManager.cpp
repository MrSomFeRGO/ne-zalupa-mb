//
// Created by mrsomfergo on 13.07.2025.
//

#include "ChunkManager.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <queue>

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

    // Start generation thread
    m_generationThread = std::thread(&ChunkManager::GenerationThreadFunc, this);

    // Load initial chunks around origin
    LoadChunksAroundPosition(glm::ivec3(0, 0, 0));

    std::cout << "ChunkManager initialized" << std::endl;
}

void ChunkManager::Update(const glm::vec3& viewerPosition, float deltaTime) {
    m_updateTimer += deltaTime;

    // Throttle chunk loading to avoid frame drops
    if (m_updateTimer < UPDATE_INTERVAL) {
        // Still process generated chunks every frame
        UpdateChunkMeshes();
        return;
    }

    m_updateTimer = 0.0f;

    glm::ivec3 newChunkPosition = WorldToChunkPosition(viewerPosition);

    // Update chunks if position changed significantly
    if (newChunkPosition != m_currentChunkPosition ||
        glm::distance(viewerPosition, m_lastViewerPosition) > 8.0f) {

        m_currentChunkPosition = newChunkPosition;
        m_lastViewerPosition = viewerPosition;

        LoadChunksAroundPosition(m_currentChunkPosition);
        UnloadDistantChunks(m_currentChunkPosition);
    }

    // Process generated chunks
    UpdateChunkMeshes();
}

void ChunkManager::UpdateChunkMeshes() {
    // Process generated chunks from background thread
    size_t newChunks = 0;
    {
        std::lock_guard<std::mutex> generatedLock(m_generatedMutex);
        while (!m_generatedChunks.empty()) {
            auto chunk = std::move(m_generatedChunks.front());
            m_generatedChunks.pop();

            glm::ivec3 position = chunk->GetPosition();

            // Create OpenGL objects for new chunk (main thread only!)
            chunk->CreateOpenGLObjects();

            // Add to main chunk storage
            {
                std::lock_guard<std::mutex> chunksLock(m_chunksMutex);
                m_chunks[position] = std::move(chunk);
                UpdateChunkNeighbors(position);
            }
            newChunks++;
        }
    }

    if (newChunks > 0) {
        std::cout << "Loaded " << newChunks << " new chunks (OpenGL objects created in main thread)" << std::endl;
    }

    // Update meshes for dirty chunks (OpenGL calls in main thread)
    std::lock_guard<std::mutex> lock(m_chunksMutex);
    for (auto& [pos, chunk] : m_chunks) {
        if (chunk->NeedsMeshUpdate()) {
            chunk->GenerateMesh(); // This will update buffers
        }
    }
}

Chunk* ChunkManager::GetChunk(const glm::ivec3& position) {
    std::lock_guard<std::mutex> lock(m_chunksMutex);
    auto it = m_chunks.find(position);
    return (it != m_chunks.end()) ? it->second.get() : nullptr;
}

std::vector<Chunk*> ChunkManager::GetVisibleChunks() const {
    std::vector<Chunk*> visibleChunks;

    std::lock_guard<std::mutex> lock(m_chunksMutex);
    visibleChunks.reserve(m_chunks.size());

    for (const auto& [pos, chunk] : m_chunks) {
        // Check distance from current viewer position
        glm::vec3 chunkCenter = glm::vec3(pos) * glm::vec3(Chunk::SIZE, Chunk::HEIGHT, Chunk::SIZE) +
                               glm::vec3(Chunk::SIZE, Chunk::HEIGHT, Chunk::SIZE) * 0.5f;
        glm::vec3 viewerChunkCenter = glm::vec3(m_currentChunkPosition) *
                                     glm::vec3(Chunk::SIZE, Chunk::HEIGHT, Chunk::SIZE);

        float distance = glm::length(chunkCenter - viewerChunkCenter) / Chunk::SIZE;

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
    // Use priority queue to load closest chunks first
    std::priority_queue<ChunkLoadRequest> loadQueue;

    // Generate load requests
    for (int x = -LOAD_DISTANCE; x <= LOAD_DISTANCE; ++x) {
        for (int z = -LOAD_DISTANCE; z <= LOAD_DISTANCE; ++z) {
            for (int y = -2; y <= 2; ++y) { // Limit vertical range
                glm::ivec3 chunkPos = centerChunk + glm::ivec3(x, y, z);

                float distance = glm::length(glm::vec3(x, y * 2, z)); // Weight Y more

                if (distance <= LOAD_DISTANCE) {
                    // Check if chunk already exists
                    bool exists = false;
                    {
                        std::lock_guard<std::mutex> lock(m_chunksMutex);
                        exists = m_chunks.find(chunkPos) != m_chunks.end();
                    }

                    if (!exists) {
                        loadQueue.push({chunkPos, distance});
                    }
                }
            }
        }
    }

    // Process load requests
    while (!loadQueue.empty()) {
        ChunkLoadRequest request = loadQueue.top();
        loadQueue.pop();
        RequestChunkGeneration(request.position);
    }
}

void ChunkManager::UnloadDistantChunks(const glm::ivec3& centerChunk) {
    std::vector<glm::ivec3> chunksToUnload;

    {
        std::lock_guard<std::mutex> lock(m_chunksMutex);

        for (const auto& [pos, chunk] : m_chunks) {
            glm::vec3 diff = glm::vec3(pos - centerChunk);
            float distance = glm::length(diff);

            if (distance > UNLOAD_DISTANCE) {
                chunksToUnload.push_back(pos);
            }
        }

        // Remove chunks
        for (const auto& pos : chunksToUnload) {
            m_chunks.erase(pos);
        }
    }

    if (!chunksToUnload.empty()) {
        std::cout << "Unloaded " << chunksToUnload.size() << " distant chunks" << std::endl;
    }
}

void ChunkManager::RequestChunkGeneration(const glm::ivec3& position) {
    // Check if already in queue
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);

        // Simple check - in production you'd want a set to track queued positions
        std::queue<glm::ivec3> tempQueue = m_generationQueue;
        while (!tempQueue.empty()) {
            if (tempQueue.front() == position) {
                return; // Already queued
            }
            tempQueue.pop();
        }

        m_generationQueue.push(position);
    }
}

void ChunkManager::UpdateChunkNeighbors(const glm::ivec3& position) {
    Chunk* chunk = GetChunk(position);
    if (!chunk) return;

    // Set neighbors
    chunk->SetNeighbor(0, GetChunk(position + glm::ivec3(-1, 0, 0))); // -X
    chunk->SetNeighbor(1, GetChunk(position + glm::ivec3(1, 0, 0)));  // +X
    chunk->SetNeighbor(2, GetChunk(position + glm::ivec3(0, -1, 0))); // -Y
    chunk->SetNeighbor(3, GetChunk(position + glm::ivec3(0, 1, 0)));  // +Y
    chunk->SetNeighbor(4, GetChunk(position + glm::ivec3(0, 0, -1))); // -Z
    chunk->SetNeighbor(5, GetChunk(position + glm::ivec3(0, 0, 1)));  // +Z

    // Update neighbors to point back to this chunk
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
            neighbor->MarkDirty(); // Neighbor might need mesh update
        }
    }
}

void ChunkManager::GenerationThreadFunc() {
    std::cout << "Chunk generation thread started" << std::endl;

    while (!m_shouldStop) {
        glm::ivec3 chunkPos;
        bool hasWork = false;

        // Get next chunk to generate
        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            if (!m_generationQueue.empty()) {
                chunkPos = m_generationQueue.front();
                m_generationQueue.pop();
                hasWork = true;
            }
        }

        if (hasWork) {
            // Generate chunk
            auto chunk = std::make_unique<Chunk>(chunkPos);
            m_worldGenerator->GenerateChunk(chunk.get());

            // Add to generated chunks queue
            {
                std::lock_guard<std::mutex> lock(m_generatedMutex);
                m_generatedChunks.push(std::move(chunk));
            }
        } else {
            // No work, sleep briefly
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    std::cout << "Chunk generation thread stopped" << std::endl;
}

size_t ChunkManager::GetLoadedChunkCount() const {
    std::lock_guard<std::mutex> lock(m_chunksMutex);
    return m_chunks.size();
}

size_t ChunkManager::GetTotalMemoryUsage() const {
    size_t total = 0;
    std::lock_guard<std::mutex> lock(m_chunksMutex);

    for (const auto& [pos, chunk] : m_chunks) {
        total += chunk->GetMemoryUsage();
    }

    return total;
}

size_t ChunkManager::GetGenerationQueueSize() const {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    return m_generationQueue.size();
}