#pragma once

// C Library
#include <cmath>
#include <cstdlib>

// Project Headers
#include "ECS/Components/Steering.h"
#include "ECS/Components/Transform.h"
#include "ECS/Registry.h"
#include "ECS/System.h"

namespace ECS {

    class SteeringSystem final : public System {
    public:
        SteeringSystem() = default;

        void OnUpdate(Registry& registry, float dt) override {
            registry.GetView<ECS::Steering>().Each(
                [&](ECS::EntityId id, ECS::Steering& steering) {
                    if (auto* transform = registry.TryGetComponent<ECS::Transform>(id)) {

                        sf::Vector2f desiredVelocity(0.f, 0.f);

                        sf::Vector2f direction = steering.target - transform->position;
                        float distance = std::sqrt(direction.x * direction.x +
                            direction.y * direction.y);

                        
                        // 1. LÓGICA WANDER (Vagar aleatoriamente) ===
                        
                        if (steering.currentBehavior == SteeringType::Wander) {
                            // Generar un cambio aleatorio entre -1 y 1
                            float randomClamped = ((std::rand() % 100) / 100.f) * 2.f - 1.f;
                            steering.wanderAngle += randomClamped * steering.wanderJitter;

                            // Calcular hacia dónde mira actualmente
                            sf::Vector2f heading = steering.velocity;
                            float length = std::sqrt(heading.x * heading.x +
                                heading.y * heading.y);
                            if (length > 0.0001f) {
                                heading /= length;
                            }
                            else {
                                heading = sf::Vector2f(1.f, 0.f);
                            }

                            // Proyectar un círculo al frente
                            sf::Vector2f circleCenter = transform->position +
                                (heading * steering.wanderDistance);
                            sf::Vector2f displacement(
                                std::cos(steering.wanderAngle) * steering.wanderRadius,
                                std::sin(steering.wanderAngle) * steering.wanderRadius
                            );

                            sf::Vector2f wanderForce = (circleCenter + displacement) -
                                transform->position;

                            float forceLen = std::sqrt(wanderForce.x * wanderForce.x +
                                wanderForce.y * wanderForce.y);
                            if (forceLen > 0.0001f) {
                                desiredVelocity = (wanderForce / forceLen) * steering.maxSpeed;
                            }
                        }

                        // 2. LÓGICA OBSTACLE AVOIDANCE (Esquivar obstáculos)
                       
                        else if (steering.currentBehavior == SteeringType::ObstacleAvoidance) {
                            sf::Vector2f vectorToObstacle = steering.obstaclePos -
                                transform->position;
                            float distToObs = std::sqrt(vectorToObstacle.x * vectorToObstacle.x +
                                vectorToObstacle.y * vectorToObstacle.y);

                            float avoidDistance = 150.f; // Distancia de peligro
                            if (distToObs > 0 && distToObs < avoidDistance) {
                                // Si está muy cerca del obstáculo, aplica una fuerza inversa
                                desiredVelocity = -(vectorToObstacle / distToObs) *
                                    steering.maxSpeed;
                            }
                            else {
                                // Si no hay peligro cercano, hace un Seek normal hacia su objetivo
                                if (distance > 0.0001f) {
                                    desiredVelocity = (direction / distance) * steering.maxSpeed;
                                }
                            }
                        }
                        // Resto de comportamientos que dependen directamente de la distancia
                        else if (distance > 0.0001f) {

                            // 3. LÓGICA SEEK (Buscar / Perseguir directamente) 
                         
                            if (steering.currentBehavior == SteeringType::Seek) {
                                desiredVelocity = (direction / distance) * steering.maxSpeed;
                            }

                            //  4. LÓGICA FLEE (Huir / Alejarse del objetivo) 
                            
                            else if (steering.currentBehavior == SteeringType::Flee) {
                                desiredVelocity = -(direction / distance) * steering.maxSpeed;
                            }

							// 5. LÓGICA ARRIVE (Buscar con desaceleración)

                            else if (steering.currentBehavior == SteeringType::Arrive) {
                                float speed = steering.maxSpeed;
                                if (distance < steering.slowingRadius) {
                                    speed = steering.maxSpeed * (distance / steering.slowingRadius);
                                }
                                desiredVelocity = (direction / distance) * speed;
                            }

							// 6. LÓGICA PURSUIT(Persecución con predicción)

                            else if (steering.currentBehavior == SteeringType::Pursuit) {
                                // Predecir la posición futura basada en la velocidad del objetivo
                                float lookAheadTime = distance / steering.maxSpeed;
                                sf::Vector2f predictedTarget = steering.target +
                                    (steering.targetVelocity * lookAheadTime);

                                sf::Vector2f pursuitDir = predictedTarget - transform->position;
                                float pDist = std::sqrt(pursuitDir.x * pursuitDir.x +
                                    pursuitDir.y * pursuitDir.y);
                                if (pDist > 0.0001f) {
                                    desiredVelocity = (pursuitDir / pDist) * steering.maxSpeed;
                                }
                            }
                        }

						// Aplicar la fuerza de steering y actualizar la posición y rotación

                        if (steering.currentBehavior != SteeringType::None) {
                            sf::Vector2f steeringForce = desiredVelocity - steering.velocity;

                            float forceLength = std::sqrt(steeringForce.x * steeringForce.x +
                                steeringForce.y * steeringForce.y);
                            if (forceLength > steering.maxForce) {
                                steeringForce = (steeringForce / forceLength) * steering.maxForce;
                            }

                            sf::Vector2f acceleration = steeringForce / steering.mass;
                            steering.velocity += acceleration * dt;

                            float speed = std::sqrt(steering.velocity.x * steering.velocity.x +
                                steering.velocity.y * steering.velocity.y);
                            if (speed > steering.maxSpeed) {
                                steering.velocity = (steering.velocity / speed) * steering.maxSpeed;
                            }

                            transform->position += steering.velocity * dt;

                            // Hacer que la figura gire para mirar hacia donde se mueve
                            if (speed > 0.0001f) {
                                transform->rotation = std::atan2(steering.velocity.y,
                                    steering.velocity.x) *
                                    (180.f / 3.14159265f);
                            }

                            
                            // 7. SCREEN WRAPPING (Efecto Pac-Man en los bordes) 
                       
                            if (transform->position.x > 800.f) {
                                transform->position.x = 0.f;
                            }
                            if (transform->position.x < 0.f) {
                                transform->position.x = 800.f;
                            }
                            if (transform->position.y > 600.f) {
                                transform->position.y = 0.f;
                            }
                            if (transform->position.y < 0.f) {
                                transform->position.y = 600.f;
                            }
                        }
                    }
                });
        }
    };

} // namespace ECS