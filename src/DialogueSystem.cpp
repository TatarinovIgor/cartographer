#include "DialogueSystem.h"
#include <algorithm>

void DialogueSystem::start(const std::vector<DialogueNode>& dialogueNodes) {
    nodes = dialogueNodes;
    currentNode = 0;
    active = true;
    charIndex = 0;
    charTimer = 0;
    displayedText = "";
    textComplete = false;
    selectedChoice = 0;
}

void DialogueSystem::update(const Input& input, float dt) {
    if (!active || currentNode < 0 || currentNode >= (int)nodes.size()) {
        active = false;
        return;
    }

    auto& node = nodes[currentNode];

    if (!textComplete) {
        charTimer += dt;
        while (charTimer >= Constants::TYPEWRITER_SPEED && charIndex < (int)node.text.size()) {
            charTimer -= Constants::TYPEWRITER_SPEED;
            displayedText += node.text[charIndex];
            charIndex++;
        }
        if (charIndex >= (int)node.text.size())
            textComplete = true;

        // Skip typewriter with Space/Enter
        if (input.isKeyPressed(SDL_SCANCODE_SPACE) || input.isKeyPressed(SDL_SCANCODE_RETURN)) {
            displayedText = node.text;
            charIndex = (int)node.text.size();
            textComplete = true;
        }
    } else {
        if (!node.choices.empty()) {
            if (input.isKeyPressed(SDL_SCANCODE_W) || input.isKeyPressed(SDL_SCANCODE_UP))
                selectedChoice = std::max(0, selectedChoice - 1);
            if (input.isKeyPressed(SDL_SCANCODE_S) || input.isKeyPressed(SDL_SCANCODE_DOWN))
                selectedChoice = std::min((int)node.choices.size() - 1, selectedChoice + 1);

            if (input.isKeyPressed(SDL_SCANCODE_SPACE) || input.isKeyPressed(SDL_SCANCODE_RETURN)) {
                int next = node.choices[selectedChoice].nextNode;
                completeNode();
                if (next >= 0 && next < (int)nodes.size()) {
                    currentNode = next;
                    charIndex = 0;
                    charTimer = 0;
                    displayedText = "";
                    textComplete = false;
                    selectedChoice = 0;
                } else {
                    active = false;
                }
            }
        } else {
            if (input.isKeyPressed(SDL_SCANCODE_SPACE) || input.isKeyPressed(SDL_SCANCODE_RETURN)) {
                int next = node.nextNode;
                completeNode();
                if (next >= 0 && next < (int)nodes.size()) {
                    currentNode = next;
                    charIndex = 0;
                    charTimer = 0;
                    displayedText = "";
                    textComplete = false;
                    selectedChoice = 0;
                } else {
                    active = false;
                }
            }
        }
    }
}

void DialogueSystem::completeNode() {
    if (currentNode >= 0 && currentNode < (int)nodes.size()) {
        auto& node = nodes[currentNode];
        if (node.onComplete) node.onComplete();
    }
}

void DialogueSystem::render(Renderer& renderer, const AssetManager& assets) {
    if (!active || currentNode < 0 || currentNode >= (int)nodes.size()) return;

    auto& node = nodes[currentNode];
    TTF_Font* dialogueFont = assets.getFont("dialogue");
    TTF_Font* smallFont = assets.getFont("small");
    if (!dialogueFont) return;

    int boxH = Constants::DIALOGUE_BOX_HEIGHT;
    int boxY = Constants::SCREEN_HEIGHT - boxH - 10;
    int boxX = 40;
    int boxW = Constants::SCREEN_WIDTH - 80;

    // Background
    renderer.fillRectAbsolute(boxX, boxY, boxW, boxH, Constants::Colors::UI_BG);
    renderer.drawRectAbsolute(boxX, boxY, boxW, boxH, Constants::Colors::UI_HIGHLIGHT);

    // Speaker name
    if (!node.speaker.empty()) {
        renderer.fillRectAbsolute(boxX + 10, boxY - 18, (int)node.speaker.size() * 12 + 20, 26,
            Constants::Colors::UI_BG);
        renderer.drawRectAbsolute(boxX + 10, boxY - 18, (int)node.speaker.size() * 12 + 20, 26,
            node.speakerColor);

        TTF_Font* uiFont = assets.getFont("ui");
        if (uiFont)
            renderer.drawText(node.speaker, boxX + 20, boxY - 15, node.speakerColor, uiFont);
    }

    // Dialogue text (wrapped)
    renderer.drawTextWrapped(displayedText, boxX + 20, boxY + 15, boxW - 40,
        Constants::Colors::UI_TEXT, dialogueFont);

    // Choices
    if (textComplete && !node.choices.empty() && smallFont) {
        int choiceY = boxY + boxH - 10 - (int)node.choices.size() * 22;
        for (int i = 0; i < (int)node.choices.size(); i++) {
            auto color = (i == selectedChoice)
                ? Constants::Colors::UI_HIGHLIGHT
                : Constants::Colors::UI_TEXT;
            std::string prefix = (i == selectedChoice) ? "> " : "  ";
            renderer.drawText(prefix + node.choices[i].text,
                boxX + boxW - 300, choiceY + i * 22, color, smallFont);
        }
    }

    // Continue indicator
    if (textComplete && node.choices.empty() && smallFont) {
        renderer.drawText("[Space/Enter]", boxX + boxW - 130, boxY + boxH - 22,
            Constants::Colors::UI_HIGHLIGHT, smallFont);
    }
}
