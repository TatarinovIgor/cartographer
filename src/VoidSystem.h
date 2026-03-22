#pragma once
#include "TileMap.h"
#include "ParticleSystem.h"
#include "Camera.h"
#include <vector>
#include <unordered_map>

struct VoidMemory {
    int tx, ty;
    TileType originalType;
    float memoryStrength;
};

class VoidSystem {
public:
    float creepTimer = 0;
    int tilesConsumed = 0;
    bool finalRevealed = false;

    void update(TileMap& map, ParticleSystem& particles, Camera& camera, float dt);
    void creepOnce(TileMap& map, ParticleSystem& particles, Camera& camera);
    bool shouldTriggerEnding(const TileMap& map, int tilesRevealed) const;

    void rememberTile(int tx, int ty, TileType type);
    bool hasMemoryOf(int tx, int ty) const;
    VoidMemory getMemory(int tx, int ty) const;
    float getMemoryStrength(int tx, int ty) const;

    void decayMemories(float dt);

    int getConsumedCount() const { return tilesConsumed; }
    float getVoidThreatLevel(const TileMap& map) const;

private:
    float shimmerTimer = 0;
    std::vector<VoidMemory> memories;

    int memoryKey(int tx, int ty) const { return ty * Constants::MAP_WIDTH + tx; }
};
