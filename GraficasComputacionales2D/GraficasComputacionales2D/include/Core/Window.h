#pragma once

// Project Header
#include "Prerequisites.h"

// Window
// Clase envoltoria para la ventana de renderizado de SFML
class Window {
public:
    Window() = default;
    Window(int width, int height, const std::string& tittle);
    ~Window() = default;

    // Consulta si la ventana sigue abierta, retorna t o f
    bool isOpen() const;

    // limpia la pantalla antes de dibujar el siguiente frame
    void clear(const sf::Color& color = sf::Color(0, 0, 0, 255));

    // Dibuja un objeto renderizable
    // drawable es el objeto a dibujar
    // states Estados de renderizado en el proceso
    void draw(const sf::Drawable& drawable,
        const sf::RenderStates& states = sf::RenderStates::Default);

    // Muestra en la pantalla lo que se rendeirza
    // intercambia buferes
    void display();

    void close();

    // actualiza con frames
    void update();

    void render();

    void destroy();

    void handleResize(const sf::Vector2u& size);

    void applyCameraView(const sf::Vector2f& position,
        float zoom,
        float rotationDeg = 0.f);

    // puntero inteligente de la ventana real
    std::unique_ptr<sf::RenderWindow> m_window = nullptr;

private:
    sf::View m_view;
    sf::Time m_deltaTime;
    sf::Clock m_clock;
    sf::Vector2f m_baseViewSize;
};