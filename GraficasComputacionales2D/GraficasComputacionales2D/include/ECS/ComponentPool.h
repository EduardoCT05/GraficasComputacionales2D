#pragma once
// Project Header
#include "Prerequisites.h"
#include "SparseSet.h"

// ============================================================
//  ECS :: ComponentPool.h
//
//  IComponentPool — interfaz polimórfica sin tipo para que
//  Registry pueda gestionar pools heterogéneos.
//
//  ComponentPool<T> — almacena componentes de tipo T en un
//  dense array paralelo al de SparseSet.
//  El Remove usa swap-with-last igual que el SparseSet para
//  mantener los dos arrays sincronizados.
// ============================================================

namespace
	ECS {
	// ── Interfaz polimórfica ──────────────────────────────────
	class
		IComponentPool : public SparseSet {
	public:
		virtual
			~IComponentPool() = default;

		// Elimina el componente de la entidad si existe
		virtual void
			RemoveEntity(EntityId entity) = 0;

		// Puntero sin tipo al componente (para el Serializer)
		virtual void*
			GetRaw(EntityId entity) noexcept = 0;
	};

	// ── Pool tipado ───────────────────────────────────────────
	template<typename T>
	class
		ComponentPool final : public IComponentPool {
	public:
		// ── Añadir ────────────────────────────────────────────
		template<typename... Args> T&
			Add(EntityId entity, Args&&... args) {
			assert(!Contains(entity) && "La entidad ya tiene este componente");
			InsertEntity(entity); // registra en sparse/dense
			m_components.emplace_back(std::forward<Args>(args)...);
			return m_components.back();
		}

		// ── Obtener ───────────────────────────────────────────
		[[nodiscard]] T&
			Get(EntityId entity) noexcept {
			assert(Contains(entity) && "La entidad no tiene este componente");
			return m_components[m_sparse[GetEntityIndex(entity)]];
		}

		[[nodiscard]] const T&
			Get(EntityId entity) const noexcept {
			assert(Contains(entity) && "La entidad no tiene este componente");
			return m_components[m_sparse[GetEntityIndex(entity)]];
		}

		// Devuelve nullptr si la entidad no tiene el componente
		[[nodiscard]] T*
			TryGet(EntityId entity) noexcept {
			if (!Contains(entity)) return nullptr;
			return &m_components[m_sparse[GetEntityIndex(entity)]];
		}

		// ── Eliminar (swap-with-last) ─────────────────────────
	// IMPORTANTE: primero sincronizamos m_components y luego
	// llamamos a SparseSet::Remove para que sincronice m_dense.
	// Ambos swap usan el mismo denseIdx, así quedan alineados.
		void
			Remove(EntityId entity) override {
			if (!Contains(entity)) return;

			const EntityIndex denseIdx = m_sparse[GetEntityIndex(entity)];

			// Mueve el último componente al hueco
			m_components[denseIdx] = std::move(m_components.back());
			m_components.pop_back();

			// Sincroniza sparse/dense (base class)
			SparseSet::Remove(entity);
		}

		void
			RemoveEntity(EntityId entity) override { Remove(entity); }

		void*
			GetRaw(EntityId entity) noexcept override { return TryGet(entity); }

		// ── Acceso masivo (útil para el Serializer / sistemas) ─
		[[nodiscard]] std::vector<T>&
			GetComponents() noexcept { return m_components; }

		[[nodiscard]] const std::vector<T>&
			GetComponents() const noexcept { return m_components; }

		void
			Clear() override {
			SparseSet::Clear();
			m_components.clear();
		}

	private:
		std::vector<T> m_components;   // Paralelo a m_dense
	};
}