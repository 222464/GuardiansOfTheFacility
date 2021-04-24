#pragma once

#include <LDtkLoader/World.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Audio.hpp>
#include "ltbl/lighting/LightSystem.h"
#include <vector>
#include <memory>
#include <random>

const float pi = 3.141f;
const float renderScale = 4.0f;
const float renderScaleInv = 1.0f / renderScale;

class World {
public:
    ldtk::World world;
    ltbl::LightSystem ls;

    std::shared_ptr<class Player> player;

    const ldtk::Level* level;
    sf::VertexArray tilemap1;
    sf::VertexArray tilemap2;
    sf::VertexArray tilemap3;
    sf::Texture tileset;

    sf::Texture penumbraTexture;
    sf::Shader unshadowShader;
    sf::Shader lightOverShapeShader;

    sf::Texture spookyLightTexture;

    std::vector<std::shared_ptr<ltbl::LightPointEmission>> lights;
    std::vector<std::shared_ptr<ltbl::LightShape>> lightShapes;

    std::mt19937 rng;

    void init(
        const std::string &fileName,
        sf::RenderWindow &window
    );

    void update(
        sf::RenderWindow &window,
        float dt
    );

    void render(
        sf::RenderWindow &window
    );
};
