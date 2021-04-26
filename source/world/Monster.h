#pragma once

#include "World.h"
#include <SFML/Graphics.hpp>
#include <aogmaneo/Hierarchy.h>
#include <box2d/box2d.h>

class Monster {
public:
    struct Limb {
        sf::Vector2f size;
        sf::Vector2f texOffset;

        Limb* parent;

        b2Body* body;

        b2RevoluteJoint* motorJoint;

        float targetAngle;

        // Gameplay
        bool hasWeakspot;

        Limb()
        :
        parent(nullptr),
        targetAngle(0.0f),
        hasWeakspot(false)
        {}
    };

private:
    sf::Texture meat;

    std::vector<Limb> limbs;

    aon::Hierarchy h;

    sf::SoundBuffer* popBuffer;
    sf::Sound pop;
    sf::SoundBuffer* grossBuffer;
    sf::Sound gross;

    int agentStep;
    
public:
    bool dead;

    void init(
        int monsterLevel,
        class MonsterEnv* env,
        const b2Vec2 &spawnPos,
        unsigned int seed,
        sf::SoundBuffer* popSound,
        sf::SoundBuffer* grossSound
    );

    void step(
        World* world,
        bool simMode = false
    );

    void render(
        sf::RenderTarget &window,
        sf::Texture* monsterTexture,
        sf::Texture* weakSpotTexture
    );

    void move(
        const b2Vec2 &position
    );

    const b2Vec2 &getPosition() const {
        return limbs[0].body->GetPosition();
    }
};
