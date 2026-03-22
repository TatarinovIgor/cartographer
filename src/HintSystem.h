#pragma once
#include "Constants.h"
#include "Renderer.h"
#include "AssetManager.h"
#include <string>
#include <vector>

struct Hint {
    std::string id;
    std::string text;
    float displayTime;
    float fadeInTime = 0.5f;
    float fadeOutTime = 1.0f;
    bool dismissed = false;
};

class HintSystem {
public:
    void update(float dt);
    void render(Renderer& renderer, const AssetManager& assets);

    void show(const std::string& id, const std::string& text, float duration = 5.0f);
    void showOnce(const std::string& id, const std::string& text, float duration = 5.0f);
    void dismiss(const std::string& id);
    void clear();

    bool wasShown(const std::string& id) const;

private:
    struct ActiveHint {
        std::string id;
        std::string text;
        float timer;
        float maxTime;
        float fadeIn;
        float fadeOut;
    };

    std::vector<ActiveHint> activeHints;
    std::vector<std::string> shownIds;

    float getAlpha(const ActiveHint& h) const;
};
