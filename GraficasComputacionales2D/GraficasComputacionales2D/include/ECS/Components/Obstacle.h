#pragma once

namespace ECS {

	// Componente etiqueta (tag) vacio que marca una entidad como obstaculo para la IA.
	// IASystem::computeAvoidance la considera al calcular fuerzas de evasion.
	struct Obstacle {};

} // namespace ECS