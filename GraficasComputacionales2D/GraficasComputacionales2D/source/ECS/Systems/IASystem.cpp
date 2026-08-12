// C Library
#include <cmath>

// C++ Library
#include <algorithm>
#include <limits>

// Project Headers
#include "Ecs/Systems/IASystem.h"

namespace ECS {

    // Actualiza el comportamiento de direccion (steering) de todas las entidades.
    void IASystem::OnUpdate(Registry& registry, float dt) {
        registry.GetView<Transform, Physics, SteeringTarget>().Each(
            [&registry, dt](EntityId id, Transform& t, Physics& ph, SteeringTarget& st) {
                if (!st.enabled) return;

                sf::Vector2f targetVelocity{ 0.f, 0.f };
                if (st.followAnEntity && registry.IsAlive(st.followEntity)) {
                    if (auto* targetT = registry.TryGetComponent<Transform>(st.followEntity)) {
                        st.targetPosition = targetT->position;
                    }
                    if (auto* targetPh = registry.TryGetComponent<Physics>(st.followEntity)) {
                        targetVelocity = targetPh->velocity;
                    }
                }

                sf::Vector2f desired{ 0.f, 0.f };
                switch (st.behavior) {
                case SteeringBehavior::Seek:
                    desired = SeekForce(t.position, ph, st);
                    break;
                case SteeringBehavior::Flee:
                    desired = FleeForce(t.position, ph, st);
                    break;
                case SteeringBehavior::Arrive:
                    desired = ArriveForce(t.position, ph, st);
                    break;
                case SteeringBehavior::Wander:
                    desired = WanderForce(t.position, ph, st, dt);
                    break;
                case SteeringBehavior::Pursuit:
                    desired = PursuitForce(t.position, ph, st, targetVelocity);
                    break;
                case SteeringBehavior::ObstacleAvoidance:
                    desired = SeekForce(t.position, ph, st);
                    break;
                case SteeringBehavior::PathFollowing:
                    desired = PathFollowingForce(registry, t, ph, st);
                    break;
                }

                sf::Vector2f steer = desired - ph.velocity;
                steer = limit(steer, ph.maxForce);
                ph.acceleration += steer / ph.mass;

                // Integracion de Flocking (Evadir Karts)
                sf::Vector2f avoid = ObstacleAvoidanceForce(registry, id, t.position, st);
                ph.acceleration += avoid / ph.mass;

                ph.velocity += ph.acceleration * dt;
                ph.velocity = limit(ph.velocity, ph.maxSpeed);
                t.position += ph.velocity * dt;
                ph.acceleration = { 0.f, 0.f };

                if (length(ph.velocity) > 0.1f) {
                    float targetAngleRad = std::atan2(ph.velocity.y, ph.velocity.x);
                    float targetAngleDeg = targetAngleRad * 180.f / 3.14159265f;

                    float diff = targetAngleDeg - t.rotation;
                    while (diff > 180.f) diff -= 360.f;
                    while (diff < -180.f) diff += 360.f;
                    t.rotation += diff * std::min(1.f, dt * 10.f);
                }
            });
        UpdateLapCounters(registry);
    }

    sf::Vector2f IASystem::SeekForce(
        const sf::Vector2f& pos, const Physics& ph, const SteeringTarget& st
    ) {
        sf::Vector2f diff = st.targetPosition - pos;
        if (length(diff) == 0.f) return { 0.f, 0.f };
        return normalize(diff) * ph.maxSpeed;
    }

    sf::Vector2f IASystem::FleeForce(
        const sf::Vector2f& pos, const Physics& ph, const SteeringTarget& st
    ) {
        sf::Vector2f diff = st.targetPosition - pos;
        float distance = length(diff);
        if (distance == 0.f) return { 0.f, 0.f };
        return normalize(-diff) * ph.maxSpeed;
    }

