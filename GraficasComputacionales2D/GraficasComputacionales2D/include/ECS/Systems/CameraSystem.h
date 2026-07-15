#pragma once

#include <cmath>

// Project Headers
#include "Core/Window.h"
#include "ECS/Components/Camera.h"
#include "ECS/Components/Transform.h"
#include "ECS/Registry.h"
#include "ECS/System.h"

// ============================================================
//  ECS :: Systems/CameraSystem.h
//
//  LÓGICA de cámara. Busca la cámara activa (Transform + Camera),
//  interpola su posición hacia el objetivo a seguir y aplica la
//  vista resultante a la ventana.
//
//  ORDEN: debe registrarse ANTES del RenderSystem, porque
//  setView afecta a todo lo que se dibuje DESPUÉS.
// ============================================================

namespace ECS {

    class CameraSystem final : public System {
    public:
        explicit CameraSystem(Window& window) noexcept
            : m_window(window) {
        }

        void OnUpdate(Registry& registry, float deltaTime) override {
            registry.GetView<Transform, Camera>().Each(
                [this, &registry, deltaTime](EntityId, Transform& camT, Camera& cam) {
                    if (!cam.active) {
                        return;
                    }

                    // ── Seguimiento interpolado ───────────────────────
                    // Suavizado exponencial INDEPENDIENTE DEL FRAMERATE:
                    //   t = 1 - e^(-speed * dt)
                    // Garantiza el mismo movimiento a 30 o 144 FPS, a
                    // diferencia de un lerp con factor fijo.
                    if (cam.followTarget != NULL_ENTITY &&
                        registry.IsAlive(cam.followTarget)) {
                        if (auto* targetT =
                            registry.TryGetComponent<Transform>(cam.followTarget)) {
                            const float t = 1.f - std::exp(-cam.followSpeed * deltaTime);
                            camT.position += (targetT->position - camT.position) * t;
                        }
                    }

                    // Vuelca posición + zoom + rotación de la cámara a la vista SFML.
                    // (la rotación viene del Transform; el zoom del componente Camera)
                    m_window.applyCameraView(camT.position, cam.zoom, camT.rotation);
                });
        }

    private:
        Window& m_window;
    };

} // namespace ECS