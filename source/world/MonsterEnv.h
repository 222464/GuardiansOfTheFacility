#include "Monster.h"

class MonsterEnv {
public:
    float reward;
    Monster monster;
    sf::SoundBuffer* popSound;

    unsigned int seed;
    b2Vec2 spawn;
    b2Vec2 floorPos;

    std::shared_ptr<b2World> world;

    MonsterEnv()
    :
    spawn(0.0f, 0.0f)
    {}

    void init(
        const sf::Vector2f &floorPos,
        sf::SoundBuffer* popSound
    );

    void reset();

    void step(
        float dt,
        World* world,
        bool simMode = false
    );
};
