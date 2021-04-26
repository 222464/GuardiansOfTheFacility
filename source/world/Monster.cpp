#include "Monster.h"
#include "MonsterEnv.h"
#include "Player.h"

const float moveRange = 1.0f;
const float texRectSize = 0.1f;
const float sizeDecay = 0.85;
const float side0Chance = 0.4f;
const float density = 10.0f;

const int sensorRes = 7;
const int actionRes = 5;

const float motorSpeed = 9.0f;
const float weakSpotChance = 0.25f;
const float weakSpotSize = 0.4f;

void Monster::init(
    int monsterLevel,
    MonsterEnv* env,
    const b2Vec2 &spawnPos,
    unsigned int seed,
    sf::SoundBuffer* popSound,
    sf::SoundBuffer* grossSound
) {
    std::mt19937 rng(seed);

    int maxLimbs = 6;

    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
    std::uniform_real_distribution<float> lengthDist(0.4f, 1.2f);
    std::uniform_real_distribution<float> widthDist(0.1f, 0.25f);
    std::uniform_real_distribution<float> angleDist(-0.2f, 0.2f);
    std::uniform_int_distribution<int> repeatDist(1, 6);
    std::uniform_int_distribution<int> branchDist(1, 3);
    std::uniform_real_distribution<float> texOffsetDist(0.0f, 1.0f - texRectSize);
    std::uniform_int_distribution<int> sideDist(0, 2);
    std::uniform_int_distribution<int> rootSideDist(0, 3);

    float branchBaseChance = 1.0f;
    float repeatBaseChance = 0.5f;
    float branchDecay = 0.7f;
    float repeatDecay = 0.6f;

    if (monsterLevel == 2) {
        maxLimbs = 16;

        branchDecay = 0.8f;
        repeatDecay = 0.7f;

        repeatDist = std::uniform_int_distribution<int>(1, 8);
        branchDist = std::uniform_int_distribution<int>(1, 5);
    }
    else if (monsterLevel == 3) {
        maxLimbs = 32;

        branchDecay = 0.9f;
        repeatDecay = 0.8f;

        repeatDist = std::uniform_int_distribution<int>(1, 10);
        branchDist = std::uniform_int_distribution<int>(1, 6);
    }

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
        fixtureDef.density = density;
        fixtureDef.friction = 1.0f;
        fixtureDef.restitution = 0.001f;
        
        root.body->CreateFixture(&fixtureDef);

        root.motorJoint = nullptr;

        root.texOffset = sf::Vector2f(texOffsetDist(rng), texOffsetDist(rng));

        root.hasWeakspot = true;

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

        if (dist01(subRng) < side0Chance)
            side = 0;

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
        fixtureDef.density = density;
        fixtureDef.friction = 1.0f;
        fixtureDef.restitution = 0.001f;
        
        next.body->CreateFixture(&fixtureDef);

        float volume = next.size.x * next.size.y;

        b2RevoluteJointDef motorJointDef;
        motorJointDef.Initialize(base.body, next.body, attachPosition);
        motorJointDef.collideConnected = false;
        motorJointDef.enableMotor = true;
        motorJointDef.maxMotorTorque = 300.0f * volume * density;
        motorJointDef.motorSpeed = 0.0f;

        next.motorJoint = static_cast<b2RevoluteJoint*>(env->world->CreateJoint(&motorJointDef));

        next.texOffset = sf::Vector2f(texOffsetDist(subRng), texOffsetDist(subRng));

        if (dist01(subRng) < weakSpotChance)
            next.hasWeakspot = true;

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
    
    aon::Array<aon::Hierarchy::LayerDesc> lds(1);

    for (int i = 0; i < lds.size(); i++) {
        lds[i].hiddenSize = aon::Int3(3, 3, 32);
        lds[i].errorSize = aon::Int3(3, 3, 32);

        lds[i].dRadius = 1;
        lds[i].bRadius = 1;
    }

    aon::Array<aon::Hierarchy::IODesc> ioDescs(3);

    int numMotors = limbs.size() - 1;

    int inputSizeRoot = std::ceil(std::sqrt(numMotors));
    ioDescs[0].size = aon::Int3(inputSizeRoot, inputSizeRoot, sensorRes);
    ioDescs[0].type = aon::prediction;
    ioDescs[0].dRadius = 1;
    ioDescs[0].bRadius = 1;
    ioDescs[1].size = aon::Int3(inputSizeRoot, inputSizeRoot, sensorRes);
    ioDescs[1].type = aon::prediction;
    ioDescs[1].dRadius = 1;
    ioDescs[1].bRadius = 1;
    ioDescs[2].size = aon::Int3(inputSizeRoot, inputSizeRoot, actionRes);
    ioDescs[2].type = aon::action;
    ioDescs[2].dRadius = 1;
    ioDescs[2].bRadius = 1;

    h.initRandom(ioDescs, lds);

    agentStep = 0;

    // Resources
    popBuffer = popSound;
    pop.setBuffer(*popBuffer);

    grossBuffer = grossSound;
    gross.setBuffer(*grossBuffer);
    gross.setLoop(true);
    gross.setVolume(20.0f);

    dead = false;
}

