#include "TileMap.h"
#include <cmath>
#include <cstdlib>
#include <algorithm>

void TileMap::init() {
    for (int x = 0; x < Constants::MAP_WIDTH; x++)
        for (int y = 0; y < Constants::MAP_HEIGHT; y++) {
            tiles[x][y] = TileType::GROUND;
            hiddenReveals[x][y] = TileType::GROUND;
        }
    buildBackwardTown();
}

void TileMap::buildBackwardTown() {
    // Scatter some ground variation
    for (int x = 0; x < Constants::MAP_WIDTH; x++)
        for (int y = 0; y < Constants::MAP_HEIGHT; y++) {
            if (rand() % 5 == 0)
                tiles[x][y] = TileType::GROUND_ALT;
            if (rand() % 12 == 0)
                tiles[x][y] = TileType::GRASS;
            if (rand() % 40 == 0)
                tiles[x][y] = TileType::FLOWER;
        }

    // Main paths - crossroads through town center (around 25, 19)
    placePath(10, 19, 40, 19);  // horizontal main road
    placePath(25, 8, 25, 30);   // vertical main road

    // Town square (central clearing)
    for (int x = 22; x <= 28; x++)
        for (int y = 17; y <= 21; y++)
            tiles[x][y] = TileType::PATH;

    // Buildings
    placeBuilding(12, 13, 5, 4);  // Inn (top-left of center)
    placeBuilding(14, 22, 4, 3);  // Small house
    placeBuilding(30, 14, 6, 4);  // Cartographer's workshop
    placeBuilding(30, 22, 4, 4);  // Storage
    placeBuilding(18, 9, 4, 3);   // House
    placeBuilding(33, 9, 3, 3);   // Cottage
    placeBuilding(20, 26, 5, 3);  // Large house
    placeBuilding(10, 26, 3, 3);  // Shed

    // River running along the east side (forgetful river)
    for (int y = 0; y < Constants::MAP_HEIGHT; y++) {
        int rx = 42 + (int)(sin(y * 0.3) * 2);
        for (int dx = 0; dx < 3; dx++) {
            int wx = rx + dx;
            if (wx >= 0 && wx < Constants::MAP_WIDTH)
                tiles[wx][y] = TileType::WATER;
        }
    }

    // Trees scattered around the edges and between buildings
    int treePositions[][2] = {
        {8,10}, {9,15}, {7,20}, {8,25}, {11,8}, {16,7},
        {22,7}, {28,8}, {35,7}, {38,12}, {39,18}, {38,25},
        {36,30}, {28,28}, {15,30}, {10,32}, {6,28}, {6,16},
        {20,14}, {32,19}, {18,24}, {27,25}, {34,27}, {9,22},
        {37,14}, {22,32}, {14,10}, {29,10}
    };
    for (auto& pos : treePositions) {
        if (pos[0] >= 0 && pos[0] < Constants::MAP_WIDTH &&
            pos[1] >= 0 && pos[1] < Constants::MAP_HEIGHT &&
            tiles[pos[0]][pos[1]] != TileType::WALL &&
            tiles[pos[0]][pos[1]] != TileType::PATH &&
            tiles[pos[0]][pos[1]] != TileType::WATER)
            tiles[pos[0]][pos[1]] = TileType::TREE;
    }

    // The void: creeping in from the north and east edges
    placeVoidBorder();
}

void TileMap::placeBuilding(int bx, int by, int w, int h) {
    for (int x = bx; x < bx + w && x < Constants::MAP_WIDTH; x++) {
        for (int y = by; y < by + h && y < Constants::MAP_HEIGHT; y++) {
            if (y == by)
                tiles[x][y] = TileType::ROOF;
            else
                tiles[x][y] = TileType::WALL;
        }
    }
    // Doorway
    int doorX = bx + w / 2;
    int doorY = by + h - 1;
    if (doorX < Constants::MAP_WIDTH && doorY < Constants::MAP_HEIGHT)
        tiles[doorX][doorY] = TileType::WALL_DARK;
}

void TileMap::placePath(int x1, int y1, int x2, int y2) {
    int dx = (x2 > x1) ? 1 : (x2 < x1) ? -1 : 0;
    int dy = (y2 > y1) ? 1 : (y2 < y1) ? -1 : 0;
    int x = x1, y = y1;
    while (true) {
        if (x >= 0 && x < Constants::MAP_WIDTH && y >= 0 && y < Constants::MAP_HEIGHT) {
            if (tiles[x][y] != TileType::WALL && tiles[x][y] != TileType::ROOF)
                tiles[x][y] = TileType::PATH;
            // Widen path
            if (dy != 0 && x + 1 < Constants::MAP_WIDTH &&
                tiles[x+1][y] != TileType::WALL && tiles[x+1][y] != TileType::ROOF)
                tiles[x+1][y] = TileType::PATH;
            if (dx != 0 && y + 1 < Constants::MAP_HEIGHT &&
                tiles[x][y+1] != TileType::WALL && tiles[x][y+1] != TileType::ROOF)
                tiles[x][y+1] = TileType::PATH;
        }
        if (x == x2 && y == y2) break;
        x += dx;
        y += dy;
    }
}

