#pragma once

// Third-party Libraries
#include <SFML/Graphics/VertexArray.hpp>

// Project Headers
#include "Core/Window.h"
#include "ECS/Components/Render.h"
#include "ECS/Components/Steering.h"
#include "ECS/Components/Transform.h"
#include "ECS/Registry.h"
#include "ECS/System.h"

// ============================================================
//  ECS :: Systems/RenderSystem.h
//
//  LÓGICA pura: recorre todas las entidades que tengan a la vez
//  Transform + Render, vuelca el Transform sobre la sf::Shape
//  y la dibuja en la ventana.
//
//  Debe llamarse DENTRO del frame, entre Window::clear()
//  y Window::display().
// ============================================================

namespace ECS {

    class RenderSystem final : public System {
    public:
        explicit RenderSystem(Window& window) noexcept
            : m_window(window) {
        }

        void OnUpdate(Registry& registry, float /*deltaTime*/) override {

            // 1. DIBUJAR TODAS LAS FIGURAS (Tu lógica original)
            registry.GetView<Transform, Render>().Each(
                [this](EntityId /*entity*/, Transform& t, Render& r) {
                    if (!r.shape || !r.visible) {
                        return;
                    }

                    // Vuelca el estado del Transform sobre la forma SFML.
                    r.shape->setPosition(t.position);
                    r.shape->setRotation(sf::degrees(t.rotation)); // SFML 3: usa sf::Angle
                    r.shape->setScale(t.scale);
                    r.shape->setFillColor(r.fillColor);

                    m_window.draw(*r.shape);
                });

            // 2. DIBUJAR LAS LÍNEAS DE DEBUG PARA STEERING
            registry.GetView<Steering>().Each([&](EntityId id, Steering& steering) {

                // Buscamos si la entidad tiene Transform para saber desde dónde dibujar
                if (auto* transform = registry.TryGetComponent<Transform>(id)) {

                    // LÍNEA VERDE: Dirección actual (Velocidad)
                    // (Nota: Si tu compilador no reconoce PrimitiveType::Lines, 
                    // cámbialo a sf::Lines)
                    sf::VertexArray currentVelLine(sf::PrimitiveType::Lines, 2);
                    currentVelLine[0].position = transform->position;
                    currentVelLine[0].color = sf::Color::Green;

                    // Multiplicamos por 0.5f para escalar visualmente la línea
                    currentVelLine[1].position = transform->position +
                        (steering.velocity * 0.5f);
                    currentVelLine[1].color = sf::Color::Green;

                    m_window.draw(currentVelLine);

                    // LÍNEA ROJA: Objetivo (Hacia dónde quiere ir)
                    // No dibujamos línea roja en Wander porque su objetivo es un 
                    // círculo imaginario móvil
                    if (steering.currentBehavior != SteeringType::Wander &&
                        steering.currentBehavior != SteeringType::None) {

                        sf::VertexArray targetLine(sf::PrimitiveType::Lines, 2);
                        targetLine[0].position = transform->position;
                        targetLine[0].color = sf::Color::Red;

                        // Si es Obstacle Avoidance, la línea roja apunta al obstáculo 
                        // en lugar del target
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
        }

    private:
        Window& m_window;
    };

} // namespace ECS