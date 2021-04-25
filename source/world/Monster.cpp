#include "Monster.h"
#include "MonsterEnv.h"

const int maxLimbs = 30;
const float moveRange = 0.5f;
const float texRectSize = 0.1f;

void Monster::init(
    MonsterEnv* env,
    const b2Vec2 &spawnPos,
    unsigned int seed
) {
    std::mt19937 rng(seed);

    std::uniform_real_distribution<float> lengthDist(0.2f, 1.0f);
    std::uniform_real_distribution<float> widthDist(0.05f, 0.5f);
    std::uniform_real_distribution<float> angleDist(0.0f, pi * 2.0f);
    std::uniform_int_distribution<int> repeatDist(0, 4);
    std::uniform_int_distribution<int> branchDist(0, 3);
    std::uniform_real_distribution<float> texOffsetDist(0.0f, 1.0f - texRectSize);

    limbs.clear();

    {
        Limb root;

        b2PolygonShape shape;
        root.size = sf::Vector2f(lengthDist(rng), widthDist(rng));
        shape.SetAsBox(root.size.x * 0.5f, root.size.y * 0.5f);

        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = spawnPos;
        bodyDef.angle = angleDist(rng);
        bodyDef.allowSleep = false;

        root.body = env->world->CreateBody(&bodyDef);

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &shape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 1.0f;
        fixtureDef.restitution = 0.001f;
        
        root.body->CreateFixture(&fixtureDef);

        root.motorJoint = nullptr;

        root.texOffset = sf::Vector2f(texOffsetDist(rng), texOffsetDist(rng));

        limbs.push_back(root);
    }

    std::function<int(int, unsigned int)> limbGen = [&](int baseIndex, unsigned int subSeed) -> int {
        if (limbs.size() >= maxLimbs)
            return limbs.size() - 1;

        std::mt19937 subRng(subSeed);

        Limb &base = limbs[baseIndex];

        Limb next;

        next.parent = &base;

        b2PolygonShape shape;
        next.size = sf::Vector2f(lengthDist(subRng), widthDist(subRng));
        shape.SetAsBox(next.size.x * 0.5f, next.size.y * 0.5f);

        float angle = angleDist(subRng);
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = base.body->GetPosition() + b2Vec2(std::cos(angle) * next.size.x * 0.5f, std::sin(angle) * next.size.x * 0.5f);
        bodyDef.angle = base.body->GetAngle() + angle;
        bodyDef.allowSleep = false;

        next.body = env->world->CreateBody(&bodyDef);

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &shape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 1.0f;
        fixtureDef.restitution = 0.001f;
        
        next.body->CreateFixture(&fixtureDef);

        b2MotorJointDef motorJointDef;
        motorJointDef.Initialize(base.body, next.body);
        motorJointDef.collideConnected = false;

        next.motorJoint = static_cast<b2MotorJoint*>(env->world->CreateJoint(&motorJointDef));

        next.texOffset = sf::Vector2f(texOffsetDist(subRng), texOffsetDist(subRng));

        limbs.push_back(next);

        int lastIndex = limbs.size() - 1;

        int branches = branchDist(subRng);

        for (int b = 0; b < branches; b++)
            limbGen(lastIndex, subSeed + (1 + b) * 12345);

        int repeats = repeatDist(subRng);

        for (int r = 0; r < repeats; r++)
            lastIndex = limbGen(lastIndex, subSeed + 12345);

        return lastIndex;
    };

    std::uniform_int_distribution<int> seedDist(0, 99999);

    limbGen(0, seedDist(rng));
}

void Monster::step(
    float reward
) {
}

void Monster::render(
    sf::RenderWindow &window,
    sf::Texture* monsterTexture
) {
    for (int i = 0; i < limbs.size(); i++) {
        sf::RectangleShape rs;
        rs.setTexture(monsterTexture);
        rs.setSize(limbs[i].size * renderScale);
        rs.setOrigin(limbs[i].size * 0.5f);
        rs.setPosition(limbs[i].body->GetPosition().x * renderScale, -limbs[i].body->GetPosition().y * renderScale);
        rs.setRotation(limbs[i].body->GetAngle() * 180.0f / pi);
        rs.setTextureRect(sf::IntRect(monsterTexture->getSize().x * limbs[i].texOffset.x, monsterTexture->getSize().y * limbs[i].texOffset.y, texRectSize * monsterTexture->getSize().x, texRectSize * monsterTexture->getSize().y));

        window.draw(rs);
    }
}
