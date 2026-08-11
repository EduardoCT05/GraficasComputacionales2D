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

namespace ECS {

    class RenderSystem final : public System {
    public:
        explicit RenderSystem(Window& window) noexcept
            : m_window(window) {
        }

        void OnUpdate(Registry& registry, float deltaTime) override;

    private:
        // Declaramos las funciones de dibujo auxiliares
        void DrawSprites(Registry& registry);
        void DrawPaths(Registry& registry);

        Window& m_window;
    };

} // namespace ECS