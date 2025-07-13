//
// Created by mrsomfergo on 13.07.2025.
//

#include "WorldGenerator.h"
#include "FastNoiseLite.h"
#include <algorithm>
#include <random>
#include <cmath>

WorldGenerator::WorldGenerator() {
    m_terrainNoise = std::make_unique<FastNoiseLite>();
    m_detailNoise = std::make_unique<FastNoiseLite>();
    m_biomeNoise = std::make_unique<FastNoiseLite>();
    m_caveNoise = std::make_unique<FastNoiseLite>();
    m_treeNoise = std::make_unique<FastNoiseLite>();
    m_oreNoise = std::make_unique<FastNoiseLite>();
}

WorldGenerator::~WorldGenerator() = default;

void WorldGenerator::Initialize() {
    // Main terrain noise
    m_terrainNoise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_terrainNoise->SetFrequency(m_settings.terrainScale);
    m_terrainNoise->SetFractalType(FastNoiseLite::FractalType_FBm);
    m_terrainNoise->SetFractalOctaves(4);
    m_terrainNoise->SetFractalLacunarity(2.0f);
    m_terrainNoise->SetFractalGain(0.5f);
    m_terrainNoise->SetSeed(TERRAIN_SEED);

    // Detail noise for terrain variation
    m_detailNoise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_detailNoise->SetFrequency(m_settings.detailScale);
    m_detailNoise->SetFractalType(FastNoiseLite::FractalType_FBm);
    m_detailNoise->SetFractalOctaves(3);
    m_detailNoise->SetSeed(DETAIL_SEED);

    // Biome noise
    m_biomeNoise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_biomeNoise->SetFrequency(0.003f);
    m_biomeNoise->SetFractalType(FastNoiseLite::FractalType_FBm);
    m_biomeNoise->SetFractalOctaves(2);
    m_biomeNoise->SetSeed(BIOME_SEED);

    // Cave noise
    m_caveNoise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_caveNoise->SetFrequency(m_settings.caveScale);
    m_caveNoise->SetFractalType(FastNoiseLite::FractalType_Ridged);
    m_caveNoise->SetFractalOctaves(2);
    m_caveNoise->SetSeed(CAVE_SEED);

    // Tree placement noise
    m_treeNoise->SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    m_treeNoise->SetFrequency(m_settings.treeFrequency);
    m_treeNoise->SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    m_treeNoise->SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2);
    m_treeNoise->SetSeed(TREE_SEED);

    // Ore placement noise
    m_oreNoise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_oreNoise->SetFrequency(0.1f);
    m_oreNoise->SetSeed(ORE_SEED);
}

void WorldGenerator::GenerateChunk(Chunk* chunk) {
    GenerateTerrain(chunk);

    if (m_settings.generateCaves) {
        GenerateCaves(chunk);
    }

    if (m_settings.generateOres) {
        GenerateOres(chunk);
    }

    if (m_settings.generateTrees) {
        GenerateTrees(chunk);
    }
}

void WorldGenerator::GenerateTerrain(Chunk* chunk) {
    const glm::ivec3& chunkPos = chunk->GetPosition();
    const glm::vec3& worldPos = chunk->GetWorldPosition();

    for (int x = 0; x < Chunk::SIZE; ++x) {
        for (int z = 0; z < Chunk::SIZE; ++z) {
            float worldX = worldPos.x + x;
            float worldZ = worldPos.z + z;

            // Get biome and terrain height
            BiomeType biome = GetBiome(worldX, worldZ);
            float terrainHeight = GetTerrainHeight(worldX, worldZ);
            int heightInt = static_cast<int>(terrainHeight);

            for (int y = 0; y < Chunk::HEIGHT; ++y) {
                int worldY = chunkPos.y * Chunk::HEIGHT + y;

                BlockType blockType = GetBlockTypeForHeight(worldY, terrainHeight, biome);
                chunk->SetBlock(x, y, z, blockType);
            }
        }
    }
}

