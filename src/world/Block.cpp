//
// Created by mrsomfergo on 13.07.2025.
//

#include "Block.h"
#include <iostream>

std::array<BlockInfo, static_cast<size_t>(BlockType::Count)> Block::s_blockInfo;
bool Block::s_initialized = false;

void Block::Initialize() {
    if (s_initialized) return;

    // Air
    s_blockInfo[0] = {
        "Air",
        "", "", "",
        true, false, false,
        0.0f, 0, false
    };

    // Stone
    s_blockInfo[1] = {
        "Stone",
        "stone.png", "stone.png", "stone.png",
        false, true, false,
        1.5f, 0, true
    };

    // Dirt
    s_blockInfo[2] = {
        "Dirt",
        "dirt.png", "dirt.png", "dirt.png",
        false, true, false,
        0.5f, 0, true
    };

    // Grass
    s_blockInfo[3] = {
        "Grass",
        "grass_top.png", "grass_side.png", "dirt.png",
        false, true, false,
        0.6f, 0, true
    };

    // Sand
    s_blockInfo[4] = {
        "Sand",
        "sand.png", "sand.png", "sand.png",
        false, true, false,
        0.5f, 0, true
    };

    // Wood
    s_blockInfo[5] = {
        "Wood",
        "wood_top.png", "wood_side.png", "wood_top.png",
        false, true, false,
        2.0f, 0, true
    };

    // Leaves
    s_blockInfo[6] = {
        "Leaves",
        "leaves.png", "leaves.png", "leaves.png",
        true, true, false,
        0.2f, 0, true
    };

    // Water
    s_blockInfo[7] = {
        "Water",
        "water.png", "water.png", "water.png",
        true, false, true,
        100.0f, 0, false
    };

    // // Cobblestone
    // s_blockInfo[8] = {
    //     "Cobblestone",
    //     "cobblestone.png", "cobblestone.png", "cobblestone.png",
    //     false, true, false,
    //     2.0f, 0, true
    // };
    //
    // // Planks
    // s_blockInfo[9] = {
    //     "Planks",
    //     "planks.png", "planks.png", "planks.png",
    //     false, true, false,
    //     2.0f, 0, true
    // };
    //
    // // Glass
    // s_blockInfo[10] = {
    //     "Glass",
    //     "glass.png", "glass.png", "glass.png",
    //     true, true, false,
    //     0.3f, 0, true
    // };
    //
    // // Iron Ore
    // s_blockInfo[11] = {
    //     "Iron Ore",
    //     "iron_ore.png", "iron_ore.png", "iron_ore.png",
    //     false, true, false,
    //     3.0f, 0, true
    // };
    //
    // // Coal Ore
    // s_blockInfo[12] = {
    //     "Coal Ore",
    //     "coal_ore.png", "coal_ore.png", "coal_ore.png",
    //     false, true, false,
    //     3.0f, 0, true
    // };
    //
    // // Diamond Ore
    // s_blockInfo[13] = {
    //     "Diamond Ore",
    //     "diamond_ore.png", "diamond_ore.png", "diamond_ore.png",
    //     false, true, false,
    //     5.0f, 0, true
    // };
    //
    // // Gold Ore
    // s_blockInfo[14] = {
    //     "Gold Ore",
    //     "gold_ore.png", "gold_ore.png", "gold_ore.png",
    //     false, true, false,
    //     3.0f, 0, true
    // };
    //
    // // Gravel
    // s_blockInfo[15] = {
    //     "Gravel",
    //     "gravel.png", "gravel.png", "gravel.png",
    //     false, true, false,
    //     0.6f, 0, true
    // };

    s_initialized = true;
    std::cout << "Block system initialized with " << static_cast<int>(BlockType::Count) << " block types" << std::endl;
}

const BlockInfo& Block::GetBlockInfo(BlockType type) {
    return s_blockInfo[static_cast<size_t>(type)];
}

bool Block::IsTransparent(BlockType type) {
    return s_blockInfo[static_cast<size_t>(type)].isTransparent;
}

bool Block::IsSolid(BlockType type) {
    return s_blockInfo[static_cast<size_t>(type)].isSolid;
}

bool Block::IsLiquid(BlockType type) {
    return s_blockInfo[static_cast<size_t>(type)].isLiquid;
}

float Block::GetHardness(BlockType type) {
    return s_blockInfo[static_cast<size_t>(type)].hardness;
}

int Block::GetLightLevel(BlockType type) {
    return s_blockInfo[static_cast<size_t>(type)].lightLevel;
}

bool Block::CanBePlaced(BlockType type) {
    return s_blockInfo[static_cast<size_t>(type)].canBePlaced;
}

uint32_t Block::GetTextureIndex(BlockType type, int face) {
    // face: 0=top, 1=side, 2=bottom
    uint32_t baseIndex = static_cast<uint32_t>(type) * 3;

    switch (face) {
        case 0: return baseIndex + 0; // top
        case 1: return baseIndex + 1; // side
        case 2: return baseIndex + 2; // bottom
        default: return baseIndex + 1; // default to side
    }
}

BlockType Block::GetDroppedItem(BlockType type) {
    switch (type) {
        case BlockType::Grass:
            return BlockType::Dirt;
        case BlockType::Stone:
            return BlockType::Cobblestone;
        case BlockType::Leaves:
            // 20% chance to drop nothing (handled elsewhere)
            return BlockType::Air;
        default:
            return type; // Most blocks drop themselves
    }
}

bool Block::CanBreak(BlockType type) {
    switch (type) {
        case BlockType::Air:
            return false;
        default:
            return true;
    }
}