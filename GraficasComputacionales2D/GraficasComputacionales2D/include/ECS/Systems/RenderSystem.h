#pragma once
#include "ECS/System.h"
#include "ECS/Registry.h"
#include "ECS/Components/Transform.h"
#include "ECS/Components/Render.h"
#include "Core/Window.h"

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
            registry.GetView<Transform, Render>().Each(
                [this](EntityID /*entity*/, Transform& t, Render& r) {
                    if (!r.shape || !r.visible) return;

                    // Vuelca el estado del Transform sobre la forma SFML.
                    r.shape->setPosition(t.position);
                    r.shape->setRotation(sf::degrees(t.rotation)); // SFML 3: usa sf::Angle
                    r.shape->setScale(t.scale);
                    r.shape->setFillColor(r.fillColor);

                    m_window.draw(*r.shape);
                });
        }

    private:
        Window& m_window;
    };

} // namespace ECS