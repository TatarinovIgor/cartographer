#include "Renderer.h"
#include <vector>
#include <cmath>

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

void Renderer::drawLine(float x1, float y1, float x2, float y2, Constants::Colors::Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderDrawLine(renderer, (int)x1, (int)y1, (int)x2, (int)y2);
}

void Renderer::drawLineThick(float x1, float y1, float x2, float y2,
                               int thickness, Constants::Colors::Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    for (int i = -thickness/2; i <= thickness/2; i++) {
        SDL_RenderDrawLine(renderer, (int)x1, (int)y1 + i, (int)x2, (int)y2 + i);
        SDL_RenderDrawLine(renderer, (int)x1 + i, (int)y1, (int)x2 + i, (int)y2);
    }
}

void Renderer::drawCircle(float cx, float cy, float radius, Constants::Colors::Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    int x = (int)radius, y = 0, err = 1 - x;
    while (x >= y) {
        SDL_RenderDrawPoint(renderer, (int)cx + x, (int)cy + y);
        SDL_RenderDrawPoint(renderer, (int)cx - x, (int)cy + y);
        SDL_RenderDrawPoint(renderer, (int)cx + x, (int)cy - y);
        SDL_RenderDrawPoint(renderer, (int)cx - x, (int)cy - y);
        SDL_RenderDrawPoint(renderer, (int)cx + y, (int)cy + x);
        SDL_RenderDrawPoint(renderer, (int)cx - y, (int)cy + x);
        SDL_RenderDrawPoint(renderer, (int)cx + y, (int)cy - x);
        SDL_RenderDrawPoint(renderer, (int)cx - y, (int)cy - x);
        y++;
        if (err < 0) { err += 2 * y + 1; }
        else { x--; err += 2 * (y - x) + 1; }
    }
}

void Renderer::fillCircle(float cx, float cy, float radius, Constants::Colors::Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    for (int dy = -(int)radius; dy <= (int)radius; dy++) {
        int dx = (int)sqrt(radius * radius - dy * dy);
        SDL_RenderDrawLine(renderer, (int)cx - dx, (int)cy + dy, (int)cx + dx, (int)cy + dy);
    }
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
