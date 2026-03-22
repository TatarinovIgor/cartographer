#include "EmotionSystem.h"
#include <cmath>
#include <algorithm>

void EmotionSystem::update(float dt) {
    decay(dt);
}

void EmotionSystem::trigger(Emotion emotion, float strength) {
    current = emotion;
    intensity = std::clamp(intensity + strength, 0.0f, 1.0f);
}

void EmotionSystem::decay(float dt) {
    intensity -= 0.03f * dt;
    if (intensity <= 0.0f) {
        intensity = 0.0f;
        current = Emotion::CALM;
    }
}

float EmotionSystem::getShakiness() const {
    switch (current) {
        case Emotion::FEARFUL:    return intensity * 4.0f;
        case Emotion::GRIEVING:   return intensity * 1.5f;
        case Emotion::DETERMINED: return intensity * 0.5f;
        default: return 0.0f;
    }
}

float EmotionSystem::getSizeDistortion() const {
    switch (current) {
        case Emotion::GRIEVING:   return 1.0f - intensity * 0.3f;
        case Emotion::FEARFUL:    return 1.0f - intensity * 0.15f;
        case Emotion::HOPEFUL:    return 1.0f + intensity * 0.15f;
        case Emotion::DETERMINED: return 1.0f;
        default: return 1.0f;
    }
}

float EmotionSystem::getLineWobble() const {
    switch (current) {
        case Emotion::FEARFUL:    return intensity * 6.0f;
        case Emotion::GRIEVING:   return intensity * 2.0f;
        case Emotion::HOPEFUL:    return intensity * 1.0f;
        default: return 0.0f;
    }
}

Constants::Colors::Color EmotionSystem::getTintColor() const {
    switch (current) {
        case Emotion::FEARFUL:    return Constants::Colors::EMOTION_FEAR;
        case Emotion::GRIEVING:   return Constants::Colors::EMOTION_GRIEF;
        case Emotion::HOPEFUL:    return Constants::Colors::EMOTION_HOPE;
        default: return {0, 0, 0, 0};
    }
}

std::string EmotionSystem::getEmotionName() const {
    switch (current) {
        case Emotion::FEARFUL:    return "Fearful";
        case Emotion::GRIEVING:   return "Grieving";
        case Emotion::HOPEFUL:    return "Hopeful";
        case Emotion::DETERMINED: return "Determined";
        default: return "Calm";
    }
}

float EmotionSystem::applyToStrokeX(float x, float time) const {
    float shake = getShakiness();
    float wobble = getLineWobble();
    return x + sin(time * 12.0f + x * 0.1f) * shake
             + sin(time * 5.0f + x * 0.3f) * wobble;
}

float EmotionSystem::applyToStrokeY(float y, float time) const {
    float shake = getShakiness();
    float wobble = getLineWobble();
    return y + cos(time * 11.0f + y * 0.1f) * shake
             + cos(time * 4.5f + y * 0.25f) * wobble;
}

void EmotionSystem::renderOverlay(Renderer& renderer, float time) {
    if (intensity < 0.05f) return;
    auto tint = getTintColor();
    tint.a = (unsigned char)(tint.a * intensity * 0.4f);
    renderer.fillRectAbsolute(0, 0, Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT, tint);
}

void EmotionSystem::renderIndicator(Renderer& renderer, const AssetManager& assets) {
    if (intensity < 0.05f) return;
    TTF_Font* font = assets.getFont("small");
    if (!font) return;

    std::string text = getEmotionName();
    auto color = getTintColor();
    color.a = (unsigned char)(200 * intensity);

    int barW = (int)(80 * intensity);
    renderer.fillRectAbsolute(15, 42, barW, 4, color);
    renderer.drawText(text, 15, 46, color, font);
}
