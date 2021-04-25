#include "Monster.h"
#include "MonsterEnv.h"

const int maxLimbs = 20;
const float moveRange = 1.0f;
const float texRectSize = 0.1f;
const float sizeDecay = 0.92f;
const float branchBaseChance = 1.0f;
const float repeatBaseChance = 0.5f;
const float branchDecay = 0.9f;
const float repeatDecay = 0.8f;

const int sensorRes = 9;
const int actionRes = 7;

const float motorSpeed = 1.0f;

void Monster::init(
    MonsterEnv* env,
    const b2Vec2 &spawnPos,
    unsigned int seed
) {
    std::mt19937 rng(seed);

    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
    std::uniform_real_distribution<float> lengthDist(0.2f, 1.0f);
    std::uniform_real_distribution<float> widthDist(0.05f, 0.5f);
    std::uniform_real_distribution<float> angleDist(-0.5f, 0.5f);
    std::uniform_int_distribution<int> repeatDist(1, 5);
    std::uniform_int_distribution<int> branchDist(1, 3);
    std::uniform_real_distribution<float> texOffsetDist(0.0f, 1.0f - texRectSize);
    std::uniform_int_distribution<int> sideDist(0, 2);
    std::uniform_int_distribution<int> rootSideDist(0, 3);

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

    std::function<int(int, unsigned int, int)> limbGen = [&](int baseIndex, unsigned int subSeed, int depth) -> int {
        if (limbs.size() >= maxLimbs)
            return limbs.size() - 1;

        std::mt19937 subRng(subSeed);

        Limb &base = limbs[baseIndex];

        Limb next;

        next.parent = &base;

        float sizeMult = std::pow(sizeDecay, depth);

        b2PolygonShape shape;
        next.size = sf::Vector2f(lengthDist(subRng) * sizeMult, widthDist(subRng) * sizeMult);
        shape.SetAsBox(next.size.x * 0.5f, next.size.y * 0.5f);

        float angle = angleDist(subRng);

        int side;

        if (baseIndex == 0)
            side = rootSideDist(subRng);
        else
            side = sideDist(subRng);

        b2Vec2 attachDelta;

        switch (side) {
        case 0:
            attachDelta = b2Vec2(std::cos(base.body->GetAngle()) * base.size.x * 0.5f, std::sin(base.body->GetAngle()) * base.size.x * 0.5f);

            break;

        case 1:
            attachDelta = b2Vec2(std::cos(base.body->GetAngle() + 0.5f * pi) * base.size.y * 0.5f, std::sin(base.body->GetAngle() + 0.5f * pi) * base.size.y * 0.5f);
            angle += 0.5f * pi;
            break;

        case 2:
            attachDelta = b2Vec2(std::cos(base.body->GetAngle() - 0.5f * pi) * base.size.y * 0.5f, std::sin(base.body->GetAngle() - 0.5f * pi) * base.size.y * 0.5f);
            angle -= 0.5f * pi;
            break;
        case 3:
            attachDelta = b2Vec2(std::cos(base.body->GetAngle() + pi) * base.size.x * 0.5f, std::sin(base.body->GetAngle() + pi) * base.size.x * 0.5f);
            angle += pi;
            break;
        }

        b2Vec2 attachPosition = base.body->GetPosition() + attachDelta;

        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.angle = base.body->GetAngle() + angle;
        bodyDef.position = attachPosition + b2Vec2(std::cos(bodyDef.angle) * next.size.x * 0.5f, std::sin(bodyDef.angle) * next.size.x * 0.5f);
        bodyDef.allowSleep = false;

        next.body = env->world->CreateBody(&bodyDef);

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &shape;
        fixtureDef.density = 1.0f;
        fixtureDef.friction = 1.0f;
        fixtureDef.restitution = 0.001f;
        
        next.body->CreateFixture(&fixtureDef);

        b2RevoluteJointDef motorJointDef;
        motorJointDef.Initialize(base.body, next.body, attachPosition);
        motorJointDef.collideConnected = false;
        motorJointDef.enableMotor = true;
        motorJointDef.maxMotorTorque = 10.0f;
        motorJointDef.motorSpeed = 0.0f;

        next.motorJoint = static_cast<b2RevoluteJoint*>(env->world->CreateJoint(&motorJointDef));

        next.texOffset = sf::Vector2f(texOffsetDist(subRng), texOffsetDist(subRng));

        limbs.push_back(next);

        int lastIndex = limbs.size() - 1;

        int branches = 0;

        if (dist01(subRng) < branchBaseChance * std::pow(branchDecay, depth))
            branches = branchDist(subRng);

        for (int b = 0; b < branches; b++)
            limbGen(lastIndex, subSeed + (1 + b) * 12345, depth + 1);

        int repeats = 0;

        if (dist01(subRng) < repeatBaseChance * std::pow(repeatDecay, depth))
            repeats = repeatDist(subRng);

        for (int r = 0; r < repeats; r++)
            lastIndex = limbGen(lastIndex, subSeed, depth + 1);

        return lastIndex;
    };

    std::uniform_int_distribution<int> seedDist(0, 99999);

    int startBranches = branchDist(rng);

    for (int b = 0; b < startBranches; b++)
        limbGen(0, seedDist(rng), 1);

    // Agent
    
    aon::Array<aon::Hierarchy::LayerDesc> lds(2);

    for (int i = 0; i < lds.size(); i++) {
        lds[i].hiddenSize = aon::Int3(4, 4, 16);
        lds[i].errorSize = aon::Int3(4, 4, 16);
    }

    aon::Array<aon::Hierarchy::IODesc> ioDescs(2);

    int numMotors = limbs.size() - 1;

    int inputSizeRoot = std::ceil(std::sqrt(numMotors));
    ioDescs[0].size = aon::Int3(inputSizeRoot, inputSizeRoot, sensorRes);
    ioDescs[0].type = aon::prediction;
    ioDescs[1].size = aon::Int3(inputSizeRoot, inputSizeRoot, actionRes);
    ioDescs[1].type = aon::action;

    h.initRandom(ioDescs, lds);
}

