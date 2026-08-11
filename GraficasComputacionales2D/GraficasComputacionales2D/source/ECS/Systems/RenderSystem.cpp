#include "ECS/Systems/RenderSystem.h"
#include "ECS/Components/Path.h"
#include "ECS/Components/Sprite.h"
#include <cmath>

namespace ECS {

    void RenderSystem::OnUpdate(Registry& registry, float /*deltaTime*/) {
        // 1. DIBUJAR SPRITES DE FONDO
        DrawSprites(registry);

        // 2. DIBUJAR FIGURAS PRINCIPALES (Transform + Render)
        registry.GetView<Transform, Render>().Each(
            [this](EntityId /*entity*/, Transform& t, Render& r) {
                if (!r.shape || !r.visible) return;

                r.shape->setPosition(t.position);
                r.shape->setRotation(sf::degrees(t.rotation));
                r.shape->setScale(t.scale);

                if (!r.texture) {
                    r.shape->setFillColor(r.fillColor);
                }

                m_window.draw(*r.shape);
            });

        // 3. DIBUJAR LAS LÍNEAS DE DEBUG PARA STEERING
        registry.GetView<Steering>().Each([&](EntityId id, Steering& steering) {
            if (auto* transform = registry.TryGetComponent<Transform>(id)) {
                // LÍNEA VERDE: Dirección actual (Velocidad)
                sf::VertexArray currentVelLine(sf::PrimitiveType::Lines, 2);
                currentVelLine[0].position = transform->position;
                currentVelLine[0].color = sf::Color::Green;
                currentVelLine[1].position = transform->position + (steering.velocity * 0.5f);
                currentVelLine[1].color = sf::Color::Green;

                m_window.draw(currentVelLine);

                // LÍNEA ROJA: Objetivo (Hacia dónde quiere ir)
                if (steering.currentBehavior != SteeringType::Wander &&
                    steering.currentBehavior != SteeringType::None) {

                    sf::VertexArray targetLine(sf::PrimitiveType::Lines, 2);
                    targetLine[0].position = transform->position;
                    targetLine[0].color = sf::Color::Red;

                    if (steering.currentBehavior == SteeringType::ObstacleAvoidance) {
                        targetLine[1].position = steering.obstaclePos;
                    }
                    else {
                        targetLine[1].position = steering.target;
                    }
                    targetLine[1].color = sf::Color::Red;

                    m_window.draw(targetLine);
                }
            }
            });

        // 4. DIBUJAR RUTAS DE LOS BOTS
        // DrawPaths(registry); // Descomenta esta línea si quieres ver la ruta dibujada en rojo
    }

    void RenderSystem::DrawSprites(Registry& registry) {
        registry.GetView<Sprite>().Each(
            [this](EntityId /*entity*/, Sprite& s) {
                if (!s.sprite) return;
                m_window.draw(*s.sprite);
            });
    }

    void RenderSystem::DrawPaths(Registry& registry) {
        registry.GetView<Path>().Each(
            [this](EntityId /*entity*/, Path& path) {
                if (path.points.size() < 2) return;

                const std::size_t n = path.points.size();

                sf::VertexArray road(sf::PrimitiveType::TriangleStrip);
                for (std::size_t i = 0; i <= n; ++i) {
                    const sf::Vector2f& p0 = path.points[i % n];
                    const sf::Vector2f& p1 = path.points[(i + 1) % n];
                    sf::Vector2f dir = p1 - p0;
                    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                    sf::Vector2f normal = (len > 0.f)
                        ? sf::Vector2f(-dir.y / len, dir.x / len)
                        : sf::Vector2f(0.f, 0.f);

                    sf::Vertex vTop, vBottom;
                    vTop.position = p0 + normal * path.radius;
                    vTop.color = sf::Color(100, 100, 100, 120);
                    vBottom.position = p0 - normal * path.radius;
                    vBottom.color = sf::Color(100, 100, 100, 120);

                    road.append(vTop);
                    road.append(vBottom);
                }
                m_window.draw(road);

                sf::VertexArray centerLine(sf::PrimitiveType::LineStrip);
                for (std::size_t i = 0; i <= n; ++i) {
                    sf::Vertex v;
                    v.position = path.points[i % n];
                    v.color = sf::Color::Red;
                    centerLine.append(v);
                }
                m_window.draw(centerLine);
            });
    }

} // namespace ECS