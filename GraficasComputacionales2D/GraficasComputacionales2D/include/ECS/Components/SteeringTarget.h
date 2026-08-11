#pragma once

// Project Header
#include "Prerequisites.h"
#include "ECS/Types.h"

namespace ECS {

	// Tipos de comportamiento de direccion (steering behavior) disponibles para una entidad.
	enum class SteeringBehavior {
		Seek = 0, Flee = 1, Arrive = 2, Wander = 3,
		Pursuit = 4, ObstacleAvoidance = 5, PathFollowing = 6
	};

	// Componente que define el objetivo y comportamiento de direccion (steering).
	struct SteeringTarget {
		sf::Vector2f     targetPosition{ 0.f, 0.f };
		SteeringBehavior behavior{ SteeringBehavior::Seek };
		float            slowRadius{ 100.f };
		EntityId         followEntity{ 0 };
		bool             followAnEntity{ false };
		bool             enabled{ false };

		// Wander
		float wanderAngle{ 0.f };
		float wanderRadius{ 50.f };
		float wanderDistance{ 100.f };
		float wanderJitter{ 30.f };
		float wanderBoundaryRadius{ 150.f };   // Distancia de alejamiento de targetPosition
		float wanderExclusionRadius{ 40.f };   // Zona prohibida alrededor de targetPosition

		// Pursuit
		// Tiempo usado para predecir la posicion futura del objetivo.
		float predictionTime{ 0.5f };

		// Obstacle Avoidance
		// Radio de deteccion dentro del cual los obstaculos generan repulsion.
		float obstacleDetectionRadius{ 120.f };
		float obstacleAvoidForce{ 300.f };
	};

} // namespace ECS