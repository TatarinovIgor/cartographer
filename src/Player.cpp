#include "Player.h"
#include <cmath>
#include <algorithm>

void Player::init(float startX, float startY) {
    x = startX;
    y = startY;
    ink = Constants::MAX_INK;
}

void Player::update(const Input& input, const TileMap& map, float dt) {
    float dx = 0, dy = 0;
    if (input.isKeyDown(SDL_SCANCODE_W) || input.isKeyDown(SDL_SCANCODE_UP))    dy -= 1;
    if (input.isKeyDown(SDL_SCANCODE_S) || input.isKeyDown(SDL_SCANCODE_DOWN))  dy += 1;
    if (input.isKeyDown(SDL_SCANCODE_A) || input.isKeyDown(SDL_SCANCODE_LEFT))  dx -= 1;
    if (input.isKeyDown(SDL_SCANCODE_D) || input.isKeyDown(SDL_SCANCODE_RIGHT)) dx += 1;

    moving = (dx != 0 || dy != 0);
    if (moving) {
        // Normalize diagonal movement
        float len = sqrt(dx * dx + dy * dy);
        dx /= len;
        dy /= len;

        facingX = (int)dx;
        facingY = (int)dy;

        float speed = Constants::PLAYER_SPEED * dt;
        float nx = x + dx * speed;
        float ny = y + dy * speed;

        // Try horizontal then vertical separately for wall sliding
        if (canMove(map, nx, y))
            x = nx;
        if (canMove(map, x, ny))
            y = ny;

        // Clamp to world bounds
        float maxX = (Constants::MAP_WIDTH - 1) * Constants::TILE_SIZE;
        float maxY = (Constants::MAP_HEIGHT - 1) * Constants::TILE_SIZE;
        x = std::clamp(x, 0.0f, maxX);
        y = std::clamp(y, 0.0f, maxY);
    }

    animTimer += dt;
}

void Player::render(Renderer& renderer, Camera& camera, float time) {
    float sx = camera.screenX(x) + camera.shakeOffsetX();
    float sy = camera.screenY(y) + camera.shakeOffsetY();
    int ts = Constants::TILE_SIZE;

    // Sable's cloak (slightly larger, behind)
    float bobY = moving ? sin(animTimer * 8.0f) * 2.0f : 0;
    renderer.fillRect(sx - 2, sy + ts * 0.2f + bobY,
        ts + 4, ts * 0.75f, Constants::Colors::SABLE_CLOAK);

    // Sable's body
    renderer.fillRect(sx + 2, sy + ts * 0.1f + bobY,
        ts - 4, ts * 0.7f, Constants::Colors::SABLE);

    // Head
    renderer.fillRect(sx + ts * 0.25f, sy - ts * 0.15f + bobY,
        ts * 0.5f, ts * 0.35f, Constants::Colors::SABLE);

    // Eyes (facing direction indicator)
    float eyeOffX = facingX * 3.0f;
    float eyeOffY = facingY * 2.0f;
    Constants::Colors::Color eyeColor = {30, 25, 20, 255};
    renderer.fillRect(sx + ts * 0.32f + eyeOffX, sy - ts * 0.02f + eyeOffY + bobY,
        3, 3, eyeColor);
    renderer.fillRect(sx + ts * 0.55f + eyeOffX, sy - ts * 0.02f + eyeOffY + bobY,
        3, 3, eyeColor);

    // Ink glow effect around Sable
    float glowPulse = sin(time * 2.0f) * 0.3f + 0.5f;
    Constants::Colors::Color glow = {210, 170, 60, (unsigned char)(40 * glowPulse)};
    renderer.fillRect(sx - 4, sy - 4, ts + 8, ts + 8, glow);
}

bool Player::canMove(const TileMap& map, float nx, float ny) const {
    int margin = 4;
    int left   = (int)(nx + margin) / Constants::TILE_SIZE;
    int right  = (int)(nx + Constants::TILE_SIZE - margin - 1) / Constants::TILE_SIZE;
    int top    = (int)(ny + margin) / Constants::TILE_SIZE;
    int bottom = (int)(ny + Constants::TILE_SIZE - margin - 1) / Constants::TILE_SIZE;

    return !map.isSolid(left, top) && !map.isSolid(right, top) &&
           !map.isSolid(left, bottom) && !map.isSolid(right, bottom);
}

bool Player::useInk(int amount) {
    if (ink >= amount) {
        ink -= amount;
        return true;
    }
    return false;
}

void Player::addInk(int amount) {
    ink = std::min(ink + amount, Constants::MAX_INK);
}
