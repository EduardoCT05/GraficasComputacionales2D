#pragma once
#include "Prerequisites.h"

namespace ECS {
    // Imagen de fondo sin nada más solo renderizado de la escena.
    struct Sprite {
        std::shared_ptr<sf::Texture> texture;
        std::shared_ptr<sf::Sprite> sprite;
        sf::Vector2f position{ 0.f, 0.f };

        static Sprite Make(const std::string& texturePath, const sf::Vector2f& position);
    };
}