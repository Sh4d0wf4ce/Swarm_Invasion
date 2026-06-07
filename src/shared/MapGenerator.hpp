#pragma once
#include "Config.hpp"
#include <SFML/System.hpp>
#include <vector>
#include <random>


/**
 * @brief Tile types used in procedurally generated dungeon maps.
 */
enum class TileType{
    Floor,
    Wall
};

/**
 * @brief Procedural cave-style map generator with wall smoothing and collision queries.
 */
class MapGenerator{
public:
    /**
     * @brief Constructs a map generator with the given tile grid dimensions.
     * @param width Map width in tiles.
     * @param height Map height in tiles.
     */
    MapGenerator(int width, int height);

    /**
     * @brief Generates a new map layout using the given seed.
     * @param seed Random seed for reproducible map generation.
     */
    void generate(int seed);

    /**
     * @brief Returns the tile type at grid coordinates.
     * @param x Tile column index.
     * @param y Tile row index.
     * @return Tile at (@p x, @p y), or TileType::Wall for out-of-bounds coordinates.
     */
    TileType getTile(int x, int y) const;

    /**
     * @brief Returns the map width in tiles.
     * @return Map width.
     */
    int getWidth() const {return m_width;}

    /**
     * @brief Returns the map height in tiles.
     * @return Map height.
     */
    int getHeight() const {return m_height;}

    /**
     * @brief Tests whether a circular entity overlaps any wall tile.
     * @param pos World-space center position of the entity.
     * @param radius Collision radius of the entity in world units.
     * @return True if any sample point intersects a wall tile.
     */
    bool checkCollision(const sf::Vector2f& pos, float radius) const;

private:
    /** Applies cellular-automata smoothing to reduce noise in the tile grid. */
    void smoothMap();

    /**
     * @brief Counts wall tiles in the 3x3 neighborhood around a grid cell.
     * @param gridX Tile column index.
     * @param gridY Tile row index.
     * @return Number of neighboring wall tiles, treating out-of-bounds as walls.
     */
    int getSurroundingWallCount(int gridX, int gridY) const;

    int m_width;
    int m_height;
    int m_fillPercent;
    std::vector<std::vector<TileType>> m_map;
};
