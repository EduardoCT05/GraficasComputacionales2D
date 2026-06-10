#pragma once
#include "Types.h"	
#include "ECS/ComponentPool.h"
#include "ECS/View.h"	
#include "ECS/System.h"

namespace ECS {
	class Registry
	{
	private:
		// -- Entidades --
		std::vector<EntityId> m_entities;
		std::vector<EntityVersion> m_entityVersions;
		std::queue<EntityIndex> m_freelist;

		// -- Componentes --	
		std::unordered_map<ComponentTypeId, std::unique_ptr<IComponentPool>> m_componentPools;

		// -- Sistemas --	
		std::vector<std::unique_ptr<System>> m_systems;
	};
}