void TileMap::placeVoidBorder() {
    // Void creeps in from the top and right edges
    for (int x = 0; x < Constants::MAP_WIDTH; x++) {
        int depth = 3 + rand() % 3;
        for (int y = 0; y < depth && y < Constants::MAP_HEIGHT; y++)
            tiles[x][y] = TileType::VOID;
    }
    for (int y = 0; y < Constants::MAP_HEIGHT; y++) {
        int depth = 2 + rand() % 3;
        for (int x = Constants::MAP_WIDTH - depth; x < Constants::MAP_WIDTH; x++) {
            if (x >= 0)
                tiles[x][y] = TileType::VOID;
        }
    }

    // Hidden tiles just inside the void border
    for (int x = 0; x < Constants::MAP_WIDTH; x++) {
        for (int y = 0; y < Constants::MAP_HEIGHT; y++) {
            if (tiles[x][y] != TileType::VOID) continue;
            // Check if adjacent to non-void
            bool border = false;
            for (int dx = -1; dx <= 1; dx++)
                for (int dy = -1; dy <= 1; dy++) {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < Constants::MAP_WIDTH &&
                        ny >= 0 && ny < Constants::MAP_HEIGHT &&
                        tiles[nx][ny] != TileType::VOID)
                        border = true;
                }
            if (border && rand() % 3 == 0) {
                tiles[x][y] = TileType::HIDDEN;
                hiddenReveals[x][y] = (rand() % 2 == 0) ? TileType::GROUND : TileType::PATH;
            }
        }
    }
}

void TileMap::render(Renderer& renderer, Camera& camera, float time) {
    int startX = std::max(0, (int)(camera.x / Constants::TILE_SIZE) - 1);
    int startY = std::max(0, (int)(camera.y / Constants::TILE_SIZE) - 1);
    int endX = std::min(Constants::MAP_WIDTH,
        startX + Constants::SCREEN_WIDTH / Constants::TILE_SIZE + 3);
    int endY = std::min(Constants::MAP_HEIGHT,
        startY + Constants::SCREEN_HEIGHT / Constants::TILE_SIZE + 3);

    float shX = camera.shakeOffsetX();
    float shY = camera.shakeOffsetY();

    for (int x = startX; x < endX; x++) {
        for (int y = startY; y < endY; y++) {
            float sx = camera.screenX(x * Constants::TILE_SIZE) + shX;
            float sy = camera.screenY(y * Constants::TILE_SIZE) + shY;
            auto color = getTileColor(tiles[x][y], x, y, time);
            renderer.fillRect(sx, sy, Constants::TILE_SIZE, Constants::TILE_SIZE, color);

            // Tree decoration: draw trunk + canopy on top of base tile
            if (tiles[x][y] == TileType::TREE) {
                int ts = Constants::TILE_SIZE;
                renderer.fillRect(sx + ts * 0.35f, sy + ts * 0.5f,
                    ts * 0.3f, ts * 0.5f, Constants::Colors::TREE_TRUNK);
                renderer.fillRect(sx + ts * 0.1f, sy + ts * 0.05f,
                    ts * 0.8f, ts * 0.55f, Constants::Colors::TREE_CANOPY);
            }

            // Flower decoration
            if (tiles[x][y] == TileType::FLOWER) {
                int ts = Constants::TILE_SIZE;
                renderer.fillRect(sx + ts * 0.4f, sy + ts * 0.5f,
                    ts * 0.2f, ts * 0.4f, Constants::Colors::TREE_TRUNK);
                renderer.fillRect(sx + ts * 0.3f, sy + ts * 0.2f,
                    ts * 0.4f, ts * 0.35f, Constants::Colors::FLOWER);
            }
        }
    }
}

TileType TileMap::getTile(int tx, int ty) const {
    if (tx < 0 || tx >= Constants::MAP_WIDTH || ty < 0 || ty >= Constants::MAP_HEIGHT)
        return TileType::VOID;
    return tiles[tx][ty];
}

void TileMap::setTile(int tx, int ty, TileType type) {
    if (tx >= 0 && tx < Constants::MAP_WIDTH && ty >= 0 && ty < Constants::MAP_HEIGHT)
        tiles[tx][ty] = type;
}

