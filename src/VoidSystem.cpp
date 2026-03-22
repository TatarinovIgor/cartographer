#include "VoidSystem.h"
#include <cstdlib>

void VoidSystem::update(TileMap& map, ParticleSystem& particles, Camera& camera, float dt) {
    creepTimer += dt;
    shimmerTimer += dt;

    // Void slowly creeps inward
    if (creepTimer >= Constants::VOID_CREEP_INTERVAL) {
        creepTimer = 0;
        creepOnce(map, particles, camera);
    }

    // Spawn shimmer particles on void border tiles
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
}

void VoidSystem::creepOnce(TileMap& map, ParticleSystem& particles, Camera& camera) {
    auto borders = map.getVoidBorderTiles();
    if (borders.empty()) return;

    // Pick a random border tile to consume
    int idx = rand() % borders.size();
    auto [bx, by] = borders[idx];

    // Find an adjacent non-void tile to consume
    int dirs[][2] = {{0,-1},{0,1},{-1,0},{1,0}};
    for (auto& d : dirs) {
        int nx = bx + d[0], ny = by + d[1];
        if (nx >= 0 && nx < map.width() && ny >= 0 && ny < map.height()) {
            TileType t = map.getTile(nx, ny);
            if (t != TileType::VOID && t != TileType::HIDDEN &&
                t != TileType::WALL && t != TileType::ROOF) {
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
