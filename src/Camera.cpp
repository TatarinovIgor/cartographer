#include "Camera.h"
#include <cstdlib>
#include <algorithm>

void Camera::follow(float targetX, float targetY, float dt) {
    float goalX = targetX - Constants::SCREEN_WIDTH / 2.0f;
    float goalY = targetY - Constants::SCREEN_HEIGHT / 2.0f;

    float worldW = Constants::MAP_WIDTH * Constants::TILE_SIZE;
    float worldH = Constants::MAP_HEIGHT * Constants::TILE_SIZE;
    goalX = std::clamp(goalX, 0.0f, std::max(0.0f, worldW - Constants::SCREEN_WIDTH));
    goalY = std::clamp(goalY, 0.0f, std::max(0.0f, worldH - Constants::SCREEN_HEIGHT));

    float lerp = Constants::CAMERA_LERP * dt;
    x += (goalX - x) * lerp;
    y += (goalY - y) * lerp;
}

void Camera::shake(float intensity, float duration) {
    shakeIntensity = intensity;
    shakeDuration = duration;
    shakeTimer = duration;
}

void Camera::updateShake(float dt) {
    if (shakeTimer > 0) {
        shakeTimer -= dt;
        float t = shakeTimer / shakeDuration;
        float mag = shakeIntensity * t;
        shakeOX = (float)(rand() % 100 - 50) / 50.0f * mag;
        shakeOY = (float)(rand() % 100 - 50) / 50.0f * mag;
    } else {
        shakeOX = 0;
        shakeOY = 0;
    }
}
