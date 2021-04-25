#include "Monster.h"

class MonsterEnv {
public:
    float reward;
    Monster monster;

    unsigned int seed;

    std::shared_ptr<b2World> world;

    void init();

    void reset();

    void step();
};
