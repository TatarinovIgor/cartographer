#pragma once

namespace Constants {
    constexpr int SCREEN_WIDTH = 1280;
    constexpr int SCREEN_HEIGHT = 720;
    constexpr int TILE_SIZE = 32;
    constexpr int MAP_WIDTH = 50;
    constexpr int MAP_HEIGHT = 38;
    constexpr float PLAYER_SPEED = 160.0f;
    constexpr float CAMERA_LERP = 5.0f;
    constexpr int TARGET_FPS = 60;
    constexpr float FRAME_TIME = 1.0f / TARGET_FPS;
    constexpr int MAX_INK = 100;
    constexpr float VOID_CREEP_INTERVAL = 30.0f;
    constexpr float TYPEWRITER_SPEED = 0.03f;
    constexpr int DIALOGUE_BOX_HEIGHT = 160;
    constexpr int INTERACT_RANGE = 48;

    namespace Colors {
        struct Color { unsigned char r, g, b, a; };

        constexpr Color GROUND       = {139, 119, 101, 255};
        constexpr Color GROUND_ALT   = {149, 129, 111, 255};
        constexpr Color WALL         = { 72,  60,  50, 255};
        constexpr Color WALL_DARK    = { 55,  45,  38, 255};
        constexpr Color ROOF         = {120,  70,  50, 255};
        constexpr Color PATH         = {170, 150, 130, 255};
        constexpr Color WATER        = { 70, 110, 140, 255};
        constexpr Color WATER_DEEP   = { 50,  90, 120, 255};
        constexpr Color VOID_WHITE   = {245, 245, 250, 255};
        constexpr Color VOID_SHIMMER = {230, 230, 240, 255};
        constexpr Color HIDDEN       = {100, 100, 100, 128};
        constexpr Color SABLE        = {210, 160,  60, 255};
        constexpr Color SABLE_CLOAK  = { 80,  60,  45, 255};
        constexpr Color NPC_INNKEEPER= {140, 100,  70, 255};
        constexpr Color NPC_CARTO    = { 60,  90, 130, 255};
        constexpr Color NPC_CHILD    = {180, 140, 160, 255};
        constexpr Color INK_DARK     = { 20,  15,  10, 255};
        constexpr Color INK_SPREAD   = { 40,  30,  20, 180};
        constexpr Color UI_BG        = { 20,  18,  15, 200};
        constexpr Color UI_TEXT      = {220, 210, 195, 255};
        constexpr Color UI_HIGHLIGHT = {210, 170,  60, 255};
        constexpr Color PARCHMENT    = {200, 180, 140,  80};
        constexpr Color TREE_TRUNK   = { 80,  60,  40, 255};
        constexpr Color TREE_CANOPY  = { 60,  90,  50, 255};
        constexpr Color GRASS        = {100, 120,  70, 255};
        constexpr Color FLOWER       = {190, 130, 100, 255};
    }
}
