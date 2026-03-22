#include "MappingSystem.h"
#include <cmath>
#include <algorithm>

void MappingSystem::enter(int playerTX, int playerTY) {
    active = true;
    cursorTX = playerTX;
    cursorTY = playerTY;
    cursorBlink = 0;
    moveCooldown = 0;
}

void MappingSystem::exit() {
    active = false;
}

void MappingSystem::update(const Input& input, TileMap& map, Player& player,
                            ParticleSystem& particles, Camera& camera, float dt) {
    if (!active) return;

    cursorBlink += dt;
    moveCooldown -= dt;

    // Exit mapping mode
    if (input.isKeyPressed(SDL_SCANCODE_M) || input.isKeyPressed(SDL_SCANCODE_TAB) ||
        input.isKeyPressed(SDL_SCANCODE_ESCAPE)) {
        exit();
        return;
    }

    // Move cursor
    if (moveCooldown <= 0) {
        int dx = 0, dy = 0;
        if (input.isKeyDown(SDL_SCANCODE_W) || input.isKeyDown(SDL_SCANCODE_UP))    dy = -1;
        if (input.isKeyDown(SDL_SCANCODE_S) || input.isKeyDown(SDL_SCANCODE_DOWN))  dy = 1;
        if (input.isKeyDown(SDL_SCANCODE_A) || input.isKeyDown(SDL_SCANCODE_LEFT))  dx = -1;
        if (input.isKeyDown(SDL_SCANCODE_D) || input.isKeyDown(SDL_SCANCODE_RIGHT)) dx = 1;

        if (dx != 0 || dy != 0) {
            int nx = cursorTX + dx;
            int ny = cursorTY + dy;
            if (nx >= 0 && nx < map.width() && ny >= 0 && ny < map.height()) {
                cursorTX = nx;
                cursorTY = ny;
                moveCooldown = 0.12f;
            }
        }
    }

    // Draw/reveal tile
    if (input.isKeyPressed(SDL_SCANCODE_SPACE) || input.isKeyPressed(SDL_SCANCODE_RETURN)) {
        TileType current = map.getTile(cursorTX, cursorTY);
        if (current == TileType::VOID || current == TileType::HIDDEN) {
            if (player.useInk(5)) {
                TileType revealTo = map.getHiddenReveal(cursorTX, cursorTY);
                RevealAnim anim;
                anim.tx = cursorTX;
                anim.ty = cursorTY;
                anim.progress = 0;
                anim.revealTo = revealTo;
                revealAnims.push_back(anim);

                map.setTile(cursorTX, cursorTY, revealTo);
                tilesRevealed++;

                float wx = cursorTX * Constants::TILE_SIZE;
                float wy = cursorTY * Constants::TILE_SIZE;
                particles.spawnRevealBurst(wx, wy);
                particles.spawnDrawInk(wx, wy);

                camera.shake(3.0f, 0.2f);
            }
        }
    }

    // Update reveal animations
    for (auto& anim : revealAnims)
        anim.progress += dt * 3.0f;
    revealAnims.erase(
        std::remove_if(revealAnims.begin(), revealAnims.end(),
            [](const RevealAnim& a) { return a.progress >= 1.0f; }),
        revealAnims.end()
    );
}