void WorldGenerator::GenerateCaves(Chunk* chunk) {
    const glm::ivec3& chunkPos = chunk->GetPosition();
    const glm::vec3& worldPos = chunk->GetWorldPosition();

    // Only generate caves below sea level + some margin
    if (chunkPos.y > 1) return;

    for (int x = 0; x < Chunk::SIZE; ++x) {
        for (int y = 0; y < Chunk::HEIGHT; ++y) {
            for (int z = 0; z < Chunk::SIZE; ++z) {
                int worldY = chunkPos.y * Chunk::HEIGHT + y;

                // Don't generate caves too close to surface or too deep
                if (worldY < 2 || worldY > m_settings.seaLevel + 5) continue;

                float worldX = worldPos.x + x;
                float worldZ = worldPos.z + z;

                // Generate cave noise
                float caveValue = m_caveNoise->GetNoise(worldX, worldY * 0.5f, worldZ);

                // Create caves where noise is above threshold
                if (caveValue > m_settings.caveThreshold) {
                    BlockType currentBlock = chunk->GetBlock(x, y, z);
                    if (currentBlock != BlockType::Air && currentBlock != BlockType::Water) {
                        chunk->SetBlock(x, y, z, BlockType::Air);
                    }
                }
            }
        }
    }
}

void WorldGenerator::GenerateTrees(Chunk* chunk) {
    const glm::ivec3& chunkPos = chunk->GetPosition();
    const glm::vec3& worldPos = chunk->GetWorldPosition();

    // Only generate trees in above-ground chunks
    if (chunkPos.y < 0) return;

    std::mt19937 rng(chunkPos.x * 73856093 ^ chunkPos.z * 19349663 ^ chunkPos.y * 83492791);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int x = 2; x < Chunk::SIZE - 2; ++x) {
        for (int z = 2; z < Chunk::SIZE - 2; ++z) {
            float worldX = worldPos.x + x;
            float worldZ = worldPos.z + z;

            BiomeType biome = GetBiome(worldX, worldZ);

            // Skip tree generation in desert and ocean biomes
            if (biome == BiomeType::Desert || biome == BiomeType::Ocean) {
                continue;
            }

            // Check tree noise
            float treeValue = m_treeNoise->GetNoise(worldX, worldZ);
            float treeChance = 0.1f;

            // Adjust tree chance based on biome
            switch (biome) {
                case BiomeType::Forest: treeChance = 0.3f; break;
                case BiomeType::Plains: treeChance = 0.05f; break;
                case BiomeType::Mountains: treeChance = 0.15f; break;
                default: break;
            }

            if (treeValue > 0.7f && dist(rng) < treeChance) {
                // Find surface block
                for (int y = Chunk::HEIGHT - 1; y >= 0; --y) {
                    BlockType surfaceBlock = chunk->GetBlock(x, y, z);
                    if (surfaceBlock == BlockType::Grass || surfaceBlock == BlockType::Dirt) {
                        // Place tree
                        if (y + 6 < Chunk::HEIGHT) {
                            PlaceTree(chunk, x, y + 1, z, biome);
                        }
                        break;
                    }
                }
            }
        }
    }
}

void WorldGenerator::GenerateOres(Chunk* chunk) {
    const glm::ivec3& chunkPos = chunk->GetPosition();
    const glm::vec3& worldPos = chunk->GetWorldPosition();

    // Only generate ores underground
    if (chunkPos.y > 0) return;

    std::mt19937 rng(chunkPos.x * 73856093 ^ chunkPos.z * 19349663 ^ chunkPos.y * 83492791);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    std::uniform_int_distribution<int> sizeDist(2, 6);
    std::uniform_int_distribution<int> posDist(0, Chunk::SIZE - 1);

    // Generate different ore types based on depth
    int worldY = chunkPos.y * Chunk::HEIGHT;

    // Iron ore (common, mid-depth)
    if (worldY >= -32 && worldY <= -8 && dist(rng) < 0.3f) {
        int x = posDist(rng);
        int y = posDist(rng);
        int z = posDist(rng);
        PlaceOreVein(chunk, BlockType::Stone, x, y, z, sizeDist(rng)); // Using stone as placeholder
    }
}

