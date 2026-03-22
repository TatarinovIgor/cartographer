#include "NPC.h"
#include <cmath>

NPC::NPC(float x, float y, const std::string& name, Constants::Colors::Color color)
    : x(x), y(y), name(name), color(color) {}

void NPC::update(float dt) {
    animTimer += dt;
    walkTimer += dt;

    // Simple pacing behavior
    if (walkTimer > 3.0f) {
        walkTimer = 0;
        walkDir = (walkDir == 0) ? 1 : (walkDir == 1) ? -1 : 0;
    }
    if (walkDir != 0) {
        x += walkDir * 20.0f * dt;
    }
}

void NPC::render(Renderer& renderer, Camera& camera, float time, const AssetManager& assets) {
    float sx = camera.screenX(x) + camera.shakeOffsetX();
    float sy = camera.screenY(y) + camera.shakeOffsetY();
    int ts = Constants::TILE_SIZE;

    float bob = sin(animTimer * 2.0f) * 1.5f;

    // Body
    renderer.fillRect(sx + 4, sy + ts * 0.15f + bob, ts - 8, ts * 0.7f, color);

    // Head
    renderer.fillRect(sx + ts * 0.25f, sy - ts * 0.1f + bob,
        ts * 0.5f, ts * 0.35f, color);

    // Eyes
    Constants::Colors::Color eyeColor = {20, 20, 20, 255};
    renderer.fillRect(sx + ts * 0.32f, sy + ts * 0.02f + bob, 3, 3, eyeColor);
    renderer.fillRect(sx + ts * 0.55f, sy + ts * 0.02f + bob, 3, 3, eyeColor);

    // Name tag above head
    TTF_Font* smallFont = assets.getFont("small");
    if (smallFont) {
        int tw = 0, th = 0;
        TTF_SizeUTF8(smallFont, name.c_str(), &tw, &th);
        renderer.drawText(name, (int)(sx + ts / 2 - tw / 2), (int)(sy - ts * 0.4f + bob),
            Constants::Colors::UI_TEXT, smallFont);
    }

    // Interaction indicator (floating "!")
    if (!talked) {
        float excBob = sin(time * 3.0f) * 4.0f;
        Constants::Colors::Color excColor = Constants::Colors::UI_HIGHLIGHT;
        TTF_Font* bodyFont = assets.getFont("body");
        if (bodyFont)
            renderer.drawText("!", (int)(sx + ts / 2 - 4), (int)(sy - ts * 0.65f + excBob),
                excColor, bodyFont);
    }
}

float NPC::distanceTo(float px, float py) const {
    float dx = centerX() - px;
    float dy = centerY() - py;
    return sqrt(dx * dx + dy * dy);
}

bool NPC::inRange(float px, float py) const {
    return distanceTo(px, py) < Constants::INTERACT_RANGE;
}
