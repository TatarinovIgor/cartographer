#pragma once
#include "Constants.h"
#include "Renderer.h"
#include "Camera.h"
#include <vector>

struct Particle {
    float x, y;
    float vx, vy;
    float life, maxLife;
    float size;
    Constants::Colors::Color color;
    bool worldSpace;
};

class ParticleSystem {
public:
    void update(float dt);
    void render(Renderer& renderer, Camera& camera);

    void spawnInkMotes(float x, float y, int count);
    void spawnVoidShimmer(float x, float y);
    void spawnRevealBurst(float x, float y);
    void spawnDrawInk(float x, float y);

    void clear() { particles.clear(); }

private:
    std::vector<Particle> particles;
    float randFloat(float min, float max);
};
