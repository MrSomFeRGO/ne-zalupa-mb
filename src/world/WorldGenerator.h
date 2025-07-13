//
// Created by mrsomfergo on 13.07.2025.
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

    // Biome system
    enum class BiomeType {
        Plains,
        Desert,
        Mountains,
        Forest,
        Ocean
    };

    // Generation settings
    struct GenerationSettings {
        float terrainScale = 0.008f;
        float terrainHeight = 48.0f;
        float detailScale = 0.04f;
        float detailHeight = 8.0f;
        float caveScale = 0.03f;
        float caveThreshold = 0.4f;
        float treeFrequency = 0.02f;
        int seaLevel = 12;
        int stoneLevel = 8;
        bool generateCaves = true;
        bool generateTrees = true;
        bool generateOres = true;
    };

private:
    void GenerateTerrain(Chunk* chunk);
    void GenerateCaves(Chunk* chunk);
    void GenerateTrees(Chunk* chunk);
    void GenerateOres(Chunk* chunk);
    void GenerateStructures(Chunk* chunk);

    // Terrain height calculation
    float GetTerrainHeight(float x, float z);
    float GetBiomeHeight(float x, float z, BiomeType biome);
    BiomeType GetBiome(float x, float z);

    // Block type determination
    BlockType GetBlockTypeForHeight(int worldY, float terrainHeight, BiomeType biome);
    BlockType GetSurfaceBlock(BiomeType biome);
    BlockType GetSubSurfaceBlock(BiomeType biome);

    // Structure generation
    void PlaceTree(Chunk* chunk, int x, int y, int z, BiomeType biome);
    void PlaceOakTree(Chunk* chunk, int x, int y, int z);
    void PlacePineTree(Chunk* chunk, int x, int y, int z);
    void PlaceCactus(Chunk* chunk, int x, int y, int z);

    // Ore generation
    void PlaceOreVein(Chunk* chunk, BlockType oreType, int centerX, int centerY, int centerZ, int size);

    // Noise generators
    std::unique_ptr<FastNoiseLite> m_terrainNoise;
    std::unique_ptr<FastNoiseLite> m_detailNoise;
    std::unique_ptr<FastNoiseLite> m_biomeNoise;
    std::unique_ptr<FastNoiseLite> m_caveNoise;
    std::unique_ptr<FastNoiseLite> m_treeNoise;
    std::unique_ptr<FastNoiseLite> m_oreNoise;

    // Generation settings
    GenerationSettings m_settings;

    // Seeds for different noise layers
    static constexpr int TERRAIN_SEED = 12345;
    static constexpr int DETAIL_SEED = 54321;
    static constexpr int BIOME_SEED = 98765;
    static constexpr int CAVE_SEED = 13579;
    static constexpr int TREE_SEED = 24680;
    static constexpr int ORE_SEED = 11111;
};