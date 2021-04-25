#include "MonsterEnv.h"

const b2Vec2 gravity(0.0f, -9.81f);

void MonsterEnv::init(
    const sf::Vector2f &floorPos
) {
    seed = 1234;
    reward = 0.0f;
    this->floorPos = b2Vec2(floorPos.x, floorPos.y);

    reset();
}

void MonsterEnv::reset() {
    world = std::make_shared<b2World>(gravity);

    b2PolygonShape shape;
    shape.SetAsBox(1000.0f, 4.0f);

    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.position = floorPos - b2Vec2(0.0f, 4.0f);

    b2Body* floor = world->CreateBody(&bodyDef);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.friction = 1.0f;
    fixtureDef.restitution = 0.001f;
    
    floor->CreateFixture(&fixtureDef);

    monster.init(this, spawn, seed);
}

void MonsterEnv::step(
    float dt
) {
    monster.step(reward);

    world->Step(dt, 8, 8);
}
