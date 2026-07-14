#pragma once
#include "Prerequisites.h"

// ============================================================
//  ECS :: Components/Render.h
//
//  Render — componente de DATOS PUROS que guarda la forma
//  dibujable (sf::Shape) de una entidad.
//
//  Se almacena en un ComponentPool<Render>. Usa shared_ptr
//  (movible y copiable) para ser compatible con el pool,
//  a diferencia de CShape, que tiene el move suprimido.
//
//  La POSICIÓN/ROTACIÓN/ESCALA NO viven aquí: vienen del
//  componente Transform y las aplica el RenderSystem cada frame.
// ============================================================

namespace ECS {

    struct Render {
        std::shared_ptr<sf::Shape> shape;          // la forma a dibujar
        std::shared_ptr<sf::Texture> texture;      // sprite OPCIONAL (nullptr = sin sprite)
        sf::Color fillColor{ sf::Color::White };   // relleno; sobre un sprite actúa como TINTE
        bool visible{ true };                      // permite ocultar sin quitar el componente

        Render() = default;

        explicit Render(std::shared_ptr<sf::Shape> s,
            sf::Color color = sf::Color::White) noexcept
            : shape(std::move(s)), fillColor(color) {
        }

        // ── Sprite opcional ───────────────────────────────────
        // Carga una textura desde disco y la aplica a la shape.
        // Devuelve false si la shape es nula o el archivo no carga.
        // La textura se conserva viva en 'texture' (la shape solo
        // guarda un puntero crudo a ella).
        bool SetTexture(const std::string& path, bool resetRect = true) {
            if (!shape) return false;
            auto tex = std::make_shared<sf::Texture>();
            if (!tex->loadFromFile(path)) return false;   // no toca el estado si falla
            texture = std::move(tex);
            shape->setTexture(texture.get(), resetRect);  // resetRect ajusta el rect al tamaño
            return true;
        }

        // Aplica una textura ya cargada (compartida entre entidades → eficiente).
        void SetTexture(std::shared_ptr<sf::Texture> tex, bool resetRect = true) {
            if (!shape) return;
            texture = std::move(tex);
            shape->setTexture(texture ? texture.get() : nullptr, resetRect);
        }

        // Quita el sprite y vuelve a la shape de color sólido.
        void ClearTexture() {
            if (shape) shape->setTexture(nullptr);
            texture.reset();
        }

        // ── Factory de conveniencia (reusa tu enum ShapeType) ──
        // Crea la forma centrada en su origen, para que el Transform
        // la posicione por su CENTRO (no por la esquina superior izq.).
        [[nodiscard]] static Render
            Make(ShapeType type,
                sf::Color color = sf::Color::White,
                const std::string& texturePath = "") {   // sprite OPCIONAL
            std::shared_ptr<sf::Shape> s;
            switch (type) {
            case CIRCLE: {
                auto c = std::make_shared<sf::CircleShape>(50.f);
                c->setOrigin({ 50.f, 50.f });          // centro = radio
                s = c;
                break;
            }
            case RECTANGLE: {
                sf::Vector2f size{ 100.f, 50.f };
                auto r = std::make_shared<sf::RectangleShape>(size);
                r->setOrigin(size / 2.f);
                s = r;
                break;
            }
            case TRIANGLE: {
                auto t = std::make_shared<sf::ConvexShape>(3);
                t->setPoint(0, { 0.f,   0.f });
                t->setPoint(1, { 100.f, 0.f });
                t->setPoint(2, { 50.f,  100.f });
                t->setOrigin({ 50.f, 50.f });
                s = t;
                break;
            }
            default:
                break;
            }
            if (s) s->setFillColor(color);

            Render render{ s, color };
            // Si se pasó una ruta, intenta cargar el sprite (silencioso si falla).
            if (!texturePath.empty())
                render.SetTexture(texturePath);
            return render;
        }
    };

} // namespace ECS