#pragma once

#include "World.h"
#include <SFML/Graphics.hpp>

class Player {
private:    
    sf::Texture standing;
    std::vector<sf::Texture> walking;
    sf::Texture firing;
    sf::Texture firingEmissive;
    sf::Texture lightTex;

    sf::Vector2f position;

    std::shared_ptr<ltbl::LightPointEmission> light;

    int walkFrame;
    float walkTimer;
    int direction;
    bool flipped;
    float shootTimer;

    sf::Texture crosshair;

    sf::SoundBuffer shootSoundBuffer;
    sf::Sound shootSound;

    std::shared_ptr<ltbl::LightPointEmission> flash;

    float casingAlpha;
    sf::Vector2f casingPos;
    float casingAngle;
    sf::Vector2f casingLVel;
    float casingRVel;

public:
    void init(
        World* world,
        const sf::Vector2f &spawn
    );

    void update(
        World* world,
        sf::RenderWindow &window,
        float dt
    );

    void render(
        World* world,
        sf::RenderWindow &window
    );

    void renderPostLighting(
        World* world,
        sf::RenderWindow &window
    );
};
