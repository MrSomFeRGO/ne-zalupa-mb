//
// Created by mrsomfergo on 12.07.2025.
//

#pragma once

#include <cstdint>
#include <string>

enum class BlockType : uint8_t {
    Air = 0,
    Stone = 1,
    Dirt = 2,
    Grass = 3,
    Sand = 4,
    Wood = 5,
    Leaves = 6,
    Water = 7,

    Count
};

struct BlockInfo {
    std::string name;
    std::string topTexture;
    std::string sideTexture;
    std::string bottomTexture;
    bool isTransparent;
    bool isSolid;
};

class Block {
public:
    static void Initialize();
    static const BlockInfo& GetBlockInfo(BlockType type);
    static bool IsTransparent(BlockType type);
    static bool IsSolid(BlockType type);

private:
    static BlockInfo s_blockInfo[static_cast<size_t>(BlockType::Count)];
};
