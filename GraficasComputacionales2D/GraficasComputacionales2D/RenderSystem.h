#pragma once
#include "Core/Window.h"
#include "ECS/System.h"
#include "ECS/Registry.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Render.h"


// ECS:: system/rendersystem.h
// Logica pura: recorre todas las entidades que tengan a la vez
// los componentes Transform y Render, y sincroniza la forma gráfica
// (sf::Shape) de cada una con su posición, rotación y escala antes de
// dibujarla en la ventana. Las entidades sin shape asignado, no visibles,
// o cuyo Render se omite, no se dibujan

namespace ECS {
    class RenderSystem final : public System {
    public:
        explicit RenderSystem(Window& window) noexcept
            : m_window(window) {

        }

        // Actualiza y dibuja todas las entidades con Transform y Render
        void OnUpdate(Registry& registry, float /*deltatime*/) override {
            registry.GetView<Transform, Render>().Each(
                [this](EntityId /*entity*/, Transform& t, Render& r) {
                    if (!r.shape || !r.visible) return;
                    r.shape->setPosition(t.position);
                    r.shape->setRotation(sf::degrees(t.rotation));
                    r.shape->setScale(t.scale);
                    if (!r.texture)
                        r.shape->setFillColor(r.fillColor);
                    m_window.draw(*r.shape);
                });
        }
    private:
        /// @brief Ventana donde se dibujan las entidades.
        Window& m_window;
    };
}