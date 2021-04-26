#include "MonsterEnv.h"

const b2Vec2 gravity(0.0f, -9.81f);

void MonsterEnv::init(
    const sf::Vector2f &floorPos,
    sf::SoundBuffer* popSound,
    sf::SoundBuffer* grossSound
) {
    this->floorPos = b2Vec2(floorPos.x, floorPos.y);
    this->popSound = popSound;
    this->grossSound = grossSound;

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

    monster.init(this, spawn, seed, popSound, grossSound);
}

void MonsterEnv::step(
    float dt,
    World* world,
    bool simMode 
) {
    monster.step(world, simMode);

    this->world->Step(dt, 4, 4);
}

void MonsterEnv::addCollider(
    const sf::Vector2f &pos,
    const sf::Vector2f &size
) {
    b2PolygonShape shape;
    shape.SetAsBox(size.x * 0.5f, size.y * 0.5f);

    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.position = b2Vec2(pos.x, pos.y);

    b2Body* collider = world->CreateBody(&bodyDef);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.friction = 1.0f;
    fixtureDef.restitution = 0.001f;
    
    collider->CreateFixture(&fixtureDef);
}
