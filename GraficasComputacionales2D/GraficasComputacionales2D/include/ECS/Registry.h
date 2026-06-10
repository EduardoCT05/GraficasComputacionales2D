#pragma once
// Project Header
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



		[[nodiscard]] bool
			IsAlive(EntityId entity) const noexcept {
			const EntityIndex idx = GetEntityIndex(entity);
			return idx < m_entities.size() && m_entities[idx] == entity;
		}

		[[nodiscard]] std::size_t
			EntityCount() const noexcept {
			return m_entities.size() - m_freeList.size();
		}

		// Todas las ranuras (incluye NULL_ENTITY para los huecos libres).
		// Útil para el Serializer; filtra con IsAlive.
		[[nodiscard]] const std::vector<EntityId>&
			GetAllEntities() const noexcept {
			return m_entities;
		}

		// Componentes

		// Añade un componente a la entidad y devuelve su referencia.
		// Acepta argumentos de construcción directos (perfect-forward).
		template<typename T, typename... Args> T&
			AddComponent(EntityId entity, Args&&... args) {
			assert(IsAlive(entity) && "AddComponent: entidad invalida");
			return GetOrCreatePool<T>()->Add(entity, std::forward<Args>(args)...);
		}

		// Elimina el componente T de la entidad (no-op si no lo tiene).
		template<typename T> void
			RemoveComponent(EntityId entity) {
			if (auto* pool = GetPool<T>())
				pool->Remove(entity);
		}

		// Reemplaza el componente (o lo añade si no existía).
		template<typename T>
		T& SetComponent(EntityId entity, T value) {
			assert(IsAlive(entity) && "SetComponent: entidad invalida");
			auto* pool = GetOrCreatePool<T>();
			if (pool->Contains(entity)) {
				pool->Get(entity) = std::move(value);
				return pool->Get(entity);
			}
			return pool->Add(entity, std::move(value));
		}

		template<typename T>
		[[nodiscard]] bool HasComponent(EntityId entity) const noexcept {
			const auto* pool = GetPoolConst<T>();
			return pool && pool->Contains(entity);
		}

		// Acceso garantizado (assert si no existe).
		template<typename T>
		[[nodiscard]] T& GetComponent(EntityId entity) {
			assert(IsAlive(entity));
			auto* pool = GetPool<T>();
			assert(pool && "GetComponent: pool no existe para este tipo");
			return pool->Get(entity);
		}

		template<typename T>
		[[nodiscard]] const T& GetComponent(EntityId entity) const
		{
			assert(IsAlive(entity));
			const auto* pool = GetPoolConst<T>();
			assert(pool && "GetComponent: pool no existe para este tipo");
			return pool->Get(entity);
		}

		// Acceso seguro: devuelve nullptr si la entidad no tiene el componente.
		template<typename T>
		[[nodiscard]] T* TryGetComponent(EntityId entity) noexcept
		{
			auto* pool = GetPool<T>();
			return pool ? pool->TryGet(entity) : nullptr;
		}

		// Views (queries multi-componente)
		// Ejemplo: registry.GetView<Transform, Velocity>()
		template<typename... Components>
		[[nodiscard]] view<Components...> GetView() {
			return View<Components...>(GetOrCreatePool<Components>()...);
		}

		// Sistemas
		template<typename T, typename... Args>
		T& AddSystem(Args&&... args)
		{
			static_assert(std::is_base_of_v<System, T>, "T debe derivar de ECS::System");
			auto system = std::make_unique<T>(std::forward<Args>(args)...);
			T& ref = *system;
			system->OnStart(*this);
			m_systems.push_back(std::move(system));
			return ref;
		}

		void UpdateSystems(float deltaTime)
		{
			for (auto& system : m_systems)
				if (system->IsEnabled())
					system->OnUpdate(*this, deltaTime);
		}

		void RemoveAllSystems()
		{
			for (auto& system : m_systems)
				system->OnDestroy(*this);
			m_systems.clear();
		}

		// Utilidades
		// Destruye todo: entidades, componentes y sistemas.
		void 
		Clear() {
			RemoveAllSystems();
			for (auto& [typeID, pool] : m_componentPools)
				pool->Clear();
			m_entities.clear();
			m_versions.clear();
			while (!m_freeList.empty()) m_freeList.pop();
		}

		//Acceso a pools sin tipo (para el Serializer)
		[[nordiscard]] const std::unordered_map<ComponentTypeId, std::unique_ptr<IComponentPool>>&
			GetPools() const noexcept { return m_componentPools;  }

		private:
			// --- Helpers privados -------------------------
			template<typename T>
			ComponentPool<T>* GetOrCreatePool() {
				const ComponentTypeID typeID = GetComponentTypeID<T>();
				auto it = m_componentPools.find(typeID);
				if (it == m_componentPools.end())
{
					auto [newIt, ok] = m_componentPools.emplace(
						typeID, std::make_unique<ComponentPool<T>>());
					return static_cast<ComponentPool<T>*>(newIt->second.get());
				}
				return static_cast<ComponentPool<T>*>(it->second.get());
			}

			template<typename T>
			ComponentPool<T>* GetPool() noexcept {
				const ComponentTypeID typeID = GetComponentTypeID<T>();
				auto it = m_componentPools.find(typeID);
				return (it != m_componentPools.end())
					? static_cast<ComponentPool<T>*>(it->second.get())
					: nullptr;
			}

			template<typename T>
			const ComponentPool<T>* GetPoolConst() const noexcept {
				const ComponentTypeID typeID = GetComponentTypeID<T>();
				auto it = m_componentPools.find(typeID);
				return (it != m_componentPools.end())
					? static_cast<const ComponentPool<T>*>(it->second.get())
					: nullptr;
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