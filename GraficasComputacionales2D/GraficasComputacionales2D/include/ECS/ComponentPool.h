#pragma once
// Project Header
#include "prerequisites.h"	
#include "ECS/SparseSet.h"

//=============================================================================
// 
//  ECS:: ComponentPool.h
//  IComponent Pool interfaz polimórfica sin tipo para que 
//  Registry pueda gestionar pools heterogéneos.
//
//  Component Pool<T> - almacena componentes de tipo Ten un 
//  dense array paralelo al de SparseSet.
//  El Remove usa swap-with-last igual que el SparseSet para 
//  mantener los dos arrays sincronizados.
//=============================================================================

namespace ECS 
{
	// --Interfaz polimórfica--
	class 
	IComponentPool : public Sparse_set {
	public:
		virtual 
		~IComponentPool() = default;

		// Elimina el componente asociado de la entidad (si existe) 
		virtual void* 
			RemoveEntity(EntityId entity) = 0;

		// punteros sin tipo al componente (para el Serializer)	
		virtual void* 
			GetRaw(EntityId entity) noexcept = 0;	
	};

	// --Pool tipado--	
	template<typename T>
	class 
	ComponentPool final : public IComponentPool {
	public: 
		// añadir 
		template<typename... Args> T& 
		add(EntityId entity, Args&&... args) {
			assert(!Contains(entity) && "La entidad ya tiene este componente");
			instertEntity(entity);	// registra en sparce/dense
			m_components.emplace_back(std::forward<Args>(args)...);
			return m_components.back();
		}

		//-- Obtener --
		[[nodiscard]] T&
			get(EntityId entity) noexcept {
			assert(Contains(entity) && "La entidad no tiene este componente");	
			return m_components[m_sparse[GetEntityIndex(entity)]];
		}	

		[[nodiscard]] const T& 
			get(EntityId entity) const noexcept {
			assert(Contains(entity) && "La entidad no tiene este componente");	
			return m_components[m_sparse[GetEntityIndex(entity)]];
		}	

		// Devuelve nullptr si la entidad no tiene el componente	
		[[nodiscard]] T* 
		Tryget(EntityId entity) noexcept {
			if (!Contains(entity)) return nullptr;	
			return &m_components[m_sparse[GetEntityIndex(entity)]];	
		}

		// -- Eliminar ( swap-with-last) --	
		// importante: primero sincronizamos m_components y luego 
		// llamamos a Sparse_set::Remove para que sincronice m_dense.
		// Ambos swap usan el mismo denseIdx, así quedan alineados.
		void Remove(EntityId entity) noexcept {
			if (!Contains(entity)) return;

			const EntityIndex denseIdx = m_sparse[GetEntityIndex(entity)];

			// Mueve el último componente al hueco	
			m_components[denseIdx] = std::move(m_components.back());
			m_components.pop_back();

			// Sincroniza sparse/dense (base class)
			Sparse_set::Remove(entity);	

		}

		void 
		RemoveEntity (EntityId entity) override { Remove(entity); }

		void* 
		GetRaw(EntityId entity) noexcept override { return Tryget(entity); }

		// -- Accesos masivo (útil para Serializer / sistemas) --
		[[nodiscard]] std::vector<T>&]
		GetComponents() noexcept { return m_components; }

		[[nodiscard]] const std::vector<T>& 
		GetComponents() const noexcept { return m_components; }	

		void 
		Clear() override {
			m_components.clear();
			Sparse_set::Clear();
		}	

	private:
		std:: vector<T> m_components;	// paralelo a m_dense
	};
}