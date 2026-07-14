#pragma once
#include "Prerequisites.h"
#include "Types.h"

namespace ECS {
	/**
	 * @class SparseSet
	 * @brief Estructura de datos para asociar entidades a índices densos de forma eficiente.
	 *
	 * Permite inserción, eliminación y consulta en O(1) usando dos arrays paralelos:
	 * - `m_sparse`: mapea EntityIndex -> índice denso.
	 * - `m_dense`: lista compacta de EntityIDs activos.
	 */
	class
		SparseSet {
	public:
		SparseSet() = default;
		virtual ~SparseSet() = default;

		/// @brief ---- Consultas ---- */

		/** @brief Retorna `true` si la entidad está en el set. */
		[[nodisdcard]] bool Contains(EntityId entity) const noexcept {
			const EntityIndex idx = GetEntityIndex(entity);
			if (idx >= m_sparse.size()) return false;
			const EntityIndex denseIdx = m_sparse[idx];
			return denseIdx < m_dense.size() && m_dense[denseIdx] == entity;
		}

		/** @brief Número de entidades registradas. */
		[[nodiscard]] size_t Size() const noexcept { return m_dense.size(); }

		/** @brief Retorna `true` si el set está vacío. */
		[[nodiscard]] bool empty() const noexcept { return m_dense.empty(); }

		/** @brief Referencia constante al array denso de EntityIDs activos. */
		[[nodiscard]] const std::vector<EntityId>& GetEntities() const noexcept {
			return m_dense;
		}



		/**
			 * @brief Elimina una entidad usando swap-with-last para mantener densidad.
			 *
			 * @note Las subclases deben sincronizar sus propios arrays **antes** de
			 * llamar a esta base (ver `ComponentPool::Remove`).
			 * @param entity Entidad a eliminar. Debe estar en el set.
			 */
		virtual void Remove(EntityId entity) {
			const EntityIndex sparseIdx = GetEntityIndex(entity);
			const EntityIndex denseidx = m_sparse[sparseIdx];
			const EntityId last = m_dense.back();

			//mueve el ultimo elemento al hueco
			m_dense[denseidx] = last;
			m_sparse[GetEntityIndex(last)] = denseidx;
			m_dense.pop_back();

			//invlida la entrada eliminada
			m_sparse[sparseIdx] = INVALID;
		}

		/** @brief Vacía completamente sparse y dense. */
		virtual void Clear()
		{
			m_sparse.clear();
			m_dense.clear();
		}

	protected:
		//RESERCA ESPACIO EN M_SPARSE Y REGISTRA LA ENTIDAD EN M_DENSE
		//Devuelve el dense Index asignadao

		EntityIndex InsertEntity(EntityId entity)
		{
			const EntityIndex sparseIdx = GetEntityIndex(entity);
			const EntityIndex denseIdx = static_cast<EntityIndex>(m_dense.size());

			if (sparseIdx >= m_sparse.size())
				m_sparse.resize(sparseIdx + 1, INVALID);
			assert(m_sparse[sparseIdx] == INVALID && "La entidad ya está en el set");

			m_sparse[sparseIdx] = denseIdx;
			m_dense.push_back(entity);
			return denseIdx;

		}

	protected:
		static constexpr EntityIndex INVALID = std::numeric_limits<EntityIndex>::max();

		std::vector<EntityIndex> m_sparse; //sparse[entityIndex] -> dense index
		std::vector<EntityId> m_dense; //dense[i] -> EntityID
	};
}