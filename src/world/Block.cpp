//
// Created by mrsomfergo on 12.07.2025.
//

#include "Block.h"

BlockInfo Block::s_blockInfo[static_cast<size_t>(BlockType::Count)];

void Block::Initialize() {
    // Air
    s_blockInfo[0] = {
        "Air",
        "", "", "",
        true, false
    };

    // Stone
    s_blockInfo[1] = {
        "Stone",
        "stone.png", "stone.png", "stone.png",
        false, true
    };

    // Dirt
    s_blockInfo[2] = {
        "Dirt",
        "dirt.png", "dirt.png", "dirt.png",
        false, true
    };

    // Grass
    s_blockInfo[3] = {
        "Grass",
        "grass_top.png", "grass_side.png", "grass_side.png",
        false, true
    };

    // Sand
    s_blockInfo[4] = {
        "Sand",
        "sand.png", "sand.png", "sand.png",
        false, true
    };

    // Wood
    s_blockInfo[5] = {
        "Wood",
        "wood_top.png", "wood_side.png", "wood_top.png",
        false, true
    };

    // Leaves
    s_blockInfo[6] = {
        "Leaves",
        "leaves.png", "leaves.png", "leaves.png",
        true, true
    };

    // Water
    s_blockInfo[7] = {
        "Water",
        "water.png", "water.png", "water.png",
        true, false
    };
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