#pragma once

// c++ Library
#include <vector>

// Project Headers
#include "Prerequisites.h"

namespace ECS {

    // Circuito cerrado definido por puntos de control, suavizado con Catmull-Rom.
    struct Path {
        std::vector<sf::Vector2f> controlPoints; // Puntos fijos
        std::vector<sf::Vector2f> points;        // Puntos suavizados interpolados
        float radius{ 15.f };
        int segmentsPerCurve{ 20 };

        // Solo declaraciones (El cuerpo ya esta definido en Path.cpp)
        void GenerateSmoothPoints();

    private:
        static sf::Vector2f CatmullRom(
            const sf::Vector2f& p0, const sf::Vector2f& p1,
            const sf::Vector2f& p2, const sf::Vector2f& p3, float t);
    };

} // namespace ECS