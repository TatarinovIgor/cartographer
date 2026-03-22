#pragma once
#include <SDL_ttf.h>
#include <string>
#include <unordered_map>

class AssetManager {
public:
    bool init();
    void shutdown();

    TTF_Font* getFont(const std::string& name) const;

private:
    std::unordered_map<std::string, TTF_Font*> fonts;
    bool loadEmbeddedFonts();
};
