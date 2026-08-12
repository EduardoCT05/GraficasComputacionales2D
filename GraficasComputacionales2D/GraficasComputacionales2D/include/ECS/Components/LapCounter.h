#pragma once

// Project Headers
#include "ECS/Types.h"

namespace ECS {

	// Cuenta las vueltas completadas por una entidad que sigue un Path cerrado.
	struct LapCounter {
		int laps{ 0 };
		std::size_t lastIndex{ 0 };  // Indice del punto mas cercano en el frame anterior
		bool initialized{ false };   // Evita contar una vuelta falsa en el primer frame
		bool passedHalfway{ false }; // Checkpoint para sumar punto
		EntityId trackEntity{ 0 };
	};

} // namespace ECS