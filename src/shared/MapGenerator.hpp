#pragma once

#include <vector>

enum class TileType{
    Floor,
    Wall
};

class MapGenerator{
public:
    MapGenerator(int width, int height);

    void generate(int seed);

    TileType getTile(int x, int y) const;
    int getWidth() const {return m_width;}
    int getHeight() const {return m_height;}

private:
    void smoothMap();
    int getSurroundingWallCount(int gridX, int gridY) const;

    int m_width;
    int m_height;
    int m_fillPercent;

    std::vector<std::vector<TileType>> m_map;
};