float WorldGenerator::GetTerrainHeight(float x, float z) {
    BiomeType biome = GetBiome(x, z);

    // Base terrain height
    float baseHeight = m_terrainNoise->GetNoise(x, z) * m_settings.terrainHeight;

    // Detail noise
    float detail = m_detailNoise->GetNoise(x, z) * m_settings.detailHeight;

    // Biome-specific modifications
    float biomeModifier = GetBiomeHeight(x, z, biome);

    return m_settings.seaLevel + baseHeight + detail + biomeModifier;
}

float WorldGenerator::GetBiomeHeight(float x, float z, BiomeType biome) {
    switch (biome) {
        case BiomeType::Mountains:
            return m_terrainNoise->GetNoise(x * 0.003f, z * 0.003f) * 32.0f;
        case BiomeType::Ocean:
            return -8.0f;
        case BiomeType::Desert:
            return m_detailNoise->GetNoise(x * 0.02f, z * 0.02f) * 4.0f;
        case BiomeType::Plains:
            return 0.0f;
        case BiomeType::Forest:
            return m_detailNoise->GetNoise(x * 0.01f, z * 0.01f) * 6.0f;
        default:
            return 0.0f;
    }
}

WorldGenerator::BiomeType WorldGenerator::GetBiome(float x, float z) {
    float biomeValue = m_biomeNoise->GetNoise(x, z);
    float temperatureValue = m_biomeNoise->GetNoise(x * 1.5f, z * 1.5f);

    // Combine noise values to determine biome
    if (biomeValue < -0.4f) {
        return BiomeType::Ocean;
    } else if (biomeValue > 0.4f && temperatureValue > 0.2f) {
        return BiomeType::Mountains;
    } else if (temperatureValue > 0.3f) {
        return BiomeType::Desert;
    } else if (temperatureValue < -0.2f) {
        return BiomeType::Forest;
    } else {
        return BiomeType::Plains;
    }
}

BlockType WorldGenerator::GetBlockTypeForHeight(int worldY, float terrainHeight, BiomeType biome) {
    if (worldY > terrainHeight) {
        // Above terrain
        if (worldY <= m_settings.seaLevel) {
            return BlockType::Water;
        }
        return BlockType::Air;
    } else if (worldY == static_cast<int>(terrainHeight)) {
        // Surface block
        return GetSurfaceBlock(biome);
    } else if (worldY >= terrainHeight - 3) {
        // Sub-surface
        return GetSubSurfaceBlock(biome);
    } else {
        // Deep underground
        return BlockType::Stone;
    }
}

BlockType WorldGenerator::GetSurfaceBlock(BiomeType biome) {
    switch (biome) {
        case BiomeType::Desert: return BlockType::Sand;
        case BiomeType::Ocean: return BlockType::Sand;
        case BiomeType::Mountains: return BlockType::Stone;
        default: return BlockType::Grass;
    }
}

BlockType WorldGenerator::GetSubSurfaceBlock(BiomeType biome) {
    switch (biome) {
        case BiomeType::Desert: return BlockType::Sand;
        case BiomeType::Ocean: return BlockType::Sand;
        default: return BlockType::Dirt;
    }
}

void WorldGenerator::PlaceTree(Chunk* chunk, int x, int y, int z, BiomeType biome) {
    switch (biome) {
        case BiomeType::Forest:
        case BiomeType::Mountains:
            PlacePineTree(chunk, x, y, z);
            break;
        case BiomeType::Desert:
            PlaceCactus(chunk, x, y, z);
            break;
        default:
            PlaceOakTree(chunk, x, y, z);
            break;
    }
}