void MappingSystem::render(Renderer& renderer, Camera& camera,
                            const AssetManager& assets, float time) {
    if (!active) return;

    // Parchment overlay
    renderer.fillRectAbsolute(0, 0, Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT,
        Constants::Colors::PARCHMENT);

    // Cursor
    float blink = sin(cursorBlink * 4.0f) * 0.3f + 0.7f;
    float sx = camera.screenX(cursorTX * Constants::TILE_SIZE) + camera.shakeOffsetX();
    float sy = camera.screenY(cursorTY * Constants::TILE_SIZE) + camera.shakeOffsetY();
    int ts = Constants::TILE_SIZE;

    Constants::Colors::Color cursorColor = {
        (unsigned char)(Constants::Colors::UI_HIGHLIGHT.r * blink),
        (unsigned char)(Constants::Colors::UI_HIGHLIGHT.g * blink),
        (unsigned char)(Constants::Colors::UI_HIGHLIGHT.b * blink),
        200
    };
    renderer.drawRect(sx - 2, sy - 2, ts + 4, ts + 4, cursorColor);
    renderer.drawRect(sx - 1, sy - 1, ts + 2, ts + 2, cursorColor);

    // Reveal animations (ink spreading effect)
    for (auto& anim : revealAnims) {
        float ax = camera.screenX(anim.tx * Constants::TILE_SIZE) + camera.shakeOffsetX();
        float ay = camera.screenY(anim.ty * Constants::TILE_SIZE) + camera.shakeOffsetY();
        float p = std::min(anim.progress, 1.0f);
        float sz = ts * p;
        float ox = (ts - sz) / 2.0f;
        Constants::Colors::Color inkColor = Constants::Colors::INK_SPREAD;
        inkColor.a = (unsigned char)(200 * (1.0f - p));
        renderer.fillRect(ax + ox, ay + ox, sz, sz, inkColor);
    }

    // Mapping mode UI label
    TTF_Font* uiFont = assets.getFont("ui");
    if (uiFont) {
        renderer.fillRectAbsolute(Constants::SCREEN_WIDTH / 2 - 80, 8, 160, 28,
            Constants::Colors::UI_BG);
        renderer.drawTextCentered("MAPPING MODE", 12, Constants::Colors::UI_HIGHLIGHT, uiFont);
    }

    // Controls hint
    TTF_Font* smallFont = assets.getFont("small");
    if (smallFont) {
        renderer.drawText("[WASD] Move  [Space] Draw  [M/Tab] Exit",
            10, Constants::SCREEN_HEIGHT - 25, Constants::Colors::UI_TEXT, smallFont);
    }
}

void MappingSystem::renderMinimap(Renderer& renderer, const TileMap& map, const Player& player) {
    int mmW = 120, mmH = 90;
    int mmX = Constants::SCREEN_WIDTH - mmW - 15;
    int mmY = 15;

    renderer.fillRectAbsolute(mmX - 2, mmY - 2, mmW + 4, mmH + 4, Constants::Colors::UI_BG);
    renderer.drawRectAbsolute(mmX - 2, mmY - 2, mmW + 4, mmH + 4, Constants::Colors::UI_HIGHLIGHT);

    float scaleX = (float)mmW / map.width();
    float scaleY = (float)mmH / map.height();

    for (int x = 0; x < map.width(); x++) {
        for (int y = 0; y < map.height(); y++) {
            Constants::Colors::Color c;
            TileType t = map.getTile(x, y);
            switch (t) {
                case TileType::VOID:     c = Constants::Colors::VOID_WHITE; break;
                case TileType::HIDDEN:   c = Constants::Colors::HIDDEN; break;
                case TileType::WATER:    c = Constants::Colors::WATER; break;
                case TileType::WALL:
                case TileType::WALL_DARK:
                case TileType::ROOF:     c = Constants::Colors::WALL; break;
                case TileType::PATH:     c = Constants::Colors::PATH; break;
                default:                 c = Constants::Colors::GROUND; break;
            }
            int px = mmX + (int)(x * scaleX);
            int py = mmY + (int)(y * scaleY);
            renderer.fillRectAbsolute(px, py,
                std::max(1, (int)scaleX), std::max(1, (int)scaleY), c);
        }
    }

    // Player dot on minimap
    int ppx = mmX + (int)(player.tileX() * scaleX);
    int ppy = mmY + (int)(player.tileY() * scaleY);
    renderer.fillRectAbsolute(ppx - 1, ppy - 1, 3, 3, Constants::Colors::SABLE);
}