    sf::Vector2f IASystem::ArriveForce(
        const sf::Vector2f& pos, const Physics& ph, const SteeringTarget& st
    ) {
        sf::Vector2f diff = st.targetPosition - pos;
        float distance = length(diff);
        if (distance == 0.f) return { 0.f, 0.f };
        float speed = (distance < st.slowRadius)
            ? ph.maxSpeed * (distance / st.slowRadius)
            : ph.maxSpeed;
        return normalize(diff) * speed;
    }

    sf::Vector2f IASystem::WanderForce(
        const sf::Vector2f& pos, const Physics& ph, SteeringTarget& st, float dt
    ) {
        st.wanderAngle += randomJitter() * st.wanderJitter * dt;

        sf::Vector2f heading = length(ph.velocity) > 0.f
            ? normalize(ph.velocity)
            : sf::Vector2f{ 1.f, 0.f };

        sf::Vector2f circleCenter = pos + heading * st.wanderDistance;
        sf::Vector2f displacement = { std::cos(st.wanderAngle), std::sin(st.wanderAngle) };
        displacement *= st.wanderRadius;
        sf::Vector2f wanderPoint = circleCenter + displacement;

        sf::Vector2f toPoint = wanderPoint - st.targetPosition;
        if (length(toPoint) < st.wanderExclusionRadius) {
            sf::Vector2f pushOut = length(toPoint) > 0.f
                ? normalize(toPoint)
                : sf::Vector2f{ 1.f, 0.f };
            wanderPoint = st.targetPosition + pushOut * st.wanderExclusionRadius;
        }

        sf::Vector2f fromCenter = pos - st.targetPosition;
        if (length(fromCenter) > st.wanderBoundaryRadius) {
            sf::Vector2f back = st.targetPosition - pos;
            if (length(back) == 0.f) return { 0.f, 0.f };
            return normalize(back) * ph.maxSpeed;
        }

        sf::Vector2f desired = wanderPoint - pos;
        if (length(desired) == 0.f) return { 0.f, 0.f };
        return normalize(desired) * ph.maxSpeed;
    }

    sf::Vector2f IASystem::PursuitForce(
        const sf::Vector2f& pos, const Physics& ph,
        const SteeringTarget& st, const sf::Vector2f& targetVelocity
    ) {
        sf::Vector2f predicted = st.targetPosition + targetVelocity * st.predictionTime;
        sf::Vector2f diff = predicted - pos;
        if (length(diff) == 0.f) return { 0.f, 0.f };
        return normalize(diff) * ph.maxSpeed;
    }

    sf::Vector2f IASystem::ObstacleAvoidanceForce(
        Registry& registry, EntityId self, const sf::Vector2f& pos, const SteeringTarget& st
    ) {
        sf::Vector2f avoidForce{ 0.f, 0.f };
        registry.GetView<Transform, Obstacle>().Each(
            [&](EntityId obsId, Transform& obsT, Obstacle&) {
                if (obsId == self) return;
                sf::Vector2f diff = pos - obsT.position;
                float distance = length(diff);
                if (distance > 0.f && distance < st.obstacleDetectionRadius) {
                    float strength = st.obstacleAvoidForce * (1.f - distance / st.obstacleDetectionRadius);
                    avoidForce += normalize(diff) * strength;
                }
            });
        return avoidForce;
    }

