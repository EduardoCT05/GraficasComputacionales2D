#pragma once
#include "Prerequisites.h"

//=============================================================================
// ECS:: Components/Transform.h
// Transform - componente de Datos PUROS (sin lógica)
// Describe la poscición, rotación y escala de una entidad
// en el espacio 2D del motor
//
// Como todo compoennente de este ECS, es un simple strcut
//	  - se almacena en un ComponentPool<Transform>
//	  - No hereda de nada ni tiene métodos virtuales
//	  - La lógica que lo usa vive en los System
//		futuro RenderSystem o MovementSystem
//=============================================================================

namespace ECS {
	struct Transform {
		// Posisición en pixeles (coordenadas de mundo) 
		sf::Vector2f position{ 0.f, 0.f };

		// Rotación en grados (SMFL usa grados, sentido horario 
		float rotation{ 0.f };

		// escala por eje (1,1 = tamaño original). 
		sf::Vector2f scale{ 1.f, 1.f };

		// -----Constructtores de conveniencia------
		Transform() = default;

		explicit Transform(sf::Vector2f pos, float rot = 0.f,
			sf::Vector2f scl = { 1.f, 1.f }) noexcept
			: position(pos), rotation(rot), scale(scl) {

		}

		//helpers (operan solo sobre los datos del componente
		//Desplaza la posisción de forma relativa 
		void
			Translate(sf::Vector2f delta) noexcept {
			position += delta;
		}

		void Rotate(float degrees) noexcept { rotation += degrees; }
	};
}