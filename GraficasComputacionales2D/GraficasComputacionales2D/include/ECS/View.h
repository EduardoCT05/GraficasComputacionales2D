#pragma once
// Project Header
#include "ECS/ComponentPool.h"

namespace ECS {
    template<typename... Components>
    class View {
    public:
        explicit View(ComponentPool<Components>*... pools) noexcept
            : m_pools(pools...) {
            FindSmallest();
        }

        // ── Iteración principal ───────────────────────────────
        // Callback: void(EntityID, Components&...)
        template<typename Func>
        void Each(Func&& func)
        {
            if (!m_smallest) return;

            const auto& entities = m_smallest->GetEntities();

            // Recorrido inverso → seguro al eliminar durante la iteración
            for (std::size_t i = entities.size(); i > 0; --i)
            {
                const EntityId entity = entities[i - 1];
                if (AllHave(entity))
                {
                    std::apply(
                        [&](auto*... pools) {
                            func(entity, pools->Get(entity)...);
                        },
                        m_pools);
                }
            }
        }

        // ── Iteración solo de entidades ───────────────────────
            // Útil cuando solo necesitas el EntityID y accedes a
            // componentes manualmente.
        template<typename Func>
        void EachEntity(Func&& func)
        {
            if (!m_smallest) return;
            const auto& entities = m_smallest->GetEntities();
            for (std::size_t i = entities.size(); i > 0; --i)
            {
                const EntityId entity = entities[i - 1];
                if (AllHave(entity))
                    func(entity);
            }
        }

        [[nodiscard]] bool  Empty() const noexcept { return !m_smallest || m_smallest->Empty(); }
        [[nodiscard]] std::size_t Size() const noexcept { return m_smallest ? m_smallest->Size() : 0; }


    private:
        // Encuentra el pool con menos elementos (mejor filtro)
        template<std::size_t I = 0>
        void FindSmallest() noexcept {
            if constexpr (I < sizeof...(Components)) {
                auto* pool = std::get<I>(m_pools);
                if (pool && (!m_smallest || pool->Size() < m_smallest->Size()))
                    m_smallest = pool;
                FindSmallest<I + 1>();
            }
        }

        [[nodiscard]] bool AllHave(EntityId entity) const noexcept
        {
            return std::apply(
                [entity](auto*... pools) noexcept {
                    return (... && (pools && pools->Contains(entity)));
                },
                m_pools);
        }

    private:
        std::tuple<ComponentPool<Components>*...> m_pools;
        const SparseSet* m_smallest = nullptr;
    };
}