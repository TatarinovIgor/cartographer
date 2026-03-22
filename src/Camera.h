#pragma once
#include "Constants.h"

class Camera {
public:
    float x = 0, y = 0;

    void follow(float targetX, float targetY, float dt);
    float screenX(float worldX) const { return worldX - x; }
    float screenY(float worldY) const { return worldY - y; }

    void shake(float intensity, float duration);
    void updateShake(float dt);
    float shakeOffsetX() const { return shakeOX; }
    float shakeOffsetY() const { return shakeOY; }

private:
    float shakeIntensity = 0;
    float shakeDuration = 0;
    float shakeTimer = 0;
    float shakeOX = 0, shakeOY = 0;
};
