#include "Input.h"

void Input::update() {
    previousKeys = currentKeys;
    previousMouse = currentMouse;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                if (!e.key.repeat)
                    currentKeys[e.key.keysym.scancode] = true;
                break;
            case SDL_KEYUP:
                currentKeys[e.key.keysym.scancode] = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                currentMouse[e.button.button] = true;
                break;
            case SDL_MOUSEBUTTONUP:
                currentMouse[e.button.button] = false;
                break;
            case SDL_MOUSEMOTION:
                mx = e.motion.x;
                my = e.motion.y;
                break;
        }
    }
}

bool Input::isKeyDown(SDL_Scancode key) const {
    auto it = currentKeys.find(key);
    return it != currentKeys.end() && it->second;
}

bool Input::isKeyPressed(SDL_Scancode key) const {
    auto curr = currentKeys.find(key);
    auto prev = previousKeys.find(key);
    bool now = curr != currentKeys.end() && curr->second;
    bool was = prev != previousKeys.end() && prev->second;
    return now && !was;
}

bool Input::isMouseDown(int button) const {
    auto it = currentMouse.find(button);
    return it != currentMouse.end() && it->second;
}

bool Input::isMousePressed(int button) const {
    auto curr = currentMouse.find(button);
    auto prev = previousMouse.find(button);
    bool now = curr != currentMouse.end() && curr->second;
    bool was = prev != previousMouse.end() && prev->second;
    return now && !was;
}
