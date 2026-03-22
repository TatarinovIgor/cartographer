#include "LivingMap.h"
#include <cmath>
#include <algorithm>
#include <cstdlib>

void LivingMap::enter(int playerTX, int playerTY, const EmotionSystem& emotions) {
    active = true;
    cursorTX = playerTX;
    cursorTY = playerTY;
    cursorBlink = 0;
    moveCooldown = 0;
    currentTool = DrawTool::SHAPE;
    isDrawing = false;
    isLabeling = false;
    labelBuffer.clear();
    activeRegionIndex = -1;
}

void LivingMap::exit() {
    active = false;
    isDrawing = false;
    isLabeling = false;
}

void LivingMap::update(const Input& input, TileMap& map, Player& player,
                        ParticleSystem& particles, Camera& camera,
                        const EmotionSystem& emotions, float dt, float gameTime) {
    if (!active) return;

    cursorBlink += dt;
    moveCooldown -= dt;

    if (input.isKeyPressed(SDL_SCANCODE_ESCAPE) ||
        (input.isKeyPressed(SDL_SCANCODE_M) && !isLabeling) ||
        (input.isKeyPressed(SDL_SCANCODE_TAB) && !isLabeling)) {
        if (isDrawing) {
            finishStroke(map, player, particles, camera, emotions);
        }
        exit();
        return;
    }

    // Tool switching: 1=Shape, 2=Label, 3=Symbol
    if (!isDrawing && !isLabeling) {
        if (input.isKeyPressed(SDL_SCANCODE_1)) currentTool = DrawTool::SHAPE;
        if (input.isKeyPressed(SDL_SCANCODE_2)) currentTool = DrawTool::LABEL;
        if (input.isKeyPressed(SDL_SCANCODE_3)) currentTool = DrawTool::SYMBOL;
    }

    // Ink type switching: Q cycles ink
    if (input.isKeyPressed(SDL_SCANCODE_Q) && !isLabeling) {
        if (currentInk == InkType::STANDARD) currentInk = InkType::MEMORY;
        else if (currentInk == InkType::MEMORY) currentInk = InkType::EMOTION;
        else currentInk = InkType::STANDARD;
    }

    // Symbol cycling with R
    if (currentTool == DrawTool::SYMBOL && input.isKeyPressed(SDL_SCANCODE_R)) {
        int s = (int)selectedSymbol;
        s = (s + 1) % 6;
        selectedSymbol = (SymbolType)s;
    }

    // Cursor movement (when not in label mode)
    if (!isLabeling && moveCooldown <= 0) {
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
                moveCooldown = 0.1f;

                if (isDrawing) {
                    float wx = cursorTX * Constants::TILE_SIZE + Constants::TILE_SIZE / 2.0f;
                    float wy = cursorTY * Constants::TILE_SIZE + Constants::TILE_SIZE / 2.0f;
                    float pressure = 0.7f + (float)(rand() % 30) / 100.0f;
                    if ((int)currentStroke.points.size() < Constants::MAX_STROKE_POINTS) {
                        currentStroke.points.push_back({wx, wy, pressure});
                    }
                }
            }
        }
    }

    switch (currentTool) {
        case DrawTool::SHAPE:
            handleShapeTool(input, map, player, particles, camera, emotions, dt, gameTime);
            break;
        case DrawTool::LABEL:
            handleLabelTool(input, player);
            break;
        case DrawTool::SYMBOL:
            handleSymbolTool(input, map, player, particles, camera);
            break;
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

// --- SHAPE TOOL ---

void LivingMap::handleShapeTool(const Input& input, TileMap& map, Player& player,
                                 ParticleSystem& particles, Camera& camera,
                                 const EmotionSystem& emotions, float dt, float gameTime) {
    if (input.isKeyPressed(SDL_SCANCODE_SPACE) || input.isKeyPressed(SDL_SCANCODE_RETURN)) {
        if (!isDrawing) {
            TileType current = map.getTile(cursorTX, cursorTY);
            if (current == TileType::VOID || current == TileType::HIDDEN) {
                isDrawing = true;
                currentStroke = DrawnStroke{};
                currentStroke.inkType = currentInk;
                float wx = cursorTX * Constants::TILE_SIZE + Constants::TILE_SIZE / 2.0f;
                float wy = cursorTY * Constants::TILE_SIZE + Constants::TILE_SIZE / 2.0f;
                currentStroke.points.push_back({wx, wy, 1.0f});
                drawTimer = 0;
            }
        } else {
            finishStroke(map, player, particles, camera, emotions);
        }
    }

    if (isDrawing) {
        drawTimer += dt;
    }
}

// --- LABEL TOOL ---

void LivingMap::handleLabelTool(const Input& input, Player& player) {
    if (!isLabeling) {
        if (input.isKeyPressed(SDL_SCANCODE_SPACE) || input.isKeyPressed(SDL_SCANCODE_RETURN)) {
            int regionIdx = getRegionAt(cursorTX, cursorTY);
            if (regionIdx >= 0) {
                isLabeling = true;
                activeRegionIndex = regionIdx;
                labelBuffer = regions[regionIdx].label;
            } else {
                // Create a new region just for the label
                DrawnRegion region;
                region.centerTX = cursorTX;
                region.centerTY = cursorTY;
                regions.push_back(region);
                activeRegionIndex = (int)regions.size() - 1;
                isLabeling = true;
                labelBuffer.clear();
            }
        }
    } else {
        // Text input for labeling
        for (int sc = SDL_SCANCODE_A; sc <= SDL_SCANCODE_Z; sc++) {
            if (input.isKeyPressed((SDL_Scancode)sc)) {
                char c = 'a' + (sc - SDL_SCANCODE_A);
                if (input.isKeyDown(SDL_SCANCODE_LSHIFT) || input.isKeyDown(SDL_SCANCODE_RSHIFT))
                    c = c - 32;
                if (labelBuffer.size() < 20)
                    labelBuffer += c;
            }
        }
        if (input.isKeyPressed(SDL_SCANCODE_SPACE)) {
            if (labelBuffer.size() < 20)
                labelBuffer += ' ';
        }
        if (input.isKeyPressed(SDL_SCANCODE_MINUS)) {
            if (labelBuffer.size() < 20)
                labelBuffer += '-';
        }
        if (input.isKeyPressed(SDL_SCANCODE_BACKSPACE) && !labelBuffer.empty()) {
            labelBuffer.pop_back();
        }
        if (input.isKeyPressed(SDL_SCANCODE_RETURN)) {
            if (activeRegionIndex >= 0 && activeRegionIndex < (int)regions.size()) {
                regions[activeRegionIndex].label = labelBuffer;
                regions[activeRegionIndex].hasLabel = true;
                if (player.useInk(Constants::DRAW_COST_LABEL)) {
                    // Label applied
                }
            }
            isLabeling = false;
            labelBuffer.clear();
            activeRegionIndex = -1;
        }
        if (input.isKeyPressed(SDL_SCANCODE_ESCAPE)) {
            isLabeling = false;
            labelBuffer.clear();
            activeRegionIndex = -1;
        }
    }
}

// --- SYMBOL TOOL ---

void LivingMap::handleSymbolTool(const Input& input, TileMap& map, Player& player,
                                  ParticleSystem& particles, Camera& camera) {
    if (input.isKeyPressed(SDL_SCANCODE_SPACE) || input.isKeyPressed(SDL_SCANCODE_RETURN)) {
        TileType current = map.getTile(cursorTX, cursorTY);
        bool isVoidOrHidden = (current == TileType::VOID || current == TileType::HIDDEN);

        if (!player.useInk(Constants::DRAW_COST_SYMBOL)) return;

        TileType revealTo = chooseTileFromSymbol(selectedSymbol);

        if (isVoidOrHidden) {
            map.setTile(cursorTX, cursorTY, revealTo);
            tilesRevealed++;

            RevealAnim anim;
            anim.tx = cursorTX;
            anim.ty = cursorTY;
            anim.progress = 0;
            anim.revealTo = revealTo;
            revealAnims.push_back(anim);

            float wx = cursorTX * Constants::TILE_SIZE;
            float wy = cursorTY * Constants::TILE_SIZE;
            particles.spawnRevealBurst(wx, wy);
            particles.spawnDrawInk(wx, wy);
            camera.shake(2.0f, 0.15f);
        }

        int regionIdx = getRegionAt(cursorTX, cursorTY);
        if (regionIdx >= 0) {
            regions[regionIdx].hasSymbol = true;
            regions[regionIdx].symbol = selectedSymbol;
        } else {
            DrawnRegion region;
            region.centerTX = cursorTX;
            region.centerTY = cursorTY;
            region.hasSymbol = true;
            region.symbol = selectedSymbol;
            region.manifestedAs = revealTo;
            region.fidelity = 0.8f;
            region.affectedTiles.push_back({cursorTX, cursorTY});
            regions.push_back(region);
        }
    }
}

// --- STROKE COMPLETION ---

void LivingMap::finishStroke(TileMap& map, Player& player, ParticleSystem& particles,
                              Camera& camera, const EmotionSystem& emotions) {
    if (!isDrawing || currentStroke.points.size() < 2) {
        isDrawing = false;
        return;
    }

    float fidelity = calculateStrokeFidelity(currentStroke, emotions);
    currentStroke.fidelity = fidelity;
    currentStroke.emotionShake = emotions.getShakiness();

    int cost = Constants::DRAW_COST_SHAPE;
    int existingRegion = getRegionAt(cursorTX, cursorTY);
    bool isRedraw = (existingRegion >= 0 && regions[existingRegion].fullyManifested);
    if (isRedraw) cost *= Constants::REDRAW_COST_MULTIPLIER;

    if (!player.useInk(cost)) {
        isDrawing = false;
        return;
    }

    // Determine affected tiles from stroke path
    std::vector<std::pair<int,int>> affected;
    for (auto& pt : currentStroke.points) {
        int tx = (int)(pt.x / Constants::TILE_SIZE);
        int ty = (int)(pt.y / Constants::TILE_SIZE);
        if (tx >= 0 && tx < map.width() && ty >= 0 && ty < map.height()) {
            bool found = false;
            for (auto& a : affected)
                if (a.first == tx && a.second == ty) { found = true; break; }
            if (!found) affected.push_back({tx, ty});
        }
    }

    // Reveal void/hidden tiles along the stroke
    for (auto& [tx, ty] : affected) {
        TileType t = map.getTile(tx, ty);
        if (t == TileType::VOID || t == TileType::HIDDEN) {
            bool resisted = checkResistance(tx, ty, map);
            if (resisted) {
                camera.shake(4.0f, 0.3f);
                continue;
            }

            TileType revealTo = map.getHiddenReveal(tx, ty);
            // Fidelity affects what gets placed: low fidelity = rougher terrain
            if (fidelity < 0.4f) {
                revealTo = (rand() % 2 == 0) ? TileType::GROUND_ALT : TileType::GRASS;
            }

            map.setTile(tx, ty, revealTo);
            tilesRevealed++;

            RevealAnim anim;
            anim.tx = tx;
            anim.ty = ty;
            anim.progress = 0;
            anim.revealTo = revealTo;
            revealAnims.push_back(anim);

            float wx = tx * Constants::TILE_SIZE;
            float wy = ty * Constants::TILE_SIZE;
            particles.spawnRevealBurst(wx, wy);
            particles.spawnDrawInk(wx, wy);
        }
    }

    camera.shake(2.0f + (1.0f - fidelity) * 3.0f, 0.2f);

    if (isRedraw && existingRegion >= 0) {
        regions[existingRegion].strokes.push_back(currentStroke);
        regions[existingRegion].redrawCount++;
        float oldFidelity = regions[existingRegion].fidelity;
        regions[existingRegion].fidelity = (oldFidelity + fidelity) / 2.0f;
        applyConsequences(regions[existingRegion], map);
    } else {
        DrawnRegion region;
        region.centerTX = cursorTX;
        region.centerTY = cursorTY;
        region.strokes.push_back(currentStroke);
        region.fidelity = fidelity;
        region.emotionAtDrawTime = emotions.intensity;
        region.emotionType = emotions.current;
        region.affectedTiles = affected;
        region.isRedraw = false;
        regions.push_back(region);
    }

    isDrawing = false;
}

float LivingMap::calculateStrokeFidelity(const DrawnStroke& stroke,
                                          const EmotionSystem& emotions) const {
    if (stroke.points.size() < 2) return 0.5f;

    // Smoothness: how consistent is the spacing between points?
    float totalVariance = 0;
    float avgDist = 0;
    for (size_t i = 1; i < stroke.points.size(); i++) {
        float dx = stroke.points[i].x - stroke.points[i-1].x;
        float dy = stroke.points[i].y - stroke.points[i-1].y;
        avgDist += sqrt(dx * dx + dy * dy);
    }
    avgDist /= (stroke.points.size() - 1);

    for (size_t i = 1; i < stroke.points.size(); i++) {
        float dx = stroke.points[i].x - stroke.points[i-1].x;
        float dy = stroke.points[i].y - stroke.points[i-1].y;
        float dist = sqrt(dx * dx + dy * dy);
        float diff = dist - avgDist;
        totalVariance += diff * diff;
    }
    totalVariance /= (stroke.points.size() - 1);
    float smoothness = 1.0f / (1.0f + totalVariance * 0.001f);

    // Coverage: how many tiles did the stroke cover?
    float coverage = std::min(1.0f, (float)stroke.points.size() / 8.0f);

    // Emotion penalty
    float emotionPenalty = 1.0f - emotions.getShakiness() * 0.1f;
    emotionPenalty = std::clamp(emotionPenalty, 0.3f, 1.0f);

    // Ink type bonus
    float inkBonus = 1.0f;
    if (stroke.inkType == InkType::MEMORY) inkBonus = 0.9f;
    if (stroke.inkType == InkType::EMOTION) inkBonus = 1.1f;

    float fidelity = smoothness * 0.4f + coverage * 0.3f + emotionPenalty * 0.3f;
    fidelity *= inkBonus;
    return std::clamp(fidelity, 0.1f, 1.0f);
}

TileType LivingMap::chooseTileFromSymbol(SymbolType sym) const {
    switch (sym) {
        case SymbolType::HOUSE:   return TileType::WALL;
        case SymbolType::TREE:    return TileType::TREE;
        case SymbolType::WATER:   return TileType::WATER;
        case SymbolType::BRIDGE:  return TileType::PATH;
        case SymbolType::DANGER:  return TileType::GROUND_ALT;
        case SymbolType::MYSTERY: return TileType::HIDDEN;
    }
    return TileType::GROUND;
}

bool LivingMap::checkResistance(int tx, int ty, const TileMap& map) const {
    // Some tiles resist being drawn -- near the deep void center
    int distFromEdge = std::min({tx, ty, map.width() - 1 - tx, map.height() - 1 - ty});
    if (distFromEdge <= 1) return rand() % 3 == 0;
    return false;
}

void LivingMap::applyConsequences(DrawnRegion& region, TileMap& map) {
    if (region.fidelity < 0.4f) {
        DrawingConsequence cons;
        cons.regionIndex = -1;
        for (int i = 0; i < (int)regions.size(); i++)
            if (&regions[i] == &region) { cons.regionIndex = i; break; }

        if (region.fidelity < 0.25f) {
            cons.description = "Your rough drawing has made this area unstable.";
            cons.severity = 0.8f;
            // Low fidelity: some tiles revert or become wrong
            for (auto& [tx, ty] : region.affectedTiles) {
                if (rand() % 3 == 0) {
                    map.setTile(tx, ty, TileType::GROUND_ALT);
                }
            }
        } else {
            cons.description = "The lines aren't quite right. The place feels... off.";
            cons.severity = 0.4f;
        }
        consequences.push_back(cons);
    }
}

// --- MANIFESTATION (world catches up to drawing) ---

void LivingMap::updateManifestations(TileMap& map, ParticleSystem& particles,
                                      Camera& camera, float dt) {
    for (auto& region : regions) {
        if (region.fullyManifested) continue;
        if (region.strokes.empty() && !region.hasSymbol) continue;

        region.manifestProgress += dt / Constants::MANIFEST_TIME;
        if (region.manifestProgress >= 1.0f) {
            region.manifestProgress = 1.0f;
            region.fullyManifested = true;

            for (auto& [tx, ty] : region.affectedTiles) {
                float wx = tx * Constants::TILE_SIZE;
                float wy = ty * Constants::TILE_SIZE;
                particles.spawnRevealBurst(wx, wy);
            }

            if (region.fidelity < 0.3f) {
                camera.shake(5.0f, 0.5f);
            }
        }
    }
}

bool LivingMap::hasConsequences() const {
    for (auto& c : consequences)
        if (!c.resolved) return true;
    return false;
}

DrawingConsequence LivingMap::getLatestConsequence() const {
    for (int i = (int)consequences.size() - 1; i >= 0; i--)
        if (!consequences[i].resolved) return consequences[i];
    return {"", -1, 0, true};
}

int LivingMap::getRegionAt(int tx, int ty) const {
    for (int i = 0; i < (int)regions.size(); i++) {
        for (auto& [ax, ay] : regions[i].affectedTiles) {
            if (ax == tx && ay == ty) return i;
        }
        if (regions[i].centerTX == tx && regions[i].centerTY == ty) return i;
    }
    return -1;
}

float LivingMap::getRegionFidelity(int index) const {
    if (index >= 0 && index < (int)regions.size())
        return regions[index].fidelity;
    return 0.0f;
}

std::string LivingMap::getRegionLabel(int index) const {
    if (index >= 0 && index < (int)regions.size() && regions[index].hasLabel)
        return regions[index].label;
    return "";
}

// --- RENDERING ---

void LivingMap::render(Renderer& renderer, Camera& camera, const AssetManager& assets,
                        const EmotionSystem& emotions, float time) {
    if (!active) return;

    renderParchmentOverlay(renderer, camera, time);
    renderGrid(renderer, camera);
    renderStrokes(renderer, camera, emotions, time);
    renderLabels(renderer, camera, assets);
    renderSymbols(renderer, camera);

    if (isDrawing)
        renderCurrentStroke(renderer, camera, emotions, time);

    renderCursor(renderer, camera, time);
    renderToolbar(renderer, assets, Player{}, emotions);
}

void LivingMap::renderParchmentOverlay(Renderer& renderer, Camera& camera, float time) {
    renderer.fillRectAbsolute(0, 0, Constants::SCREEN_WIDTH, Constants::SCREEN_HEIGHT,
        Constants::Colors::PARCHMENT_FULL);

    // Subtle grain effect
    for (int i = 0; i < 40; i++) {
        int gx = rand() % Constants::SCREEN_WIDTH;
        int gy = rand() % Constants::SCREEN_HEIGHT;
        unsigned char a = 10 + rand() % 20;
        renderer.fillRectAbsolute(gx, gy, 2, 2, {160, 140, 110, a});
    }
}

void LivingMap::renderGrid(Renderer& renderer, Camera& camera) {
    float shX = camera.shakeOffsetX();
    float shY = camera.shakeOffsetY();

    for (int x = 0; x < Constants::MAP_WIDTH; x++) {
        float sx = camera.screenX(x * Constants::TILE_SIZE) + shX;
        renderer.drawLine(sx, 0, sx, Constants::SCREEN_HEIGHT, Constants::Colors::GRID_LINE);
    }
    for (int y = 0; y < Constants::MAP_HEIGHT; y++) {
        float sy = camera.screenY(y * Constants::TILE_SIZE) + shY;
        renderer.drawLine(0, sy, Constants::SCREEN_WIDTH, sy, Constants::Colors::GRID_LINE);
    }
}

void LivingMap::renderStrokes(Renderer& renderer, Camera& camera,
                               const EmotionSystem& emotions, float time) {
    for (auto& region : regions) {
        for (auto& stroke : region.strokes) {
            if (stroke.points.size() < 2) continue;

            Constants::Colors::Color inkColor;
            switch (stroke.inkType) {
                case InkType::STANDARD: inkColor = Constants::Colors::INK_STANDARD; break;
                case InkType::MEMORY:   inkColor = Constants::Colors::INK_MEMORY; break;
                case InkType::EMOTION:  inkColor = Constants::Colors::INK_EMOTION; break;
            }

            for (size_t i = 1; i < stroke.points.size(); i++) {
                float x1 = camera.screenX(stroke.points[i-1].x) + camera.shakeOffsetX();
                float y1 = camera.screenY(stroke.points[i-1].y) + camera.shakeOffsetY();
                float x2 = camera.screenX(stroke.points[i].x) + camera.shakeOffsetX();
                float y2 = camera.screenY(stroke.points[i].y) + camera.shakeOffsetY();

                // Apply stored emotion wobble
                if (stroke.emotionShake > 0.1f) {
                    x1 += sin(time * 2.0f + i * 0.5f) * stroke.emotionShake * 0.5f;
                    y1 += cos(time * 2.0f + i * 0.3f) * stroke.emotionShake * 0.5f;
                    x2 += sin(time * 2.0f + (i+1) * 0.5f) * stroke.emotionShake * 0.5f;
                    y2 += cos(time * 2.0f + (i+1) * 0.3f) * stroke.emotionShake * 0.5f;
                }

                int thickness = std::max(1, (int)(stroke.points[i].pressure * 3.0f));
                renderer.drawLineThick(x1, y1, x2, y2, thickness, inkColor);
            }
        }
    }
}

void LivingMap::renderCurrentStroke(Renderer& renderer, Camera& camera,
                                     const EmotionSystem& emotions, float time) {
    if (currentStroke.points.size() < 2) return;

    Constants::Colors::Color inkColor;
    switch (currentInk) {
        case InkType::STANDARD: inkColor = Constants::Colors::INK_STANDARD; break;
        case InkType::MEMORY:   inkColor = Constants::Colors::INK_MEMORY; break;
        case InkType::EMOTION:  inkColor = Constants::Colors::INK_EMOTION; break;
    }
    inkColor.a = 180;

    for (size_t i = 1; i < currentStroke.points.size(); i++) {
        float x1 = camera.screenX(currentStroke.points[i-1].x) + camera.shakeOffsetX();
        float y1 = camera.screenY(currentStroke.points[i-1].y) + camera.shakeOffsetY();
        float x2 = camera.screenX(currentStroke.points[i].x) + camera.shakeOffsetX();
        float y2 = camera.screenY(currentStroke.points[i].y) + camera.shakeOffsetY();

        x1 = emotions.applyToStrokeX(x1, time);
        y1 = emotions.applyToStrokeY(y1, time);
        x2 = emotions.applyToStrokeX(x2, time);
        y2 = emotions.applyToStrokeY(y2, time);

        renderer.drawLineThick(x1, y1, x2, y2, 2, inkColor);
    }
}

void LivingMap::renderLabels(Renderer& renderer, Camera& camera, const AssetManager& assets) {
    TTF_Font* font = assets.getFont("small");
    if (!font) return;

    for (auto& region : regions) {
        if (!region.hasLabel) continue;
        float sx = camera.screenX(region.centerTX * Constants::TILE_SIZE) + camera.shakeOffsetX();
        float sy = camera.screenY(region.centerTY * Constants::TILE_SIZE) + camera.shakeOffsetY();

        float sizeMod = 1.0f;
        if (region.emotionType == Emotion::GRIEVING)
            sizeMod = 1.0f - region.emotionAtDrawTime * 0.2f;

        renderer.drawText(region.label, (int)(sx - region.label.size() * 3),
            (int)(sy - 18 * sizeMod), Constants::Colors::LABEL_TEXT, font);
    }

    // Active label being typed
    if (isLabeling) {
        float sx = camera.screenX(cursorTX * Constants::TILE_SIZE) + camera.shakeOffsetX();
        float sy = camera.screenY(cursorTY * Constants::TILE_SIZE) + camera.shakeOffsetY();
        std::string display = labelBuffer + "_";
        renderer.fillRectAbsolute((int)sx - 5, (int)sy - 22,
            (int)display.size() * 8 + 10, 20, {240, 220, 190, 220});
        renderer.drawText(display, (int)sx, (int)(sy - 20),
            Constants::Colors::INK_STANDARD, font);
    }
}

void LivingMap::renderSymbols(Renderer& renderer, Camera& camera) {
    for (auto& region : regions) {
        if (!region.hasSymbol) continue;
        float sx = camera.screenX(region.centerTX * Constants::TILE_SIZE) + camera.shakeOffsetX();
        float sy = camera.screenY(region.centerTY * Constants::TILE_SIZE) + camera.shakeOffsetY();
        int ts = Constants::TILE_SIZE;
        auto ink = Constants::Colors::INK_STANDARD;

        switch (region.symbol) {
            case SymbolType::HOUSE:
                renderer.drawRect(sx + 6, sy + 10, ts - 12, ts - 14, ink);
                renderer.drawLine(sx + 3, sy + 10, sx + ts/2, sy + 2, ink);
                renderer.drawLine(sx + ts/2, sy + 2, sx + ts - 3, sy + 10, ink);
                break;
            case SymbolType::TREE:
                renderer.drawLine(sx + ts/2, sy + ts - 4, sx + ts/2, sy + ts/2, ink);
                renderer.drawCircle(sx + ts/2, sy + ts/3, 8, ink);
                break;
            case SymbolType::WATER:
                for (int w = 0; w < 3; w++) {
                    float wy = sy + 8 + w * 8;
                    renderer.drawLine(sx + 4, wy, sx + ts/3, wy - 3, ink);
                    renderer.drawLine(sx + ts/3, wy - 3, sx + ts*2/3, wy + 3, ink);
                    renderer.drawLine(sx + ts*2/3, wy + 3, sx + ts - 4, wy, ink);
                }
                break;
            case SymbolType::BRIDGE:
                renderer.drawLine(sx + 2, sy + ts/2, sx + ts - 2, sy + ts/2, ink);
                renderer.drawLine(sx + 6, sy + ts/2, sx + ts/3, sy + 6, ink);
                renderer.drawLine(sx + ts*2/3, sy + 6, sx + ts - 6, sy + ts/2, ink);
                break;
            case SymbolType::DANGER:
                renderer.drawLine(sx + ts/2, sy + 4, sx + 4, sy + ts - 4, ink);
                renderer.drawLine(sx + 4, sy + ts - 4, sx + ts - 4, sy + ts - 4, ink);
                renderer.drawLine(sx + ts - 4, sy + ts - 4, sx + ts/2, sy + 4, ink);
                renderer.fillRect(sx + ts/2 - 1, sy + 12, 3, 8, ink);
                renderer.fillRect(sx + ts/2 - 1, sy + 22, 3, 3, ink);
                break;
            case SymbolType::MYSTERY:
                renderer.drawCircle(sx + ts/2, sy + ts/2, 10, ink);
                renderer.fillRect(sx + ts/2 - 1, sy + ts/2 - 4, 3, 6, ink);
                renderer.fillRect(sx + ts/2 - 1, sy + ts/2 + 4, 3, 3, ink);
                break;
        }
    }
}

void LivingMap::renderCursor(Renderer& renderer, Camera& camera, float time) {
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

    if (isDrawing) {
        renderer.drawRect(sx - 1, sy - 1, ts + 2, ts + 2, Constants::Colors::INK_STANDARD);
    }
}

void LivingMap::renderToolbar(Renderer& renderer, const AssetManager& assets,
                               const Player& player, const EmotionSystem& emotions) {
    TTF_Font* uiFont = assets.getFont("ui");
    TTF_Font* smallFont = assets.getFont("small");
    if (!uiFont) return;

    // Top bar: mode + tool
    renderer.fillRectAbsolute(0, 0, Constants::SCREEN_WIDTH, 36, Constants::Colors::UI_BG);

    renderer.drawTextCentered("LIVING MAP", 8, Constants::Colors::UI_HIGHLIGHT, uiFont);

    // Tool indicators
    if (smallFont) {
        const char* tools[] = {"[1]Shape", "[2]Label", "[3]Symbol"};
        for (int i = 0; i < 3; i++) {
            auto color = ((int)currentTool == i)
                ? Constants::Colors::UI_HIGHLIGHT : Constants::Colors::UI_TEXT;
            renderer.drawText(tools[i], 20 + i * 100, 20, color, smallFont);
        }

        // Ink type
        const char* inkNames[] = {"Standard", "Memory", "Emotion"};
        Constants::Colors::Color inkColors[] = {
            Constants::Colors::INK_STANDARD,
            Constants::Colors::INK_MEMORY,
            Constants::Colors::INK_EMOTION
        };
        renderer.drawText(std::string("[Q]Ink: ") + inkNames[(int)currentInk],
            340, 20, inkColors[(int)currentInk], smallFont);

        // Symbol type (if symbol tool active)
        if (currentTool == DrawTool::SYMBOL) {
            const char* symNames[] = {"House", "Tree", "Water", "Bridge", "Danger", "Mystery"};
            renderer.drawText(std::string("[R]Symbol: ") + symNames[(int)selectedSymbol],
                520, 20, Constants::Colors::UI_TEXT, smallFont);
        }

        // Drawing state
        if (isDrawing) {
            renderer.drawText("DRAWING... [Space] to finish",
                Constants::SCREEN_WIDTH - 250, 20, Constants::Colors::UI_HIGHLIGHT, smallFont);
        }
        if (isLabeling) {
            renderer.drawText("LABELING... type name, [Enter] to confirm",
                Constants::SCREEN_WIDTH - 320, 20, Constants::Colors::UI_HIGHLIGHT, smallFont);
        }
    }

    // Bottom controls
    if (smallFont) {
        std::string controls = "[WASD]Move  [Space]Draw/Place  [M/Tab]Exit";
        if (isLabeling) controls = "[Type]Name  [Enter]Confirm  [Esc]Cancel";
        renderer.drawText(controls, 10, Constants::SCREEN_HEIGHT - 25,
            Constants::Colors::UI_TEXT, smallFont);
    }

    // Fidelity of current stroke
    if (isDrawing && !currentStroke.points.empty()) {
        float approxFidelity = std::min(1.0f, (float)currentStroke.points.size() / 8.0f);
        approxFidelity *= (1.0f - emotions.getShakiness() * 0.1f);
        renderFidelityIndicator(renderer, assets, approxFidelity);
    }
}

void LivingMap::renderFidelityIndicator(Renderer& renderer, const AssetManager& assets,
                                         float fidelity) {
    TTF_Font* font = assets.getFont("small");
    if (!font) return;

    int barX = Constants::SCREEN_WIDTH - 170, barY = 50;
    int barW = 150, barH = 12;

    renderer.fillRectAbsolute(barX - 1, barY - 1, barW + 2, barH + 2, Constants::Colors::UI_BG);

    Constants::Colors::Color fillColor;
    if (fidelity > 0.7f) fillColor = Constants::Colors::FIDELITY_HIGH;
    else if (fidelity > 0.4f) fillColor = Constants::Colors::FIDELITY_MED;
    else fillColor = Constants::Colors::FIDELITY_LOW;

    renderer.fillRectAbsolute(barX, barY, (int)(barW * fidelity), barH, fillColor);
    renderer.drawText("Fidelity", barX, barY - 14, Constants::Colors::UI_TEXT, font);
}

void LivingMap::renderMinimap(Renderer& renderer, const TileMap& map, const Player& player) {
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

    // Drawn region markers on minimap
    for (auto& region : regions) {
        int rx = mmX + (int)(region.centerTX * scaleX);
        int ry = mmY + (int)(region.centerTY * scaleY);
        renderer.fillRectAbsolute(rx - 1, ry - 1, 3, 3, Constants::Colors::INK_STANDARD);
    }

    int ppx = mmX + (int)(player.tileX() * scaleX);
    int ppy = mmY + (int)(player.tileY() * scaleY);
    renderer.fillRectAbsolute(ppx - 1, ppy - 1, 3, 3, Constants::Colors::SABLE);
}
