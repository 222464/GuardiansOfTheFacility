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

        Limb()
        :
        parent(nullptr),
        targetAngle(0.0f)
        {}
    };

private:
    sf::Texture meat;

    std::vector<Limb> limbs;
    
public:
    void init(
        class MonsterEnv* env,
        const b2Vec2 &spawnPos,
        unsigned int seed
    );

    void step(
        float reward
    );

    void render(
        sf::RenderTarget &window,
        sf::Texture* monsterTexture
    );
};
