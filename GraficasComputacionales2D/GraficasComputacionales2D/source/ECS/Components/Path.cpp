// Self-include
#include "ECS/Components/Path.h"

namespace ECS {

    // Genera los puntos suavizados de la ruta a partir de los puntos de control.
    // Recorre cada punto de control como segmento de una curva Catmull-Rom,
    // usando los puntos vecinos (anterior y siguientes dos, con wrap-around circular)
    // para interpolar una cantidad fija de puntos intermedios por curva (segmentsPerCurve).
    // El resultado se almacena en el contenedor points.
    // Si hay menos de 3 puntos de control, la funcion no genera nada y retorna inmediatamente.
    void Path::GenerateSmoothPoints() {
        points.clear();
        const int n = static_cast<int>(controlPoints.size());
        if (n < 3) return;

        for (int i = 0; i < n; ++i) {
            // Punto de control anterior (con wrap-around circular).
            const sf::Vector2f& p0 = controlPoints[(i - 1 + n) % n];
            // Punto de control actual, inicio del segmento de curva.
            const sf::Vector2f& p1 = controlPoints[i];
            // Punto de control siguiente, fin del segmento de curva.
            const sf::Vector2f& p2 = controlPoints[(i + 1) % n];
            // Segundo punto de control siguiente (con wrap-around circular).
            const sf::Vector2f& p3 = controlPoints[(i + 2) % n];

            for (int j = 0; j < segmentsPerCurve; ++j) {
                // Parametro de interpolacion t normalizado entre 0 y 1 para el segmento actual.
                float t = static_cast<float>(j) / static_cast<float>(segmentsPerCurve);
                points.push_back(CatmullRom(p0, p1, p2, p3, t));
            }
        }
    }

    // Calcula un punto interpolado usando la formula de Catmull-Rom spline.
    // p0: Punto de control anterior al segmento
    // p1: Punto de control de inicio del segmento
    // p2: Punto de control de fin del segmento (implicito en la curva)
    // p3: Segundo punto de control siguiente (implicito en la curva)
    // t: Parametro de interpolacion entre 0 y 1
    // Retorna un Vector2f con la posicion interpolada sobre la curva en el parametro t.
    sf::Vector2f Path::CatmullRom(
        const sf::Vector2f& p0, const sf::Vector2f& p1,
        const sf::Vector2f& p2, const sf::Vector2f& p3, float t
    ) {
        float t2 = t * t;
        float t3 = t2 * t;

        float x = 0.5f * ((2.f * p1.x) +
            (-p0.x + p2.x) * t +
            (2.f * p0.x - 5.f * p1.x + 4.f * p2.x - p3.x) * t2 +
            (-p0.x + 3.f * p1.x - 3.f * p2.x + p3.x) * t3);

        float y = 0.5f * ((2.f * p1.y) +
            (-p0.y + p2.y) * t +
            (2.f * p0.y - 5.f * p1.y + 4.f * p2.y - p3.y) * t2 +
            (-p0.y + 3.f * p1.y - 3.f * p2.y + p3.y) * t3);

        return { x, y };
    }

} // namespace ECS