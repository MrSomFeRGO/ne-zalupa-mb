//
// Created by mrsomfergo on 12.07.2025.
//

#include "WorldGenerator.h"
#include "FastNoiseLite.h"
#include <algorithm>
#include <random>

WorldGenerator::WorldGenerator() {
    m_terrainNoise = std::make_unique<FastNoiseLite>();
    m_detailNoise = std::make_unique<FastNoiseLite>();
    m_treeNoise = std::make_unique<FastNoiseLite>();
}

WorldGenerator::~WorldGenerator() = default;

void WorldGenerator::Initialize() {
    // Настройка генератора основного ландшафта
    m_terrainNoise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    m_terrainNoise->SetFrequency(TERRAIN_SCALE);
    m_terrainNoise->SetFractalType(FastNoiseLite::FractalType_FBm);
    m_terrainNoise->SetFractalOctaves(4);
    m_terrainNoise->SetFractalLacunarity(2.0f);
    m_terrainNoise->SetFractalGain(0.5f);
    m_terrainNoise->SetSeed(12345);

    // Настройка генератора деталей
    m_detailNoise->SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_detailNoise->SetFrequency(DETAIL_SCALE);
    m_detailNoise->SetFractalType(FastNoiseLite::FractalType_FBm);
    m_detailNoise->SetFractalOctaves(2);
    m_detailNoise->SetSeed(54321);

    // Настройка генератора для размещения деревьев
    m_treeNoise->SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    m_treeNoise->SetFrequency(0.05f);
    m_treeNoise->SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    m_treeNoise->SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance2);
    m_treeNoise->SetSeed(11111);
}

void WorldGenerator::GenerateChunk(Chunk* chunk) {
    GenerateTerrain(chunk);
    GenerateTrees(chunk);
}

void WorldGenerator::GenerateTerrain(Chunk* chunk) {
    const glm::ivec3& chunkPos = chunk->GetPosition();
    const glm::vec3& worldPos = chunk->GetWorldPosition();

    for (int x = 0; x < Chunk::SIZE; ++x) {
        for (int z = 0; z < Chunk::SIZE; ++z) {
            float worldX = worldPos.x + x;
            float worldZ = worldPos.z + z;

            // Получаем высоту terrain
            float terrainHeight = GetTerrainHeight(worldX, worldZ);
            int heightInt = static_cast<int>(terrainHeight);

            for (int y = 0; y < Chunk::HEIGHT; ++y) {
                int worldY = chunkPos.y * Chunk::HEIGHT + y;

                BlockType blockType = GetBlockTypeForHeight(worldY, terrainHeight);
                chunk->SetBlock(x, y, z, blockType);
            }
        }
    }
}

void WorldGenerator::GenerateTrees(Chunk* chunk) {
    const glm::ivec3& chunkPos = chunk->GetPosition();
    const glm::vec3& worldPos = chunk->GetWorldPosition();

    // Генерируем деревья только в надземных чанках
    if (chunkPos.y < 0) return;

    std::mt19937 rng(chunkPos.x * 73856093 ^ chunkPos.z * 19349663);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int x = 2; x < Chunk::SIZE - 2; ++x) {
        for (int z = 2; z < Chunk::SIZE - 2; ++z) {
            float worldX = worldPos.x + x;
            float worldZ = worldPos.z + z;

            // Проверяем, должно ли здесь быть дерево
            float treeValue = m_treeNoise->GetNoise(worldX, worldZ);

            if (treeValue > 0.7f && dist(rng) < 0.1f) {
                // Находим поверхность
                for (int y = Chunk::HEIGHT - 1; y >= 0; --y) {
                    if (chunk->GetBlock(x, y, z) == BlockType::Grass) {
                        // Размещаем дерево
                        if (y + 6 < Chunk::HEIGHT) {
                            PlaceTree(chunk, x, y + 1, z);
                        }
                        break;
                    }
                }
            }
        }
    }
}

void WorldGenerator::PlaceTree(Chunk* chunk, int x, int y, int z) {
    // Ствол дерева
    int trunkHeight = 4 + (rand() % 3);
    for (int h = 0; h < trunkHeight; ++h) {
        if (y + h < Chunk::HEIGHT) {
            chunk->SetBlock(x, y + h, z, BlockType::Wood);
        }
    }

    // Листва
    int leavesStart = y + trunkHeight - 2;
    int leavesEnd = y + trunkHeight + 2;

    for (int ly = leavesStart; ly <= leavesEnd && ly < Chunk::HEIGHT; ++ly) {
        int radius = 2;
        if (ly == leavesEnd) radius = 1;
        if (ly == leavesEnd - 1) radius = 1;

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

float WorldGenerator::GetTerrainHeight(float x, float z) {
    // Основной terrain
    float height = m_terrainNoise->GetNoise(x, z) * TERRAIN_HEIGHT;

    // Добавляем детали
    float detail = m_detailNoise->GetNoise(x, z) * 5.0f;

    return SEA_LEVEL + height + detail;
}

BlockType WorldGenerator::GetBlockTypeForHeight(int worldY, float terrainHeight) {
    if (worldY > terrainHeight) {
        // Над поверхностью
        if (worldY <= SEA_LEVEL) {
            return BlockType::Water;
        }
        return BlockType::Air;
    } else if (worldY == static_cast<int>(terrainHeight)) {
        // Поверхность
        if (worldY <= SEA_LEVEL - 2) {
            return BlockType::Sand;
        } else if (worldY <= SEA_LEVEL + 1) {
            return BlockType::Sand;
        } else {
            return BlockType::Grass;
        }
    } else if (worldY >= terrainHeight - 3) {
        // Под поверхностью
        if (worldY <= SEA_LEVEL) {
            return BlockType::Sand;
        }
        return BlockType::Dirt;
    } else {
        // Глубоко под землей
        return BlockType::Stone;
    }
}
