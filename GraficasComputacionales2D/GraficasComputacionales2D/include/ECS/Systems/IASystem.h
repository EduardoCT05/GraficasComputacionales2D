#pragma once
// Project Header
#include "ECS/Components/LapCounter.h"
#include "ECS/Components/Obstacle.h"
#include "ECS/Components/Path.h"
#include "ECS/Components/Physics.h"
#include "ECS/Components/SteeringTarget.h"
#include "ECS/Components/Transform.h"
#include "ECS/Registry.h"
#include "ECS/System.h"

namespace ECS {

    // Sistema de inteligencia artificial que aplica comportamientos de direccion 
    // a las entidades con Transform, Physics y SteeringTarget.
    // Soporta Seek, Flee, Arrive, Wander, Pursuit, ObstacleAvoidance y PathFollowing,
    // ademas de llevar el conteo de vueltas de las entidades con LapCounter.
    class IASystem final : public System {
    public:
        void OnUpdate(Registry& registry, float dt) override;

    private:
        // Fuerza de direccion para acercarse directamente al objetivo.
        static sf::Vector2f SeekForce(
            const sf::Vector2f& pos, const Physics& ph, const SteeringTarget& st);

        // Fuerza de direccion para alejarse del objetivo.
        static sf::Vector2f FleeForce(
            const sf::Vector2f& pos, const Physics& ph, const SteeringTarget& st);

        // Fuerza de direccion para llegar al objetivo desacelerando dentro de un radio.
        static sf::Vector2f ArriveForce(
            const sf::Vector2f& pos, const Physics& ph, const SteeringTarget& st);

        // Fuerza de direccion para un movimiento erratico (wander) alrededor del objetivo.
        static sf::Vector2f WanderForce(
            const sf::Vector2f& pos, const Physics& ph, SteeringTarget& st, float dt);

        // Fuerza de direccion para interceptar un objetivo en movimiento.
        static sf::Vector2f PursuitForce(
            const sf::Vector2f& pos, const Physics& ph, const SteeringTarget& st,
            const sf::Vector2f& targetVelocity);

        // Fuerza de repulsion acumulada respecto a obstaculos cercanos a la entidad self.
        static sf::Vector2f ObstacleAvoidanceForce(
            Registry& registry, EntityId self, const sf::Vector2f& pos, const SteeringTarget& st);

        // Fuerza de direccion para seguir el Path de la entidad referenciada en la SteeringTarget.
        static sf::Vector2f PathFollowingForce(
            Registry& registry, const Transform& t, const Physics& ph, const SteeringTarget& st);

        // Proyecta el punto p sobre el segmento de linea a-b y devuelve el punto proyectado.
        static sf::Vector2f getNormalPoint(
            const sf::Vector2f& p, const sf::Vector2f& a, const sf::Vector2f& b);

        // Devuelve un valor aleatorio uniforme entre -1 y 1.
        static float randomJitter();

        // Devuelve la magnitud (longitud) de un vector 2D.
        static float length(sf::Vector2f v);

        // Devuelve el vector normalizado (longitud 1), o vector cero si la entrada es nula.
        static sf::Vector2f normalize(sf::Vector2f v);

        // Limita la magnitud de un vector a un valor maximo, preservando su direccion.
        static sf::Vector2f limit(sf::Vector2f v, float max);

        // Actualiza el conteo de vueltas de las entidades con LapCounter segun su posicion.
        static void UpdateLapCounters(Registry& registry);
    };

} // namespace ECS