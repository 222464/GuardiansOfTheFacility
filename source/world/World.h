#pragma once

#include <LDtkLoader/World.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Audio.hpp>
#include "ltbl/lighting/LightSystem.h"
#include <vector>
#include <memory>
#include <random>

const float pi = 3.141f;
const float renderScale = 16.0f;
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
    sf::Texture monsterTexture;
    sf::Shader monsterRenderShader;
    sf::RenderTexture monsterRenderTexture;
    sf::SoundBuffer monsterPopSound;
    sf::Texture weakSpotTexture;

    std::vector<sf::Vector3f> splatters;
    sf::Texture splatter;

    std::vector<std::shared_ptr<ltbl::LightPointEmission>> lights;
    std::vector<std::shared_ptr<ltbl::LightShape>> lightShapes;

    std::mt19937 rng;

    std::shared_ptr<class MonsterEnv> env;

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
