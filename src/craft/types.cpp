//
// Created by admin on 6/6/2024.
//

#include "types.hpp"
#include <iostream>

std::unordered_map<std::string, BlockType> stringToBlockType =
{
        {"grass", BlockType::GRASS},
        {"dirt", BlockType::DIRT},
        {"stone", BlockType::STONE}
};

std::ostream& operator<<(std::ostream& os, BlockType blockType)
{
    switch (blockType)
    {
        case BlockType::GRASS:
            os << "GRASS";
            break;
        case BlockType::DIRT:
            os << "DIRT";
            break;
        case BlockType::STONE:
            os << "STONE";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}
