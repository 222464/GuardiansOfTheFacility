#include "Monster.h"

class MonsterEnv {
public:
    float reward;
    Monster monster;
    sf::SoundBuffer* popSound;
    sf::SoundBuffer* grossSound;

    unsigned int seed;
    b2Vec2 spawn;
    b2Vec2 floorPos;

    std::shared_ptr<b2World> world;

    MonsterEnv()
    :
    seed(12345),
    spawn(0.0f, 0.0f)
    {}

    void init(
        const sf::Vector2f &floorPos,
        sf::SoundBuffer* popSound,
        sf::SoundBuffer* grossSound
    );

    void reset();

    void moveToSpawn() {
        monster.move(spawn);
    }

    void step(
        float dt,
        World* world,
        bool simMode = false
    );

    void addCollider(
        const sf::Vector2f &pos,
        const sf::Vector2f &size
    );
};
