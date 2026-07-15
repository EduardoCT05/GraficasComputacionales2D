// Self-Include
#include "Core/window.h"

Window::Window(int width, int height, const std::string& title) {
    m_window = std::make_unique<sf::RenderWindow>(
        sf::VideoMode({ static_cast<unsigned int>(width),
                        static_cast<unsigned int>(height) }),
        title,
        sf::Style::Default
    );

    if (m_window) {
        m_window->setFramerateLimit(60);
        MESSAGE("window", "windows", "Window created successfully");

        // Inicializar el tamaño base de la vista con el tamaño original de la ventana
        m_baseViewSize = sf::Vector2f(static_cast<float>(width),
            static_cast<float>(height));
        m_view.setSize(m_baseViewSize);
    }
    else {
        ERROR("Window", "windows", "Failed to create window");
    }
}

bool Window::isOpen() const {
    //check that window is not null
    if (m_window) {
        return m_window && m_window->isOpen();
    }
    else {
        ERROR("Window", "isOpen", "Window is null");
        return false;
    }
}

void Window::clear(const sf::Color& color) {
    if (m_window) {
        m_window->clear(color);
    }
    else {
        ERROR("Window", "clear", "Window is null");
    }
}

void Window::draw(const sf::Drawable& drawable,
    const sf::RenderStates& states) {
    if (m_window) {
        m_window->draw(drawable, states);
    }
    else {
        ERROR("Window", "draw", "Window is null");
    }
}

void Window::display() {
    if (m_window) {
        m_window->display();
    }
    else {
        ERROR("Window", "display", "Window is null");
    }
}

void Window::close() {
    if (m_window) {
        m_window->close();
    }
    else {
        ERROR("Window", "close", "Window is null");
    }
}

void Window::update() {
    //Almacena el deltaTime una sola vez
    m_deltaTime = m_clock.restart();
}

void Window::render() {
}

void Window::destroy() {
    m_window.reset();
}

void Window::handleResize(const sf::Vector2u& size) {
    // Ajusta el tamaño de la vista base cuando la ventana cambia de tamaño
    m_baseViewSize = sf::Vector2f(static_cast<float>(size.x),
        static_cast<float>(size.y));
    m_view.setSize(m_baseViewSize);
}

void Window::applyCameraView(const sf::Vector2f& position,
    float zoom,
    float rotationDeg) {
    // 1. Centramos la cámara en la posición de la entidad
    m_view.setCenter(position);

    // 2. Aplicamos el zoom usando el tamaño base
    m_view.setSize(sf::Vector2f(m_baseViewSize.x * zoom,
        m_baseViewSize.y * zoom));

    // 3. Aplicamos la rotación
    m_view.setRotation(sf::degrees(rotationDeg));

    // 4. Actualizamos la vista de la ventana
    if (m_window) {
        m_window->setView(m_view);
    }
}