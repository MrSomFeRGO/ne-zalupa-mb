//
// Created by mrsomfergo on 12.07.2025.
//

#pragma once

#include "Chunk.h"
#include <memory>

// Forward declaration
class FastNoiseLite;

class WorldGenerator {
public:
    WorldGenerator();
    ~WorldGenerator();

    void Initialize();
    void GenerateChunk(Chunk* chunk);

private:
    void GenerateTerrain(Chunk* chunk);
    void GenerateTrees(Chunk* chunk);
    void PlaceTree(Chunk* chunk, int x, int y, int z);

    float GetTerrainHeight(float x, float z);
    BlockType GetBlockTypeForHeight(int worldY, float terrainHeight);

    // Noise generators
    std::unique_ptr<FastNoiseLite> m_terrainNoise;
    std::unique_ptr<FastNoiseLite> m_detailNoise;
    std::unique_ptr<FastNoiseLite> m_treeNoise;

    // Generation parameters
    static constexpr float TERRAIN_SCALE = 0.01f;
    static constexpr float DETAIL_SCALE = 0.05f;
    static constexpr float TERRAIN_HEIGHT = 32.0f;
    static constexpr int SEA_LEVEL = 10;
    static constexpr int STONE_LEVEL = 5;
};
