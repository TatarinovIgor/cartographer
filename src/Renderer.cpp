#include "Renderer.h"
#include <vector>

bool Renderer::init(SDL_Window* window) {
    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return true;
}

void Renderer::shutdown() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
}

void Renderer::clear(Constants::Colors::Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderClear(renderer);
}

void Renderer::present() {
    SDL_RenderPresent(renderer);
}

void Renderer::fillRect(float x, float y, float w, float h, Constants::Colors::Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_Rect r = {(int)x, (int)y, (int)w, (int)h};
    SDL_RenderFillRect(renderer, &r);
}

void Renderer::drawRect(float x, float y, float w, float h, Constants::Colors::Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_Rect r = {(int)x, (int)y, (int)w, (int)h};
    SDL_RenderDrawRect(renderer, &r);
}

void Renderer::fillRectAbsolute(int x, int y, int w, int h, Constants::Colors::Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_Rect r = {x, y, w, h};
    SDL_RenderFillRect(renderer, &r);
}

void Renderer::drawRectAbsolute(int x, int y, int w, int h, Constants::Colors::Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_Rect r = {x, y, w, h};
    SDL_RenderDrawRect(renderer, &r);
}

void Renderer::drawText(const std::string& text, int x, int y,
                         Constants::Colors::Color c, TTF_Font* font) {
    if (text.empty() || !font) return;
    SDL_Color sc = {c.r, c.g, c.b, c.a};
    SDL_Surface* surf = TTF_RenderUTF8_Blended(font, text.c_str(), sc);
    if (!surf) return;
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_Rect dst = {x, y, surf->w, surf->h};
    SDL_RenderCopy(renderer, tex, nullptr, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

void Renderer::drawTextCentered(const std::string& text, int y,
                                 Constants::Colors::Color c, TTF_Font* font) {
    if (text.empty() || !font) return;
    int w = 0, h = 0;
    TTF_SizeUTF8(font, text.c_str(), &w, &h);
    drawText(text, (Constants::SCREEN_WIDTH - w) / 2, y, c, font);
}

void Renderer::drawTextWrapped(const std::string& text, int x, int y, int maxW,
                                Constants::Colors::Color c, TTF_Font* font) {
    if (text.empty() || !font) return;

    std::vector<std::string> lines;
    std::string current;
    std::string word;

    for (size_t i = 0; i <= text.size(); i++) {
        char ch = (i < text.size()) ? text[i] : ' ';
        if (ch == '\n') {
            if (!word.empty()) {
                if (!current.empty()) current += ' ';
                current += word;
                word.clear();
            }
            lines.push_back(current);
            current.clear();
        } else if (ch == ' ') {
            if (!word.empty()) {
                std::string test = current.empty() ? word : current + ' ' + word;
                int tw = 0, th = 0;
                TTF_SizeUTF8(font, test.c_str(), &tw, &th);
                if (tw > maxW && !current.empty()) {
                    lines.push_back(current);
                    current = word;
                } else {
                    current = test;
                }
                word.clear();
            }
        } else {
            word += ch;
        }
    }
    if (!current.empty()) lines.push_back(current);

    int lineH = TTF_FontLineSkip(font);
    for (size_t i = 0; i < lines.size(); i++) {
        drawText(lines[i], x, y + (int)(i * lineH), c, font);
    }
}

void Renderer::setAlpha(float alpha) {
    (void)alpha;
}

void Renderer::resetAlpha() {
}
