#include "World.h"
#include "Player.h"
#include "MonsterEnv.h"

void World::init(
    const std::string &fileName,
    sf::RenderWindow &window
) {
    world.loadFromFile(fileName);

    level = &world.getLevel(0);

    // Tiles 1
    {
        const ldtk::Layer &layer = level->getLayer("Tiles");

        const std::vector<ldtk::Tile> &tiles = layer.allTiles();

        tilemap1.resize(tiles.size() * 4);
        tilemap1.setPrimitiveType(sf::PrimitiveType::Quads);

        tileset.loadFromFile("resources/maps/" + layer.getTileset().path);

        int i = 0;

        for (const ldtk::Tile &tile : tiles) {
            for (int j = 0; j < 4 ; ++j) {
                tilemap1[i * 4 + j].position.x = tile.vertices[j].pos.x;
                tilemap1[i * 4 + j].position.y = tile.vertices[j].pos.y;
                tilemap1[i * 4 + j].texCoords.x = static_cast<float>(tile.vertices[j].tex.x);
                tilemap1[i * 4 + j].texCoords.y = static_cast<float>(tile.vertices[j].tex.y);
            }

            i++;
        }
    }

    // Tiles 2
    {
        const ldtk::Layer &layer = level->getLayer("Tiles2");

        const std::vector<ldtk::Tile> &tiles = layer.allTiles();

        tilemap2.resize(tiles.size() * 4);
        tilemap2.setPrimitiveType(sf::PrimitiveType::Quads);

        int i = 0;

        for (const ldtk::Tile &tile : tiles) {
            for (int j = 0; j < 4 ; ++j) {
                tilemap2[i * 4 + j].position.x = tile.vertices[j].pos.x;
                tilemap2[i * 4 + j].position.y = tile.vertices[j].pos.y;
                tilemap2[i * 4 + j].texCoords.x = static_cast<float>(tile.vertices[j].tex.x);
                tilemap2[i * 4 + j].texCoords.y = static_cast<float>(tile.vertices[j].tex.y);
            }

            i++;
        }
    }

    // Tiles 3
    {
        const ldtk::Layer &layer = level->getLayer("Tiles3");

        const std::vector<ldtk::Tile> &tiles = layer.allTiles();

        tilemap3.resize(tiles.size() * 4);
        tilemap3.setPrimitiveType(sf::PrimitiveType::Quads);

        int i = 0;

        for (const ldtk::Tile &tile : tiles) {
            for (int j = 0; j < 4 ; ++j) {
                tilemap3[i * 4 + j].position.x = tile.vertices[j].pos.x;
                tilemap3[i * 4 + j].position.y = tile.vertices[j].pos.y;
                tilemap3[i * 4 + j].texCoords.x = static_cast<float>(tile.vertices[j].tex.x);
                tilemap3[i * 4 + j].texCoords.y = static_cast<float>(tile.vertices[j].tex.y);
            }

            i++;
        }
    }

    penumbraTexture.loadFromFile("resources/ltbl/penumbraTexture.png");
    unshadowShader.loadFromFile("resources/ltbl/unshadowShader.vert", "resources/ltbl/unshadowShader.frag");
    lightOverShapeShader.loadFromFile("resources/ltbl/lightOverShapeShader.vert", "resources/ltbl/lightOverShapeShader.frag");
    spookyLightTexture.loadFromFile("resources/ltbl/spookyLightTexture.tga");

    ls.create(sf::FloatRect(0.0f, 0.0f, 512.0f, 512.0f), sf::Vector2u(window.getSize().x, window.getSize().y), penumbraTexture, unshadowShader, lightOverShapeShader);

    // Add entities
    const ldtk::Layer &entities = level->getLayer("Entities");

    for (const ldtk::Entity &entity : entities.getEntities("Light")) {
        std::shared_ptr<ltbl::LightPointEmission> light = std::make_shared<ltbl::LightPointEmission>();

        light->emissionSprite.setOrigin(spookyLightTexture.getSize().x * 0.5f, 64.0f);
        light->emissionSprite.setPosition(entity.getPosition().x * renderScale, entity.getPosition().y * renderScale);
        light->emissionSprite.setTexture(spookyLightTexture);
        light->emissionSprite.setScale(0.5f * renderScale, 0.5f * renderScale);
        light->emissionSprite.setColor(sf::Color(200, 200, 200, 255));
        light->localCastCenter = sf::Vector2f(0.0f, 0.0f);
        light->sourceRadius = 10.0f * renderScale;

        ls.addLight(light);
    }

    for (const ldtk::Entity &entity : entities.getEntities("Blocker")) {
        std::shared_ptr<ltbl::LightShape> shape = std::make_shared<ltbl::LightShape>();

        shape->shape.setPointCount(4);
        shape->shape.setPoint(0, sf::Vector2f(entity.getPosition().x, entity.getPosition().y) * renderScale);
        shape->shape.setPoint(1, sf::Vector2f(entity.getPosition().x + entity.getSize().x, entity.getPosition().y) * renderScale);
        shape->shape.setPoint(2, sf::Vector2f(entity.getPosition().x + entity.getSize().x, entity.getPosition().y + entity.getSize().y) * renderScale);
        shape->shape.setPoint(3, sf::Vector2f(entity.getPosition().x, entity.getPosition().y + entity.getSize().y) * renderScale);
        shape->renderLightOverShape = true;

        ls.addShape(shape);
    }

    player = std::make_shared<Player>();
    player->init(this, sf::Vector2f(200.0f, 16.0f * 13.0f));

    monsterTexture.loadFromFile("resources/textures/meat.png");

    env = std::make_shared<MonsterEnv>();
    env->init();
}

void World::update(
    sf::RenderWindow &window,
    float dt
) {
    player->update(this, window, dt);
}

void World::render(
    sf::RenderWindow &window
) {
    sf::RenderStates states;
    states.transform.scale(renderScale, renderScale);
    states.texture = &tileset;
    window.draw(tilemap1, states);
    window.draw(tilemap2, states);
    window.draw(tilemap3, states);

    player->render(this, window);
    env->monster.render(window, &monsterTexture);

    ls.render(window.getView(), unshadowShader, lightOverShapeShader);

    sf::Sprite lightSprite;
    lightSprite.setTexture(ls.getLightingTexture());
    
    sf::RenderStates lightStates = sf::RenderStates::Default;
    lightStates.blendMode = sf::BlendMultiply;
    
    sf::View oldView = window.getView();

    window.setView(window.getDefaultView());

    window.draw(lightSprite, lightStates);

    window.setView(oldView);

    player->renderPostLighting(this, window);
}
