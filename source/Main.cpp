#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <aogmaneo/Hierarchy.h>

#include "world/World.h"
#include "world/Player.h"
#include "world/MonsterEnv.h"

int main() {
    aon::setNumThreads(8);

    int frameRate = 60;

    sf::RenderWindow window(sf::VideoMode(1200, 900), "SFML window");

    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(frameRate);

    window.setMouseCursorVisible(false);

    float dt = 1.0f / frameRate;

    int levelIndex = 0;

    std::mt19937 rng(time(nullptr));

    std::uniform_int_distribution<int> seedDist(0, 99999);

    std::unique_ptr<World> world = std::make_unique<World>();
    world->init("resources/maps/map1.ldtk", window, seedDist(rng));

    sf::Font msgFont;
    msgFont.loadFromFile("resources/terminal.ttf");
    sf::Text msg;
    msg.setFont(msgFont);
    msg.setFillColor(sf::Color::Green);
    int dots = 0;

    int pretrainTime = 300;
    int pretrainTimer = 0;

    sf::View view = window.getDefaultView();
    view.zoom(0.2f);
    //sf::View newView = view;

    float zoomRate = 0.25f;
    float viewInterpolateRate = 20.0f;

    sf::Vector2i prevMousePos = sf::Mouse::getPosition(window);

    sf::Clock globalClock;

    sf::Color backgroundColor = sf::Color::Black;
    bool gameFinished = false;
    int numLevels = 6;

    sf::SoundBuffer screamBuffer;
    screamBuffer.loadFromFile("resources/sounds/scream.wav");

    sf::Sound screamSound;
    screamSound.setBuffer(screamBuffer);

    while (window.isOpen()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        if (window.hasFocus()) {
            sf::Event event;

            while (window.pollEvent(event)) {
                switch (event.type) {
                case sf::Event::Closed:
                    window.close();

                    break;

                //case sf::Event::MouseWheelMoved:
                //    int dWheel = event.mouseWheel.delta;

                //    newView = view;

                //    window.setView(newView);

                //    sf::Vector2f mouseZoomDelta0 = window.mapPixelToCoords(mousePos);

                //    newView.setSize(view.getSize() + newView.getSize() * (-zoomRate * dWheel));

                //    window.setView(newView);

                //    sf::Vector2f mouseZoomDelta1 = window.mapPixelToCoords(mousePos);

                //    window.setView(view);

                //    newView.setCenter(view.getCenter() + mouseZoomDelta0 - mouseZoomDelta1);

                //    break;
                }
            }

            //sf::Time duration = globalClock.restart();

            //view.setCenter(view.getCenter() + (newView.getCenter() - view.getCenter()) * viewInterpolateRate * dt);
            //view.setSize(view.getSize() + (newView.getSize() - view.getSize()) * viewInterpolateRate * dt);

            //if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            //    sf::Vector2i dPos = mousePos - prevMousePos;

            //    view.setCenter(view.getCenter() - sf::Vector2f(dPos.x * (static_cast<float>(view.getSize().x) / window.getSize().x), dPos.y * (static_cast<float>(view.getSize().x) / window.getSize().x)));

            //    newView.setCenter(view.getCenter());
            //}
        }

        //prevMousePos = mousePos;

        view.setCenter(sf::Vector2f(world->player->getPosition().x + 80.0f, world->player->getPosition().y - 30.0f));
        window.setView(view);

        if (pretrainTimer < pretrainTime) {
            world->pretrain(dt);

            pretrainTimer++;

            if (pretrainTimer == pretrainTime) {
                world->start();
                backgroundColor = sf::Color::Black;
            }
        }
        else {
            if (!world->levelFailed)
                world->update(window, dt);
        }

        if (world->levelClear) {
            // Start new level
            levelIndex++;

            if (levelIndex >= numLevels) {
                gameFinished = true;
            }

            world = std::make_unique<World>();
            world->init("resources/maps/map" + std::to_string(levelIndex + 1) + ".ldtk", window, levelIndex / 2 + 1, seedDist(rng));
            pretrainTimer = 0;
        }

        if (world->levelFailed) {
            // Start new level
            levelIndex = 0;
            world = std::make_unique<World>();
            world->init("resources/maps/map" + std::to_string(levelIndex + 1) + ".ldtk", window, levelIndex / 2 + 1, seedDist(rng));
            pretrainTimer = 0;

            backgroundColor = sf::Color::Red;

            screamSound.play();
        }

        window.clear(backgroundColor);

        if (pretrainTimer < pretrainTime) {
            window.setView(window.getDefaultView());

            if (pretrainTimer % 20 == 0) {
                dots++;

                if (dots > 3)
                    dots = 0;
            }

            switch (dots) {
            case 0:
                msg.setString("Reanimating");
                break;
            case 1:
                msg.setString("Reanimating.");
                break;
            case 2:
                msg.setString("Reanimating..");
                break;
            case 3:
                msg.setString("Reanimating...");
                break;
            }

            msg.setPosition(20.0f, 20.0f);

            window.draw(msg);

            int percentage = 100.0f * (pretrainTimer / static_cast<float>(pretrainTime - 1));

            msg.setString(std::to_string(percentage) + "%");

            msg.setPosition(20.0f, 55.0f);

            window.draw(msg);

            msg.setString("Level " + std::to_string(levelIndex + 1));

            msg.setPosition(20.0f, 90.0f);

            window.draw(msg);
        }
        else {
            if (gameFinished) {
                msg.setPosition(20.0f, 20.0f);

                msg.setString("Victory?");

                window.draw(msg);

                msg.setPosition(20.0f, 55.0f);

                msg.setString("Space to play again");

                window.draw(msg);

                if (window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
                    gameFinished = false;

                    levelIndex = 0;
                    world = std::make_unique<World>();
                    world->init("resources/maps/map" + std::to_string(levelIndex + 1) + ".ldtk", window, levelIndex / 2 + 1, seedDist(rng));
                    pretrainTimer = 0;
                }

            }
            else {
                if (!world->levelFailed)
                    world->render(window);
            }
        }

        window.display();
    }

    return 0;
}
