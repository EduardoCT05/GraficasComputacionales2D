#pragma once
#include "ECS/Types.h"	
#include "ECS/ComponentPool.h"
#include "ECS/View.h"	
#include "ECS/System.h"

namespace ECS {
	class Registry {
	public:
		EntityId CreateEntity() {
			EntityIndex idx;
			if (!m_freeList.empty()) {
				idx = m_freeList.front();
				m_freeList.pop();
			}
			else {
				idx = static_cast<EntityIndex>(m_versions.size());
				m_versions.push_back(0);
				m_entities.push_back(NULL_ENTITY);		// placeholder
			}

			EntityId id = CreateEntityID(idx, m_versions[idx]);
			m_entities[idx] = id;
			return id;
		}

		void DestroyEntity(EntityId entity) {
			assert(IsAlive(entity) && "DestroyEntity: entidad invalida o ya destruida");

			// Elimina todos los componentes de esta entidad 
			for (auto& [typeID, pool] : m_componentPools)
				pool->RemoveEntity(entity);

			// Incrementa version -> los IDs viejos quedan invalidos
			const EntityIndex idx = GetEntityIndex(entity);
			++m_versions[idx];
			m_entities[idx] = NULL_ENTITY;
			m_freeList.push(idx);
		}

	private:
		// -- Entidades --
		std::vector<EntityId>      m_entities;
		std::vector<EntityVersion> m_versions;
		std::queue<EntityIndex>    m_freeList;

		// -- Componentes --	
		std::unordered_map<ComponentTypeId, std::unique_ptr<IComponentPool>> m_componentPools;

		// -- Sistemas --	
		std::vector<std::unique_ptr<System>> m_systems;
	};
}