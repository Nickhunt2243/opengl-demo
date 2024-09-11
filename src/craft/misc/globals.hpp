//
// Created by admin on 8/23/2024.
//

#ifndef OPENGLDEMO_GLOBALS_HPP
#define OPENGLDEMO_GLOBALS_HPP

namespace Craft
{
    /*  World Information Globals  */
    const int CHUNK_WIDTH = 16;
    const int CHUNK_HEIGHT = 256;
    const int RENDER_DISTANCE = 5;
    const int CHUNK_BASE_HEIGHT = 100;

    /*  Player Globals  */
    const long double PLAYER_FRONT_BOUND = 0.15l;
    const long double PLAYER_BACK_BOUND = 0.15l;
    const long double PLAYER_LEFT_BOUND = 0.3l;
    const long double PLAYER_RIGHT_BOUND = 0.3l;
    const float JUMP_HEIGHT = 1.25f;
    const float PLAYER_EYE_DIFF = 0.38f;
    const float REACH_DISTANCE = 4.5l;
    const long double PLAYER_BOUND = 0.3l;
    const long double PLAYER_BOUND_SQRD = PLAYER_BOUND * PLAYER_BOUND;
    const long double EPSILON = 1e-4;
    const long double VERTICAL_TOLERANCE = 1e-1;

    /*  Weather Globals  */
    const int SUN_DISTANCE = 384; // Allows 48 Chunks without overlap but will
    const int SUN_WIDTH = 50;
    const float TIME_CONVERSION = 1.0f / 13.8f; /* Each 1s in Minecraft is 13.8 ms in real time. */

    /*  Const Expressions  */
//    constexpr int CHUNK_BOUNDS = ;
    constexpr int CHUNK_SIZE = CHUNK_WIDTH * CHUNK_WIDTH;
    constexpr int TOTAL_CHUNK_WIDTH = RENDER_DISTANCE + RENDER_DISTANCE + 1;
    constexpr int TOTAL_MAX_CHUNKS = TOTAL_CHUNK_WIDTH * TOTAL_CHUNK_WIDTH;
    constexpr int BLOCKS_IN_CHUNK = CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT;
    constexpr int MAX_WORLD_BLOCKS = BLOCKS_IN_CHUNK * TOTAL_MAX_CHUNKS;
    constexpr int MAX_BLOCKS = CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_HEIGHT;
    constexpr float M_PI = 3.14159265358979323846f;
    constexpr float M_PI_4 = M_PI / 4.0f;

}


#endif //OPENGLDEMO_GLOBALS_HPP
