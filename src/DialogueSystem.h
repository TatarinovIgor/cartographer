#pragma once
#include "Renderer.h"
#include "Input.h"
#include "AssetManager.h"
#include "Constants.h"
#include <string>
#include <vector>
#include <functional>

struct DialogueChoice {
    std::string text;
    int nextNode;
};

struct DialogueNode {
    std::string speaker;
    std::string text;
    Constants::Colors::Color speakerColor;
    std::vector<DialogueChoice> choices;
    int nextNode = -1; // -1 = end, or auto-advance to this node
    std::function<void()> onComplete;
};

class DialogueSystem {
public:
    void start(const std::vector<DialogueNode>& nodes);
    void update(const Input& input, float dt);
    void render(Renderer& renderer, const AssetManager& assets);

    bool isActive() const { return active; }

private:
    std::vector<DialogueNode> nodes;
    int currentNode = 0;
    bool active = false;

    std::string displayedText;
    float charTimer = 0;
    int charIndex = 0;
    bool textComplete = false;

    int selectedChoice = 0;

    void advanceText();
    void completeNode();
};
