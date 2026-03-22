#include "HintSystem.h"
#include <algorithm>

void HintSystem::update(float dt) {
    for (auto& h : activeHints)
        h.timer += dt;

    activeHints.erase(
        std::remove_if(activeHints.begin(), activeHints.end(),
            [](const ActiveHint& h) { return h.timer >= h.maxTime; }),
        activeHints.end()
    );
}

void HintSystem::render(Renderer& renderer, const AssetManager& assets) {
    TTF_Font* font = assets.getFont("dialogue");
    TTF_Font* smallFont = assets.getFont("small");
    if (!font) return;

    int y = Constants::SCREEN_HEIGHT - 80;

    for (int i = (int)activeHints.size() - 1; i >= 0; i--) {
        auto& h = activeHints[i];
        float alpha = getAlpha(h);
        if (alpha < 0.01f) continue;

        int textW = 0, textH = 0;
        TTF_SizeUTF8(font, h.text.c_str(), &textW, &textH);

        int boxW = std::min(textW + 40, Constants::SCREEN_WIDTH - 100);
        int boxH = textH + 20;
        int boxX = (Constants::SCREEN_WIDTH - boxW) / 2;
        int boxY = y - boxH;

        Constants::Colors::Color bg = {15, 12, 10, (unsigned char)(210 * alpha)};
        renderer.fillRectAbsolute(boxX, boxY, boxW, boxH, bg);

        Constants::Colors::Color border = {210, 170, 60, (unsigned char)(120 * alpha)};
        renderer.drawRectAbsolute(boxX, boxY, boxW, boxH, border);

        Constants::Colors::Color textColor = {220, 210, 195, (unsigned char)(255 * alpha)};
        renderer.drawTextWrapped(h.text, boxX + 20, boxY + 10, boxW - 40, textColor, font);

        y = boxY - 8;
    }
}

void HintSystem::show(const std::string& id, const std::string& text, float duration) {
    for (auto& h : activeHints) {
        if (h.id == id) {
            h.timer = 0;
            h.text = text;
            h.maxTime = duration;
            return;
        }
    }
    activeHints.push_back({id, text, 0, duration, 0.5f, 1.0f});
}

void HintSystem::showOnce(const std::string& id, const std::string& text, float duration) {
    if (wasShown(id)) return;
    shownIds.push_back(id);
    show(id, text, duration);
}

void HintSystem::dismiss(const std::string& id) {
    activeHints.erase(
        std::remove_if(activeHints.begin(), activeHints.end(),
            [&](const ActiveHint& h) { return h.id == id; }),
        activeHints.end()
    );
}

void HintSystem::clear() {
    activeHints.clear();
}

bool HintSystem::wasShown(const std::string& id) const {
    for (auto& s : shownIds)
        if (s == id) return true;
    return false;
}

float HintSystem::getAlpha(const ActiveHint& h) const {
    if (h.timer < h.fadeIn)
        return h.timer / h.fadeIn;
    float remaining = h.maxTime - h.timer;
    if (remaining < h.fadeOut)
        return remaining / h.fadeOut;
    return 1.0f;
}