    sf::Vector2f IASystem::PathFollowingForce(
        Registry& registry, const Transform& t, const Physics& ph, const SteeringTarget& st
    ) {
        if (!registry.IsAlive(st.followEntity)) return { 0.f, 0.f };
        auto* path = registry.TryGetComponent<Path>(st.followEntity);
        if (!path || path->points.size() < 2) return { 0.f, 0.f };

        sf::Vector2f dir = length(ph.velocity) > 0.f
            ? normalize(ph.velocity)
            : sf::Vector2f{ 0.f, 0.f };
        sf::Vector2f future = t.position + dir * 10.f;

        float worldRecord = std::numeric_limits<float>::max();
        sf::Vector2f target{ 0.f, 0.f };
        bool found = false;

        const auto& pts = path->points;
        const std::size_t n = pts.size();
        for (std::size_t i = 0; i < n; ++i) {
            const sf::Vector2f& a = pts[i];
            const sf::Vector2f& b = pts[(i + 1) % n];

            sf::Vector2f normalPoint = getNormalPoint(future, a, b);

            float segLen = length(b - a);
            float distToA = length(normalPoint - a);
            float distToB = length(normalPoint - b);
            if (distToA + distToB > segLen + 0.01f) {
                normalPoint = b;
            }

            float distance = length(future - normalPoint);
            if (distance < worldRecord) {
                worldRecord = distance;
                sf::Vector2f segDir = segLen > 0.f ? normalize(b - a) : sf::Vector2f{ 0.f, 0.f };
                target = normalPoint + segDir * 25.f;
                found = true;
            }
        }

        if (!found) return { 0.f, 0.f };

        sf::Vector2f diff = target - t.position;
        if (length(diff) == 0.f) return ph.velocity;
        return normalize(diff) * ph.maxSpeed;
    }

    sf::Vector2f IASystem::getNormalPoint(
        const sf::Vector2f& p, const sf::Vector2f& a, const sf::Vector2f& b
    ) {
        sf::Vector2f ap = p - a;
        sf::Vector2f ab = b - a;
        float abLenSq = ab.x * ab.x + ab.y * ab.y;
        if (abLenSq == 0.f) return a;
        float t = (ap.x * ab.x + ap.y * ab.y) / abLenSq;
        ab *= t;
        return a + ab;
    }

    float IASystem::randomJitter() {
        return (static_cast<float>(std::rand()) / RAND_MAX) * 2.f - 1.f;
    }

    float IASystem::length(sf::Vector2f v) {
        return std::sqrt(v.x * v.x + v.y * v.y);
    }

    sf::Vector2f IASystem::normalize(sf::Vector2f v) {
        float l = length(v);
        return l > 0.f ? v / l : sf::Vector2f{ 0.f, 0.f };
    }

    sf::Vector2f IASystem::limit(sf::Vector2f v, float max) {
        return length(v) > max ? normalize(v) * max : v;
    }

    void IASystem::UpdateLapCounters(Registry& registry) {
        registry.GetView<Transform, LapCounter>().Each(
            [&registry](EntityId, Transform& t, LapCounter& lap) {
                if (!registry.IsAlive(lap.trackEntity)) return;
                auto* path = registry.TryGetComponent<Path>(lap.trackEntity);
                if (!path || path->points.size() < 2) return;

                const auto& pts = path->points;
                const std::size_t n = pts.size();

                std::size_t closestIndex = 0;
                float closestDist = std::numeric_limits<float>::max();
                for (std::size_t i = 0; i < n; ++i) {
                    sf::Vector2f diff = t.position - pts[i];
                    float dist = diff.x * diff.x + diff.y * diff.y;
                    if (dist < closestDist) {
                        closestDist = dist;
                        closestIndex = i;
                    }
                }

                if (!lap.initialized) {
                    lap.lastIndex = closestIndex;
                    lap.initialized = true;
                    lap.passedHalfway = false;
                    return;
                }

                const std::size_t threshold = n / 4;
                const std::size_t halfway = n / 2;

                // SOLUCION: Marcar que el kart llego a la mitad del circuito (Checkpoint)
                if (closestIndex > halfway - threshold && closestIndex < halfway + threshold) {
                    lap.passedHalfway = true;
                }

                // Solo cuenta vuelta si cruzo la linea de meta Y paso por el checkpoint antes
                if (lap.passedHalfway && lap.lastIndex > n - threshold && closestIndex < threshold) {
                    lap.laps++;
                    lap.passedHalfway = false; // Resetear para la siguiente vuelta
                }

                lap.lastIndex = closestIndex;
            });
    }

} // namespace ECS