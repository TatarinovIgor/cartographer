#pragma once
#include "Constants.h"
#include "Renderer.h"
#include "Camera.h"
#include "Input.h"
#include "TileMap.h"
#include "Player.h"
#include "ParticleSystem.h"
#include "AssetManager.h"

class MappingSystem {
public:
    bool active = false;
    int cursorTX = 0, cursorTY = 0;
    int tilesRevealed = 0;

    void enter(int playerTX, int playerTY);
    void exit();
    void update(const Input& input, TileMap& map, Player& player,
                ParticleSystem& particles, Camera& camera, float dt);
    void render(Renderer& renderer, Camera& camera, const AssetManager& assets, float time);

    void renderMinimap(Renderer& renderer, const TileMap& map, const Player& player);

private:
    float cursorBlink = 0;
    float moveCooldown = 0;

    struct RevealAnim {
        int tx, ty;
        float progress;
        TileType revealTo;
    };
    std::vector<RevealAnim> revealAnims;
};