void Monster::step(
    World* world,
    bool simMode
) {
    if (!dead) {
        if (agentStep >= 3) {
            agentStep = 0;

            aon::IntBuffer sensors0(h.getInputSizes()[0].x * h.getInputSizes()[1].y, 0);
            aon::IntBuffer sensors1(sensors0.size(), 0);

            for (int i = 1; i < limbs.size(); i++) {
                sensors0[i - 1] = (std::min(moveRange, std::max(-moveRange, limbs[i].motorJoint->GetJointAngle())) / moveRange * 0.5f + 0.5f) * (sensorRes - 1) + 0.5f;
                sensors1[i - 1] = (std::min(pi, std::max(-pi, limbs[i].body->GetAngle())) / pi * 0.5f + 0.5f) * (sensorRes - 1) + 0.5f;
            }

            aon::Array<const aon::IntBuffer*> inputs(3);
            inputs[0] = &sensors0;
            inputs[1] = &sensors1;
            inputs[2] = &h.getPredictionCIs(2);

            float reward = -limbs[0].body->GetLinearVelocity().x;

            h.step(inputs, true, reward);
        }

        agentStep++;

        for (int i = 1; i < limbs.size(); i++) {
            float target = moveRange * (h.getPredictionCIs(2)[i - 1] / static_cast<float>(actionRes - 1) * 2.0f - 1.0f);

            limbs[i].motorJoint->SetMotorSpeed(motorSpeed * (target - limbs[i].motorJoint->GetJointAngle()));
        }
    }

    if (!simMode && !dead) {
        // Weak spot management
        int numWeakSpots = 0;

        for (int i = 0; i < limbs.size(); i++) {
            if (limbs[i].hasWeakspot) {
                numWeakSpots++;

                if (world->player->shot) {
                    sf::Vector2f centerPos(limbs[i].body->GetPosition().x, -limbs[i].body->GetPosition().y);
                    sf::Vector2f delta = centerPos - world->player->shootPos * renderScaleInv;

                    float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);

                    if (dist < weakSpotSize * 0.5f + 2.0f * renderScaleInv) {
                        limbs[i].hasWeakspot = false;

                        numWeakSpots--;

                        std::uniform_int_distribution<int> quadDist(0, 3);

                        world->splatters.push_back(sf::Vector3f(centerPos.x * renderScale, centerPos.y * renderScale, quadDist(world->rng) * 90.0f));

                        // Destroy sound
                        std::uniform_real_distribution<float> pitchDist(0.8f, 1.2f);
                        pop.setPitch(pitchDist(world->rng));
                        pop.play();
                    }
                }
            }
        }

        if (numWeakSpots == 0) {
            dead = true;

            gross.stop();

            // Go limp
            for (int i = 1; i < limbs.size(); i++) {
                limbs[i].motorJoint->SetMaxMotorTorque(0.01f);
            }
        }
        else {
            if (gross.getStatus() != sf::Sound::Playing)
                gross.play();
        }
    }
}

void Monster::render(
    sf::RenderTarget &window,
    sf::Texture* monsterTexture,
    sf::Texture* weakSpotTexture
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

    for (int i = 0; i < limbs.size(); i++) {
        if (limbs[i].hasWeakspot) {
            sf::Sprite s;
            s.setTexture(*weakSpotTexture);
            s.setScale(weakSpotSize / weakSpotTexture->getSize().x * renderScale, weakSpotSize / weakSpotTexture->getSize().y * renderScale);
            s.setOrigin(weakSpotTexture->getSize().x * 0.5f, weakSpotTexture->getSize().y * 0.5f);
            s.setPosition(limbs[i].body->GetPosition().x * renderScale, -limbs[i].body->GetPosition().y * renderScale);
            s.setRotation(-limbs[i].body->GetAngle() * 180.0f / pi);

            window.draw(s);
        }
    }
}

void Monster::move(
    const b2Vec2 &position
) {
    b2Vec2 delta = position - limbs[0].body->GetPosition();

    for (int i = 0; i < limbs.size(); i++)
        limbs[i].body->SetTransform(limbs[i].body->GetPosition() + delta, limbs[i].body->GetAngle());
}
