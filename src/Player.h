#pragma once
#include "Constants.h"
#include "Renderer.h"
#include "Camera.h"
#include "Input.h"
#include "TileMap.h"

class Player {
public:
    float x = 0, y = 0;
    int facingX = 0, facingY = 1;
    int ink = Constants::MAX_INK;

    void init(float startX, float startY);
    void update(const Input& input, const TileMap& map, float dt);
    void render(Renderer& renderer, Camera& camera, float time);

    int tileX() const { return (int)(x / Constants::TILE_SIZE); }
    int tileY() const { return (int)(y / Constants::TILE_SIZE); }
    float centerX() const { return x + Constants::TILE_SIZE / 2.0f; }
    float centerY() const { return y + Constants::TILE_SIZE / 2.0f; }

    bool useInk(int amount);
    void addInk(int amount);

private:
    float animTimer = 0;
    bool moving = false;

    bool canMove(const TileMap& map, float nx, float ny) const;
};
