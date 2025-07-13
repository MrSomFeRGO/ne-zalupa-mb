//
// Created by mrsomfergo on 13.07.2025.
//

#pragma once

#include <cstdint>
#include <string>
#include <array>

enum class BlockType : uint8_t {
    Air = 0,
    Stone = 1,
    Dirt = 2,
    Grass = 3,
    Sand = 4,
    Wood = 5,
    Leaves = 6,
    Water = 7,
    Cobblestone = 8,
    Planks = 9,
    Glass = 10,
    IronOre = 11,
    CoalOre = 12,
    DiamondOre = 13,
    GoldOre = 14,
    Gravel = 15,

    Count
};

struct BlockInfo {
    std::string name;
    std::string topTexture;
    std::string sideTexture;
    std::string bottomTexture;
    bool isTransparent;
    bool isSolid;
    bool isLiquid;
    float hardness;        // Mining time multiplier
    int lightLevel;        // Emitted light (0-15)
    bool canBePlaced;      // Can player place this block
};

class Block {
public:
    static void Initialize();
    static const BlockInfo& GetBlockInfo(BlockType type);
    static bool IsTransparent(BlockType type);
    static bool IsSolid(BlockType type);
    static bool IsLiquid(BlockType type);
    static float GetHardness(BlockType type);
    static int GetLightLevel(BlockType type);
    static bool CanBePlaced(BlockType type);

    // Texture index calculation
    static uint32_t GetTextureIndex(BlockType type, int face);

    // Block interaction
    static BlockType GetDroppedItem(BlockType type);
    static bool CanBreak(BlockType type);

private:
    static std::array<BlockInfo, static_cast<size_t>(BlockType::Count)> s_blockInfo;
    static bool s_initialized;
};