bool TileMap::isSolid(int tx, int ty) const {
    TileType t = getTile(tx, ty);
    return t == TileType::WALL || t == TileType::WALL_DARK ||
           t == TileType::ROOF || t == TileType::WATER ||
           t == TileType::VOID || t == TileType::TREE;
}

bool TileMap::isVoid(int tx, int ty) const {
    return getTile(tx, ty) == TileType::VOID;
}

bool TileMap::isHidden(int tx, int ty) const {
    return getTile(tx, ty) == TileType::HIDDEN;
}

TileType TileMap::getHiddenReveal(int tx, int ty) const {
    if (tx >= 0 && tx < Constants::MAP_WIDTH && ty >= 0 && ty < Constants::MAP_HEIGHT)
        return hiddenReveals[tx][ty];
    return TileType::GROUND;
}

void TileMap::setHiddenReveal(int tx, int ty, TileType revealed) {
    if (tx >= 0 && tx < Constants::MAP_WIDTH && ty >= 0 && ty < Constants::MAP_HEIGHT)
        hiddenReveals[tx][ty] = revealed;
}

int TileMap::countVoidTiles() const {
    int count = 0;
    for (int x = 0; x < Constants::MAP_WIDTH; x++)
        for (int y = 0; y < Constants::MAP_HEIGHT; y++)
            if (tiles[x][y] == TileType::VOID) count++;
    return count;
}

int TileMap::countHiddenTiles() const {
    int count = 0;
    for (int x = 0; x < Constants::MAP_WIDTH; x++)
        for (int y = 0; y < Constants::MAP_HEIGHT; y++)
            if (tiles[x][y] == TileType::HIDDEN) count++;
    return count;
}

std::vector<std::pair<int,int>> TileMap::getVoidBorderTiles() const {
    std::vector<std::pair<int,int>> borders;
    for (int x = 0; x < Constants::MAP_WIDTH; x++) {
        for (int y = 0; y < Constants::MAP_HEIGHT; y++) {
            if (tiles[x][y] != TileType::VOID) continue;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    int nx = x + dx, ny = y + dy;
                    if (nx >= 0 && nx < Constants::MAP_WIDTH &&
                        ny >= 0 && ny < Constants::MAP_HEIGHT &&
                        tiles[nx][ny] != TileType::VOID &&
                        tiles[nx][ny] != TileType::HIDDEN) {
                        borders.emplace_back(x, y);
                        goto next;
                    }
                }
            }
            next:;
        }
    }
    return borders;
}

Constants::Colors::Color TileMap::getTileColor(TileType type, int tx, int ty, float time) const {
    using namespace Constants::Colors;
    switch (type) {
        case TileType::GROUND:     return GROUND;
        case TileType::GROUND_ALT: return GROUND_ALT;
        case TileType::WALL:       return WALL;
        case TileType::WALL_DARK:  return WALL_DARK;
        case TileType::ROOF:       return ROOF;
        case TileType::PATH:       return PATH;
        case TileType::GRASS:      return GRASS;
        case TileType::FLOWER:     return GROUND;
        case TileType::TREE:       return GROUND;
        case TileType::WATER: {
            float wave = sin(time * 2.0f + tx * 0.5f + ty * 0.3f) * 0.5f + 0.5f;
            return Color{
                (unsigned char)(WATER.r + (WATER_DEEP.r - WATER.r) * wave),
                (unsigned char)(WATER.g + (WATER_DEEP.g - WATER.g) * wave),
                (unsigned char)(WATER.b + (WATER_DEEP.b - WATER.b) * wave),
                255
            };
        }
        case TileType::VOID: {
            float shimmer = sin(time * 3.0f + tx * 1.1f + ty * 0.7f) * 0.5f + 0.5f;
            return Color{
                (unsigned char)(VOID_WHITE.r + (VOID_SHIMMER.r - VOID_WHITE.r) * shimmer),
                (unsigned char)(VOID_WHITE.g + (VOID_SHIMMER.g - VOID_WHITE.g) * shimmer),
                (unsigned char)(VOID_WHITE.b + (VOID_SHIMMER.b - VOID_WHITE.b) * shimmer),
                255
            };
        }
        case TileType::HIDDEN: {
            float pulse = sin(time * 1.5f + tx + ty) * 0.3f + 0.7f;
            return Color{
                (unsigned char)(HIDDEN.r * pulse),
                (unsigned char)(HIDDEN.g * pulse),
                (unsigned char)(HIDDEN.b * pulse),
                180
            };
        }
    }
    return GROUND;
}
