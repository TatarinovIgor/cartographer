#include "VoidSystem.h"
#include <cstdlib>
#include <algorithm>
#include <cmath>

void VoidSystem::update(TileMap& map, ParticleSystem& particles, Camera& camera, float dt) {
    creepTimer += dt;
    shimmerTimer += dt;

    if (creepTimer >= Constants::VOID_CREEP_INTERVAL) {
        creepTimer = 0;
        creepOnce(map, particles, camera);
    }

    if (shimmerTimer > 0.3f) {
        shimmerTimer = 0;
        auto borders = map.getVoidBorderTiles();
        for (auto& [bx, by] : borders) {
            if (rand() % 4 == 0) {
                particles.spawnVoidShimmer(
                    bx * Constants::TILE_SIZE,
                    by * Constants::TILE_SIZE
                );
            }
        }
    }

    decayMemories(dt);
}

void VoidSystem::creepOnce(TileMap& map, ParticleSystem& particles, Camera& camera) {
    auto borders = map.getVoidBorderTiles();
    if (borders.empty()) return;

    int idx = rand() % borders.size();
    auto [bx, by] = borders[idx];

    int dirs[][2] = {{0,-1},{0,1},{-1,0},{1,0}};
    for (auto& d : dirs) {
        int nx = bx + d[0], ny = by + d[1];
        if (nx >= 0 && nx < map.width() && ny >= 0 && ny < map.height()) {
            TileType t = map.getTile(nx, ny);
            if (t != TileType::VOID && t != TileType::HIDDEN &&
                t != TileType::WALL && t != TileType::ROOF) {
                rememberTile(nx, ny, t);
                map.setTile(nx, ny, TileType::VOID);
                tilesConsumed++;
                camera.shake(2.0f, 0.3f);
                particles.spawnVoidShimmer(
                    nx * Constants::TILE_SIZE,
                    ny * Constants::TILE_SIZE
                );
                return;
            }
        }
    }
}

bool VoidSystem::shouldTriggerEnding(const TileMap& map, int tilesRevealed) const {
    return tilesRevealed >= 15;
}

void VoidSystem::rememberTile(int tx, int ty, TileType type) {
    for (auto& m : memories) {
        if (m.tx == tx && m.ty == ty) {
            m.originalType = type;
            m.memoryStrength = 1.0f;
            return;
        }
    }
    memories.push_back({tx, ty, type, 1.0f});
}

bool VoidSystem::hasMemoryOf(int tx, int ty) const {
    for (auto& m : memories)
        if (m.tx == tx && m.ty == ty && m.memoryStrength > 0.1f) return true;
    return false;
}

VoidMemory VoidSystem::getMemory(int tx, int ty) const {
    for (auto& m : memories)
        if (m.tx == tx && m.ty == ty) return m;
    return {tx, ty, TileType::GROUND, 0.0f};
}

float VoidSystem::getMemoryStrength(int tx, int ty) const {
    for (auto& m : memories)
        if (m.tx == tx && m.ty == ty) return m.memoryStrength;
    return 0.0f;
}

void VoidSystem::decayMemories(float dt) {
    for (auto& m : memories) {
        m.memoryStrength -= Constants::MEMORY_FADE_RATE * dt;
    }
    memories.erase(
        std::remove_if(memories.begin(), memories.end(),
            [](const VoidMemory& m) { return m.memoryStrength <= 0.0f; }),
        memories.end()
    );
}

float VoidSystem::getVoidThreatLevel(const TileMap& map) const {
    int voidCount = map.countVoidTiles();
    int totalTiles = map.width() * map.height();
    return (float)voidCount / totalTiles;
}
