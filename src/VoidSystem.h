#pragma once
#include "TileMap.h"
#include "ParticleSystem.h"
#include "Camera.h"

class VoidSystem {
public:
    float creepTimer = 0;
    int tilesConsumed = 0;
    bool finalRevealed = false;

    void update(TileMap& map, ParticleSystem& particles, Camera& camera, float dt);
    void creepOnce(TileMap& map, ParticleSystem& particles, Camera& camera);
    bool shouldTriggerEnding(const TileMap& map, int tilesRevealed) const;

private:
    float shimmerTimer = 0;
};
