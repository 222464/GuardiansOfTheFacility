#include "MonsterEnv.h"

const b2Vec2 gravity(0.0f, -9.81f);

void MonsterEnv::init(
) {
    seed = 1234;
    reward = 0.0f;

    reset();
}

void MonsterEnv::reset() {
    world = std::make_shared<b2World>(gravity);

    monster.init(this, b2Vec2(100.0f, -10.0f), seed);
}

void MonsterEnv::step() {
    monster.step(reward);
}
