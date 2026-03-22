#include "ParticleSystem.h"
#include <cstdlib>
#include <algorithm>

float ParticleSystem::randFloat(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

void ParticleSystem::update(float dt) {
    for (auto& p : particles) {
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.life -= dt;
        p.vy -= 5.0f * dt; // slight upward drift
    }
    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return p.life <= 0; }),
        particles.end()
    );
}

void ParticleSystem::render(Renderer& renderer, Camera& camera) {
    for (auto& p : particles) {
        float t = p.life / p.maxLife;
        unsigned char alpha = (unsigned char)(p.color.a * t);
        Constants::Colors::Color c = {p.color.r, p.color.g, p.color.b, alpha};
        float sz = p.size * (0.5f + 0.5f * t);

        float sx, sy;
        if (p.worldSpace) {
            sx = camera.screenX(p.x) + camera.shakeOffsetX();
            sy = camera.screenY(p.y) + camera.shakeOffsetY();
        } else {
            sx = p.x;
            sy = p.y;
        }
        renderer.fillRect(sx - sz/2, sy - sz/2, sz, sz, c);
    }
}

void ParticleSystem::spawnInkMotes(float x, float y, int count) {
    for (int i = 0; i < count; i++) {
        Particle p;
        p.x = x + randFloat(-20, 20);
        p.y = y + randFloat(-20, 20);
        p.vx = randFloat(-8, 8);
        p.vy = randFloat(-15, -5);
        p.life = p.maxLife = randFloat(1.0f, 3.0f);
        p.size = randFloat(2, 5);
        p.color = Constants::Colors::INK_DARK;
        p.color.a = 120;
        p.worldSpace = true;
        particles.push_back(p);
    }
}

void ParticleSystem::spawnVoidShimmer(float x, float y) {
    Particle p;
    p.x = x + randFloat(0, Constants::TILE_SIZE);
    p.y = y + randFloat(0, Constants::TILE_SIZE);
    p.vx = randFloat(-3, 3);
    p.vy = randFloat(-8, -2);
    p.life = p.maxLife = randFloat(0.5f, 1.5f);
    p.size = randFloat(1, 3);
    p.color = Constants::Colors::VOID_WHITE;
    p.color.a = 150;
    p.worldSpace = true;
    particles.push_back(p);
}

void ParticleSystem::spawnRevealBurst(float x, float y) {
    for (int i = 0; i < 15; i++) {
        Particle p;
        p.x = x + Constants::TILE_SIZE / 2.0f;
        p.y = y + Constants::TILE_SIZE / 2.0f;
        float angle = randFloat(0, 6.28f);
        float speed = randFloat(20, 60);
        p.vx = cos(angle) * speed;
        p.vy = sin(angle) * speed;
        p.life = p.maxLife = randFloat(0.3f, 0.8f);
        p.size = randFloat(3, 7);
        p.color = Constants::Colors::INK_SPREAD;
        p.worldSpace = true;
        particles.push_back(p);
    }
}

void ParticleSystem::spawnDrawInk(float x, float y) {
    for (int i = 0; i < 8; i++) {
        Particle p;
        p.x = x + randFloat(0, Constants::TILE_SIZE);
        p.y = y + randFloat(0, Constants::TILE_SIZE);
        p.vx = randFloat(-10, 10);
        p.vy = randFloat(-20, -5);
        p.life = p.maxLife = randFloat(0.4f, 1.0f);
        p.size = randFloat(2, 5);
        p.color = Constants::Colors::INK_DARK;
        p.color.a = 200;
        p.worldSpace = true;
        particles.push_back(p);
    }
}
