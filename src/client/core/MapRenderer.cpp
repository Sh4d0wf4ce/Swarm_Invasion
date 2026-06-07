#include "MapRenderer.hpp"

/**
 * @brief Constructs a map renderer bound to a map generator and tile size.
 * @param mapGen Shared map data source used to build vertex geometry.
 * @param tileSize World-space size of one map tile in pixels.
 */
MapRenderer::MapRenderer(std::shared_ptr<MapGenerator> mapGen, float tileSize): m_mapGen(mapGen), m_tileSize(tileSize){
    m_vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
}

/**
 * @brief Draws the cached map vertices to the given render target.
 * @param target SFML surface that receives the vertex array draw call.
 */
void MapRenderer::render(sf::RenderTarget& target){
    sf::View view = target.getView();
    sf::FloatRect viewBounds(view.getCenter() - view.getSize() / 2.0f, view.getSize());
    target.draw(m_vertices);
}

/**
 * @brief Rebuilds the vertex cache from the current map generator data.
 *
 * Each tile becomes two triangles colored by tile type (wall vs floor).
 * Does nothing if no map generator is bound.
 */
void MapRenderer::rebuild(){
    if(!m_mapGen) return;
    int width = m_mapGen->getWidth();
    int height = m_mapGen->getHeight();
    m_vertices.resize(width * height * 6);
    for(int x = 0; x < width; x++){
        for(int y = 0; y < height; y++){
            int idx = (x + y * width) * 6;
            sf::Color color = (m_mapGen->getTile(x, y) == TileType::Wall) ? sf::Color(80, 80, 80) : sf::Color(150, 150, 150);
            float left = x * m_tileSize;
            float right = left + m_tileSize;
            float top = y * m_tileSize;
            float bottom = top + m_tileSize;
            m_vertices[idx + 0] = sf::Vertex{{left, top}, color};
            m_vertices[idx + 1] = sf::Vertex{{right, top}, color};
            m_vertices[idx + 2] = sf::Vertex{{left, bottom}, color};
            m_vertices[idx + 3] = sf::Vertex{{left, bottom}, color};
            m_vertices[idx + 4] = sf::Vertex{{right, top}, color};
            m_vertices[idx + 5] = sf::Vertex{{right, bottom}, color};
        }
    }
}
