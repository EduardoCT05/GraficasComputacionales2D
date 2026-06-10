#pragma once
#include "ECS/ComponentPool.h"	

namespace ECS {
	template<typename... ComponentTypes>
	class view {
	public:
		explicit View(ComponentPool<Component>*... pools) noexcept
			: m_pools{ pools... } {
			FindSmallest();
		}

		// -- Iteracion principal --
		// Callback: void (EntityId, Components&...)
		template<typename Func> 
		void Each(Func&& func) 
		{
			if (!m_smallest) return;

			const auto& entities = m_smallest->GetEntities();

			// recorrido Inverso -> seguro al eliminar durante la iteracion.
			for (std::size_t i = entities.size(); i-- > 0; )
			{
				const EntityId entity = entities[i - 1];
				if (Allhave(entity))
				{
					std::apply(
						[&](auto*... pools) noexcept {
							func(entity, pools->get(entity)...);
						},
						m_pools);
				}
			}
		}

		// -- Iteracion solo de Entidades --
		// Útil cuando solo necesitas el EntitiID y accedes a 
		// componentes manualemnte 
		template<typename Func>	
		void EachEntity(Func&& func) 
		{
			if (!m_smallest) return;
			const auto& entities = m_smallest->GetEntities();
			for (std::size_t i = entities.size(); i-- > 0; )
			{
				const EntityId entity = entities[i - 1];
				if (Allhave(entity))
					func(entity);
			}
		}

		[[nodiscard]] bool Empty() const noexcept { return !m_smallest || m_smallest->Empty();}
		[[nodiscard]] std::size_t Size() const noexcept { return m_s mallest ? m_smallest->Size() : 0; }


	private:
		// Encuentra el pool con menos elementos (mejor filtro)
		template<std::size_t... Is>
		void FindSmallest() noexcept {
			if constexpr (sizeof...(ComponentTypes) > 0) {
				auto* pool = std::get<I>(m_pools);
				if (pool && (!m_smallest || pool->Size() < m_smallest->Size())) {
					m_smallest = pool;
					FindSmallest<I + 1>();
				}
			}

			[[nodiscard]] bool Allhave(EntityID entity) const noexcept
			{
				return std::apply(
					[entity](auto*... pools) noexcept {
						return (... && (pools && pools->Contains(entity)));)
					},
					m_pools);
			}
	private:
		std::tuple<ComponentPool<Component>*...> m_pools;
		const SparseSet* m_smallest = nullptr;
		}
	};
}
