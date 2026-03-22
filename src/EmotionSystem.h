#pragma once
#include "Constants.h"
#include "Renderer.h"
#include "AssetManager.h"
#include <string>

enum class Emotion {
    CALM,
    FEARFUL,
    GRIEVING,
    HOPEFUL,
    DETERMINED
};

class EmotionSystem {
public:
    Emotion current = Emotion::CALM;
    float intensity = 0.0f;

    void update(float dt);
    void renderOverlay(Renderer& renderer, float time);
    void renderIndicator(Renderer& renderer, const AssetManager& assets);

    void trigger(Emotion emotion, float strength);
    void decay(float dt);

    float getShakiness() const;
    float getSizeDistortion() const;
    float getLineWobble() const;
    Constants::Colors::Color getTintColor() const;
    std::string getEmotionName() const;

    float applyToStrokeX(float x, float time) const;
    float applyToStrokeY(float y, float time) const;
};