void WorldGenerator::PlaceOakTree(Chunk* chunk, int x, int y, int z) {
    // Tree trunk
    int trunkHeight = 4 + (rand() % 3);
    for (int h = 0; h < trunkHeight; ++h) {
        if (y + h < Chunk::HEIGHT) {
            chunk->SetBlock(x, y + h, z, BlockType::Wood);
        }
    }

    // Tree leaves (simple sphere shape)
    int leavesStart = y + trunkHeight - 2;
    int leavesEnd = y + trunkHeight + 2;

    for (int ly = leavesStart; ly <= leavesEnd && ly < Chunk::HEIGHT; ++ly) {
        int radius = 2;
        if (ly == leavesEnd) radius = 1;

        for (int lx = -radius; lx <= radius; ++lx) {
            for (int lz = -radius; lz <= radius; ++lz) {
                if (lx * lx + lz * lz <= radius * radius) {
                    int px = x + lx;
                    int pz = z + lz;

                    if (px >= 0 && px < Chunk::SIZE && pz >= 0 && pz < Chunk::SIZE) {
                        if (chunk->GetBlock(px, ly, pz) == BlockType::Air) {
                            chunk->SetBlock(px, ly, pz, BlockType::Leaves);
                        }
                    }
                }
            }
        }
    }
}

void WorldGenerator::PlacePineTree(Chunk* chunk, int x, int y, int z) {
    // Taller, thinner tree
    int trunkHeight = 6 + (rand() % 4);
    for (int h = 0; h < trunkHeight; ++h) {
        if (y + h < Chunk::HEIGHT) {
            chunk->SetBlock(x, y + h, z, BlockType::Wood);
        }
    }

    // Conical leaves
    for (int layer = 0; layer < 4; ++layer) {
        int ly = y + trunkHeight - 1 - layer;
        if (ly >= Chunk::HEIGHT) continue;

        int radius = 1 + (layer / 2);

        for (int lx = -radius; lx <= radius; ++lx) {
            for (int lz = -radius; lz <= radius; ++lz) {
                if (abs(lx) + abs(lz) <= radius) {
                    int px = x + lx;
                    int pz = z + lz;

                    if (px >= 0 && px < Chunk::SIZE && pz >= 0 && pz < Chunk::SIZE) {
                        if (chunk->GetBlock(px, ly, pz) == BlockType::Air) {
                            chunk->SetBlock(px, ly, pz, BlockType::Leaves);
                        }
                    }
                }
            }
        }
    }
}

void WorldGenerator::PlaceCactus(Chunk* chunk, int x, int y, int z) {
    // Simple cactus
    int cactusHeight = 2 + (rand() % 3);
    for (int h = 0; h < cactusHeight; ++h) {
        if (y + h < Chunk::HEIGHT) {
            chunk->SetBlock(x, y + h, z, BlockType::Leaves); // Using leaves as cactus placeholder
        }
    }
}

void WorldGenerator::PlaceOreVein(Chunk* chunk, BlockType oreType, int centerX, int centerY, int centerZ, int size) {
    std::mt19937 rng(centerX * 73856093 ^ centerY * 19349663 ^ centerZ * 83492791);
    std::uniform_int_distribution<int> offsetDist(-1, 1);

    for (int i = 0; i < size; ++i) {
        int x = centerX + offsetDist(rng);
        int y = centerY + offsetDist(rng);
        int z = centerZ + offsetDist(rng);

        if (x >= 0 && x < Chunk::SIZE && y >= 0 && y < Chunk::HEIGHT && z >= 0 && z < Chunk::SIZE) {
            if (chunk->GetBlock(x, y, z) == BlockType::Stone) {
                chunk->SetBlock(x, y, z, oreType);
            }
        }
    }
}