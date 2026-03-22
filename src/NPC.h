#pragma once
#include "Constants.h"
#include "Renderer.h"
#include "Camera.h"
#include "DialogueSystem.h"
#include <string>
#include <vector>
#include <functional>

class NPC {
public:
    float x, y;
    std::string name;
    Constants::Colors::Color color;
    std::vector<DialogueNode> dialogue;
    bool talked = false;
    int inkReward = 0;

    NPC() = default;
    NPC(float x, float y, const std::string& name, Constants::Colors::Color color);

    void update(float dt);
    void render(Renderer& renderer, Camera& camera, float time, const AssetManager& assets);

    float centerX() const { return x + Constants::TILE_SIZE / 2.0f; }
    float centerY() const { return y + Constants::TILE_SIZE / 2.0f; }
    float distanceTo(float px, float py) const;
    bool inRange(float px, float py) const;

private:
    float animTimer = 0;
    float walkTimer = 0;
    int walkDir = 0; // 0=still, 1=right, -1=left
};
