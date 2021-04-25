#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <aogmaneo/Hierarchy.h>

#include "world/World.h"

int main() {
    aon::setNumThreads(8);

    int frameRate = 60;

    sf::RenderWindow window(sf::VideoMode(1200, 900), "SFML window");

    window.setVerticalSyncEnabled(true);
    window.setFramerateLimit(frameRate);

    window.setMouseCursorVisible(false);

    float dt = 1.0f / frameRate;

    World world;
    world.init("resources/maps/map1.ldtk", window);

    sf::View view = window.getDefaultView();
    sf::View newView = view;

    float zoomRate = 0.25f;
    float viewInterpolateRate = 20.0f;

    sf::Vector2i prevMousePos = sf::Mouse::getPosition(window);

    sf::Clock globalClock;

    while (window.isOpen()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        if (window.hasFocus()) {
            sf::Event event;

            while (window.pollEvent(event)) {
                switch (event.type) {
                case sf::Event::Closed:
                    window.close();

                    break;

                case sf::Event::MouseWheelMoved:
                    int dWheel = event.mouseWheel.delta;

                    newView = view;

                    window.setView(newView);

                    sf::Vector2f mouseZoomDelta0 = window.mapPixelToCoords(mousePos);

                    newView.setSize(view.getSize() + newView.getSize() * (-zoomRate * dWheel));

                    window.setView(newView);

                    sf::Vector2f mouseZoomDelta1 = window.mapPixelToCoords(mousePos);

                    window.setView(view);

                    newView.setCenter(view.getCenter() + mouseZoomDelta0 - mouseZoomDelta1);

                    break;
                }
            }

            sf::Time duration = globalClock.restart();

            view.setCenter(view.getCenter() + (newView.getCenter() - view.getCenter()) * viewInterpolateRate * dt);
            view.setSize(view.getSize() + (newView.getSize() - view.getSize()) * viewInterpolateRate * dt);

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                sf::Vector2i dPos = mousePos - prevMousePos;

                view.setCenter(view.getCenter() - sf::Vector2f(dPos.x * (static_cast<float>(view.getSize().x) / window.getSize().x), dPos.y * (static_cast<float>(view.getSize().x) / window.getSize().x)));

                newView.setCenter(view.getCenter());
            }
        }

        prevMousePos = mousePos;

        world.update(window, dt);

        window.setView(view);

        window.clear(sf::Color::Black);

        world.render(window);

        window.display();
    }

    return 0;
}