void Monster::step(
    float reward
) {
    aon::IntBuffer sensors(h.getInputSizes()[0].x * h.getInputSizes()[1].y, 0);

    for (int i = 1; i < limbs.size(); i++)
        sensors[i - 1] = (std::min(moveRange, std::max(-moveRange, limbs[i].motorJoint->GetJointAngle())) / moveRange * 0.5f + 0.5f) * (sensorRes - 1) + 0.5f;

    aon::Array<const aon::IntBuffer*> inputs(2);
    inputs[0] = &sensors;
    inputs[1] = &h.getPredictionCIs(1);

    h.step(inputs, true, reward);

    for (int i = 1; i < limbs.size(); i++) {
        float target = moveRange * (h.getPredictionCIs(1)[i - 1] / static_cast<float>(actionRes - 1) * 2.0f - 1.0f);

        limbs[i].motorJoint->SetMotorSpeed(motorSpeed * (target - limbs[i].motorJoint->GetJointAngle()));
    }

    reward = -limbs[0].body->GetLinearVelocity().x;
}

void Monster::render(
    sf::RenderTarget &window,
    sf::Texture* monsterTexture
) {
    for (int i = 0; i < limbs.size(); i++) {
        sf::RectangleShape rs;
        rs.setTexture(monsterTexture);
        rs.setSize(limbs[i].size * renderScale);
        rs.setOrigin(limbs[i].size * 0.5f * renderScale);
        rs.setPosition(limbs[i].body->GetPosition().x * renderScale, -limbs[i].body->GetPosition().y * renderScale);
        rs.setRotation(-limbs[i].body->GetAngle() * 180.0f / pi);
        rs.setTextureRect(sf::IntRect(monsterTexture->getSize().x * limbs[i].texOffset.x, monsterTexture->getSize().y * limbs[i].texOffset.y, texRectSize * monsterTexture->getSize().x, texRectSize * monsterTexture->getSize().y));

        window.draw(rs);
    }
}
