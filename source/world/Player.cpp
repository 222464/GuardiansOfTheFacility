#include "Player.h"

const float shootTime = 1.0f;
const float flashRatio = 0.8f;

void Player::init(
    World* world,
    const sf::Vector2f &spawn
) {
    position = spawn;

    standing.loadFromFile("resources/textures/soldier.png");
    walking.resize(8);

    for (int i = 0; i < walking.size(); i++)
        walking[i].loadFromFile("resources/textures/soldier_walk" + std::to_string(i + 1) + ".png");

    walkFrame = 0;
    walkTimer = 0.0f;
    direction = 0;
    flipped = false;
    shootTimer = 0.0f;
    casingAlpha = 0.0f;
    shootPos = sf::Vector2f(0.0f, 0.0f);
    shot = false;

    firing.loadFromFile("resources/textures/soldier_firing.png");
    firingEmissive.loadFromFile("resources/textures/soldier_firing_emissive.png");
    lightTex.loadFromFile("resources/ltbl/pointLightTexture.png");

    light = std::make_shared<ltbl::LightPointEmission>();

    light->emissionSprite.setTexture(lightTex);
    light->emissionSprite.setOrigin(lightTex.getSize().x * 0.5f, lightTex.getSize().y * 0.5f);
    light->emissionSprite.setPosition(position);
    light->emissionSprite.setScale(2.0f , 2.0f );
    light->emissionSprite.setColor(sf::Color(50, 100, 50, 255));
    light->sourceRadius = 10.0f ;

    world->ls.addLight(light);

    crosshair.loadFromFile("resources/textures/crosshair.png");

    shootSoundBuffer.loadFromFile("resources/sounds/shoot.wav");
    shootSound.setBuffer(shootSoundBuffer);

    flash = std::make_shared<ltbl::LightPointEmission>();

    flash->emissionSprite.setTexture(lightTex);
    flash->emissionSprite.setOrigin(lightTex.getSize().x * 0.5f, lightTex.getSize().y * 0.5f);
    flash->emissionSprite.setPosition(position);
    flash->emissionSprite.setScale(3.0f , 3.0f );
    flash->sourceRadius = 10.0f;

    world->ls.addLight(flash);
}

void Player::update(
    World* world,
    sf::RenderWindow &window,
    float dt
) {
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    const float speed = 80.0f;
    const float walkAgainTime = shootTime * 0.5f;

    direction = 0;

    shot = false;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && shootTimer == 0.0f) {
        shootTimer = shootTime;

        casingAlpha = 1.0f;
        casingPos = position + sf::Vector2f(flipped ? -10.0f : 10.0f, -36.0f);
        casingAngle = 0.0f;

        std::normal_distribution<float> nDist(0.0f, 1.0f);

        casingLVel = sf::Vector2f(nDist(world->rng) * 20.0f, -nDist(world->rng) * 30.0f - 60.0f);
        casingRVel = nDist(world->rng) * 16.0f;

        shootSound.play();

        shootPos = mousePos;
        shot = true;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && shootTimer < walkAgainTime) {
        position.x += dt * speed;
        direction = 1;
    }
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && shootTimer < walkAgainTime) {
        position.x -= dt * speed;
        direction = -1;
    }

    if (shootTimer > 0.0f) {
        shootTimer -= dt;

        float ratio = std::min(1.0f, std::max(0.0f, shootTimer / shootTime * 4.0f - 3.5f));
        flash->emissionSprite.setColor(sf::Color(255.0f * ratio, 255.0f * ratio, 200.0f * ratio, 255));
    }
    else {
        shootTimer = 0.0f;
        flash->emissionSprite.setColor(sf::Color(0, 0, 0, 255));
    }

    const float wallDist = 101.0f;

    if (position.x < wallDist)
        position.x = wallDist;
    else if (position.x > world->level->size.x - wallDist)
        position.x = world->level->size.x - wallDist;

    light->emissionSprite.setPosition((position + sf::Vector2f(0.0f, -38.0f)) );
    light->quadtreeUpdate();

    flash->emissionSprite.setPosition((position + sf::Vector2f(flipped ? -10.0f : 10.0f, -36.0f)) );
    flash->quadtreeUpdate();

    flipped = mousePos.x < position.x ;

    if (direction != 0) {
        walkTimer += dt;

        if (walkTimer > 0.05f) {
            walkTimer = 0.0f;

            if (direction > 0 && !flipped || direction < 0 && flipped) {
                walkFrame++;

                if (walkFrame >= walking.size())
                    walkFrame = 0;
            }
            else {
                walkFrame--;

                if (walkFrame < 0)
                    walkFrame = walking.size() - 1;
            }
        }
    }

    if (casingAlpha > 0.0f) {
        casingAlpha -= dt * 1.1f;

        casingLVel.y += 200.0f * dt;
        casingPos += casingLVel * dt;
        casingAngle += casingRVel * dt;
    }
    else
        casingAlpha = 0.0f;
}

void Player::render(
    World* world,
    sf::RenderWindow &window
) {
    sf::Sprite s;

    if (direction == 0)
        s.setTexture(shootTimer > shootTime * flashRatio ? firing : standing);
    else
        s.setTexture(walking[walkFrame]);

    s.setScale(flipped ? -1.0f : 1.0f, 1.0f);

    s.setOrigin(shootTimer > shootTime * flashRatio ? standing.getSize().x * 0.5f - 3.0f : standing.getSize().x * 0.5f, standing.getSize().y);
    s.setPosition(position );

    window.draw(s);

    if (casingAlpha > 0.0f) {
        sf::RectangleShape rs;
        rs.setSize(sf::Vector2f(2.0f, 1.0f));
        rs.setFillColor(sf::Color(240, 240, 150, 255 * std::sqrt(casingAlpha)));
        rs.setOrigin(1.0f, 0.5f);
        rs.setPosition(casingPos );
        rs.setRotation(casingAngle * 180.0f / pi);

        window.draw(rs);
    }
}

void Player::renderPostLighting(
    World* world,
    sf::RenderWindow &window
) {
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    sf::Sprite chs;
    chs.setTexture(crosshair);
    chs.setOrigin(crosshair.getSize().x * 0.5f, crosshair.getSize().y * 0.5f);
    chs.setPosition(mousePos);

    window.draw(chs);

    sf::Sprite s;

    if (direction == 0 && shootTimer > shootTime * flashRatio) {
        s.setTexture(firingEmissive);

        s.setScale(flipped ? -1.0f : 1.0f, 1.0f);

        s.setOrigin(standing.getSize().x * 0.5f - 3.0f, standing.getSize().y);
        s.setPosition(position );

        window.draw(s);
    }
}
