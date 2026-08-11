#pragma once

// Project Header
#include "Prerequisites.h"

namespace ECS {

	// Componente que almacena las propiedades fisicas de una entidad.
	// Usado por sistemas de fisica y de steering behaviors 
	// (Seek, Flee, Arrive, Wander, Pursuit, Obstacle Avoidance) 
	// para calcular e integrar el movimiento de la entidad en el espacio.
	struct Physics {
		// Velocidad actual de la entidad
		sf::Vector2f velocity{ 0.f, 0.f };
		// Aceleracion actual de la entidad
		sf::Vector2f acceleration{ 0.f, 0.f };
		// Velocidad maxima permitida
		float maxSpeed{ 150.f };
		// Fuerza maxima que se puede aplicar
		float maxForce{ 10.f };
		// Masa de la entidad
		float mass{ 1.f };
	};

} // namespace ECS