#pragma once
#include "Constants.h"
#include "Renderer.h"
#include "Camera.h"
#include "Input.h"
#include "TileMap.h"
#include "Player.h"
#include "ParticleSystem.h"
#include "AssetManager.h"
#include "EmotionSystem.h"
#include <vector>
#include <string>

enum class DrawTool {
    SHAPE,
    LABEL,
    SYMBOL
};

enum class InkType {
    STANDARD,
    MEMORY,
    EMOTION
};

enum class SymbolType {
    HOUSE,
    TREE,
    WATER,
    BRIDGE,
    DANGER,
    MYSTERY
};

struct StrokePoint {
    float x, y;
    float pressure;
};

struct DrawnStroke {
    std::vector<StrokePoint> points;
    InkType inkType = InkType::STANDARD;
    float fidelity = 1.0f;
    float emotionShake = 0.0f;
};

struct DrawnRegion {
    int centerTX, centerTY;
    std::string label;
    SymbolType symbol = SymbolType::HOUSE;
    bool hasLabel = false;
    bool hasSymbol = false;
    std::vector<DrawnStroke> strokes;
    float fidelity = 1.0f;
    float manifestProgress = 0.0f;
    bool fullyManifested = false;
    bool isRedraw = false;
    int redrawCount = 0;
    TileType manifestedAs = TileType::GROUND;

    float emotionAtDrawTime = 0.0f;
    Emotion emotionType = Emotion::CALM;

    std::vector<std::pair<int,int>> affectedTiles;

    bool resisted = false;
    float resistStrength = 0.0f;
};

struct DrawingConsequence {
    std::string description;
    int regionIndex;
    float severity;
    bool resolved = false;
};

class LivingMap {
public:
    bool active = false;
    int tilesRevealed = 0;
    std::vector<DrawnRegion> regions;
    std::vector<DrawingConsequence> consequences;

    void enter(int playerTX, int playerTY, const EmotionSystem& emotions);
    void exit();
    void update(const Input& input, TileMap& map, Player& player,
                ParticleSystem& particles, Camera& camera,
                const EmotionSystem& emotions, float dt, float gameTime);
    void render(Renderer& renderer, Camera& camera, const AssetManager& assets,
                const EmotionSystem& emotions, float time);

    void renderMinimap(Renderer& renderer, const TileMap& map, const Player& player);

    void updateManifestations(TileMap& map, ParticleSystem& particles,
                              Camera& camera, float dt);

    bool hasConsequences() const;
    DrawingConsequence getLatestConsequence() const;

    int getRegionAt(int tx, int ty) const;
    float getRegionFidelity(int index) const;
    std::string getRegionLabel(int index) const;

private:
    int cursorTX = 0, cursorTY = 0;
    float cursorBlink = 0;
    float moveCooldown = 0;

    DrawTool currentTool = DrawTool::SHAPE;
    InkType currentInk = InkType::STANDARD;

    bool isDrawing = false;
    DrawnStroke currentStroke;
    float drawTimer = 0;

    bool isLabeling = false;
    std::string labelBuffer;

    SymbolType selectedSymbol = SymbolType::HOUSE;

    int activeRegionIndex = -1;

    struct RevealAnim {
        int tx, ty;
        float progress;
        TileType revealTo;
    };
    std::vector<RevealAnim> revealAnims;

    void handleShapeTool(const Input& input, TileMap& map, Player& player,
                         ParticleSystem& particles, Camera& camera,
                         const EmotionSystem& emotions, float dt, float gameTime);
    void handleLabelTool(const Input& input, Player& player);
    void handleSymbolTool(const Input& input, TileMap& map, Player& player,
                          ParticleSystem& particles, Camera& camera);

    void finishStroke(TileMap& map, Player& player, ParticleSystem& particles,
                      Camera& camera, const EmotionSystem& emotions);
    float calculateStrokeFidelity(const DrawnStroke& stroke, const EmotionSystem& emotions) const;
    TileType chooseTileFromSymbol(SymbolType sym) const;
    bool checkResistance(int tx, int ty, const TileMap& map) const;
    void applyConsequences(DrawnRegion& region, TileMap& map);

    void renderParchmentOverlay(Renderer& renderer, Camera& camera, float time);
    void renderGrid(Renderer& renderer, Camera& camera);
    void renderStrokes(Renderer& renderer, Camera& camera, const EmotionSystem& emotions, float time);
    void renderCurrentStroke(Renderer& renderer, Camera& camera,
                             const EmotionSystem& emotions, float time);
    void renderLabels(Renderer& renderer, Camera& camera, const AssetManager& assets);
    void renderSymbols(Renderer& renderer, Camera& camera);
    void renderCursor(Renderer& renderer, Camera& camera, float time);
    void renderToolbar(Renderer& renderer, const AssetManager& assets,
                       const Player& player, const EmotionSystem& emotions);
    void renderFidelityIndicator(Renderer& renderer, const AssetManager& assets, float fidelity);
};
