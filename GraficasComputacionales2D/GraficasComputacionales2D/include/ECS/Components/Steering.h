#pragma once

// Third-party Libraries
#include <SFML/System/Vector2.hpp>

namespace ECS {

	enum class SteeringType {
		None, Seek, Flee, Arrive, Wander, Pursuit, ObstacleAvoidance
	};

	struct Steering {
		SteeringType currentBehavior = SteeringType::Seek;

		sf::Vector2f velocity{ 0.f, 0.f };
		sf::Vector2f target{ 0.f, 0.f };

		// --- Nuevas variables ---

		// Para Pursuit (predecir a dónde va el objetivo)
		sf::Vector2f targetVelocity{ 0.f, 0.f };

		// Para Obstacle Avoidance
		sf::Vector2f obstaclePos{ 0.f, 0.f };

		float maxSpeed = 150.f;
		float maxForce = 50.f;
		float mass = 1.0f;
		float slowingRadius = 150.f;

		// --- Variables físicas exclusivas de Wander ---

		// Radio del círculo imaginario frente a la entidad
		float wanderRadius = 50.f;

		// Qué tan lejos está ese círculo
		float wanderDistance = 100.f;

		// Qué tan errático es el giro aleatorio
		float wanderJitter = 10.f;

		// Ángulo interno actual
		float wanderAngle = 0.f;
	};

} // namespace ECS