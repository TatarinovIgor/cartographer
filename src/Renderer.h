#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include "Constants.h"

class Renderer {
public:
    bool init(SDL_Window* window);
    void shutdown();

    void clear(Constants::Colors::Color c);
    void present();

    void fillRect(float x, float y, float w, float h, Constants::Colors::Color c);
    void drawRect(float x, float y, float w, float h, Constants::Colors::Color c);
    void fillRectAbsolute(int x, int y, int w, int h, Constants::Colors::Color c);
    void drawRectAbsolute(int x, int y, int w, int h, Constants::Colors::Color c);

    void drawText(const std::string& text, int x, int y, Constants::Colors::Color c, TTF_Font* font);
    void drawTextCentered(const std::string& text, int y, Constants::Colors::Color c, TTF_Font* font);
    void drawTextWrapped(const std::string& text, int x, int y, int maxW, Constants::Colors::Color c, TTF_Font* font);

    void setAlpha(float alpha);
    void resetAlpha();

    SDL_Renderer* raw() { return renderer; }

private:
    SDL_Renderer* renderer = nullptr;
};
