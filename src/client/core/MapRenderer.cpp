#include "MapRenderer.hpp"

MapRenderer::MapRenderer(std::shared_ptr<MapGenerator> mapGen, float tileSize): m_mapGen(mapGen), m_tileSize(tileSize){
    m_vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
}

void MapRenderer::render(sf::RenderTarget& target){
    sf::View view = target.getView();
    sf::FloatRect viewBounds(view.getCenter() - view.getSize() / 2.0f, view.getSize());

    target.draw(m_vertices);
}

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