#include "AssetManager.h"
#include <SDL.h>
#include <cstdio>

bool AssetManager::init() {
    if (TTF_Init() < 0) {
        SDL_Log("TTF_Init failed: %s", TTF_GetError());
        return false;
    }
    return loadEmbeddedFonts();
}

void AssetManager::shutdown() {
    for (auto& [name, font] : fonts) {
        if (font) TTF_CloseFont(font);
    }
    fonts.clear();
    TTF_Quit();
}

TTF_Font* AssetManager::getFont(const std::string& name) const {
    auto it = fonts.find(name);
    return (it != fonts.end()) ? it->second : nullptr;
}

bool AssetManager::loadEmbeddedFonts() {
    // Try common system font paths across platforms
    const char* fontPaths[] = {
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/SFNSMono.ttf",
        nullptr
    };

    const char* foundPath = nullptr;
    for (int i = 0; fontPaths[i]; i++) {
        FILE* f = fopen(fontPaths[i], "rb");
        if (f) {
            fclose(f);
            foundPath = fontPaths[i];
            break;
        }
    }

    if (!foundPath) {
        SDL_Log("No system font found! Text rendering will be disabled.");
        return true;
    }

    SDL_Log("Using font: %s", foundPath);

    fonts["title"] = TTF_OpenFont(foundPath, 36);
    fonts["body"] = TTF_OpenFont(foundPath, 18);
    fonts["small"] = TTF_OpenFont(foundPath, 14);
    fonts["dialogue"] = TTF_OpenFont(foundPath, 20);
    fonts["ui"] = TTF_OpenFont(foundPath, 16);

    for (auto& [name, font] : fonts) {
        if (!font) {
            SDL_Log("Failed to load font '%s': %s", name.c_str(), TTF_GetError());
            return false;
        }
    }

    return true;
}
