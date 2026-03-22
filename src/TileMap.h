#pragma once
#include "Constants.h"
#include "Renderer.h"
#include "Camera.h"
#include <vector>

enum class TileType {
    GROUND,
    GROUND_ALT,
    WALL,
    WALL_DARK,
    ROOF,
    PATH,
    WATER,
    VOID,
    HIDDEN,
    TREE,
    GRASS,
    FLOWER
};

class TileMap {
public:
    void init();
    void render(Renderer& renderer, Camera& camera, float time);

    TileType getTile(int tx, int ty) const;
    void setTile(int tx, int ty, TileType type);
    bool isSolid(int tx, int ty) const;
    bool isVoid(int tx, int ty) const;
    bool isHidden(int tx, int ty) const;

    int width() const { return Constants::MAP_WIDTH; }
    int height() const { return Constants::MAP_HEIGHT; }

    TileType getHiddenReveal(int tx, int ty) const;
    void setHiddenReveal(int tx, int ty, TileType revealed);

    int countVoidTiles() const;
    int countHiddenTiles() const;
    std::vector<std::pair<int,int>> getVoidBorderTiles() const;

private:
    TileType tiles[Constants::MAP_WIDTH][Constants::MAP_HEIGHT] = {};
    TileType hiddenReveals[Constants::MAP_WIDTH][Constants::MAP_HEIGHT] = {};

    void buildBackwardTown();
    void placeBuilding(int x, int y, int w, int h);
    void placePath(int x1, int y1, int x2, int y2);
    void placeVoidBorder();

    Constants::Colors::Color getTileColor(TileType type, int tx, int ty, float time) const;
};
