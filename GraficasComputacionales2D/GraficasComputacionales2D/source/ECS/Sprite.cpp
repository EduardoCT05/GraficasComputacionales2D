#include "ECS/Components/Sprite.h"

namespace ECS {
    //Crea un objeto Sprite a partir de una ruta de textura y una posición.
    //
    // Este método carga la textura desde el archivo especificado por `texturePath`,
    // crea un objeto `sf::Sprite` asociado a esa textura y establece su posición
    // en el vector `position`. Si la carga de la textura falla, se imprime un mensaje
    // de error y se termina la ejecución del programa.
    // 
    // texturePath Ruta del archivo de imagen a cargar como textura.
    // position Posición inicial del sprite en coordenadas 2D.
    // Un objeto `Sprite` con la textura y posición especificadas.
     
    Sprite Sprite::Make(const std::string& texturePath, const sf::Vector2f& position) {
        Sprite s;
        s.texture = std::make_shared<sf::Texture>();
        if (!s.texture->loadFromFile(texturePath)) {
            std::cerr << "Ruta que fallo: " << texturePath << "\n";
            ERROR("Sprite", "Make", "No se pudo cargar la imagen");
        }
        s.sprite = std::make_shared<sf::Sprite>(*s.texture);
        s.position = position;
        s.sprite->setPosition(position);
        return s;
    }

}