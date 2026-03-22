#pragma once
#include <SDL.h>
#include <unordered_map>

class Input {
public:
    void update();
    bool isKeyDown(SDL_Scancode key) const;
    bool isKeyPressed(SDL_Scancode key) const;
    bool isMouseDown(int button) const;
    bool isMousePressed(int button) const;
    int mouseX() const { return mx; }
    int mouseY() const { return my; }
    bool shouldQuit() const { return quit; }

private:
    std::unordered_map<SDL_Scancode, bool> currentKeys;
    std::unordered_map<SDL_Scancode, bool> previousKeys;
    std::unordered_map<int, bool> currentMouse;
    std::unordered_map<int, bool> previousMouse;
    int mx = 0, my = 0;
    bool quit = false;
};
