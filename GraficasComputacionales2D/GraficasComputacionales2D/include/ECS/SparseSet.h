#pragma once
// Project Header
#include "prerequisites.h"
#include "ECS/Types.h"	

namespace ECS {

	class 
	Sparse_set {
	public:
		Sparse_set();
		virtual ~Sparse_set() =	default;

		//consultas
		[[nodiscard]] bool Contains(EntityId entity) const noexcept
		{
			const EntityIndex idx = GetEntityIndex(entity);	
			if (idx >= m_sparse.size()) return false;	
			const EntityId denseIdx = m_sparse[idx];	
			return denseIdx < m_dense.size() && m_dense[denseIdx] == entity;
		} 

		[[nodiscard]] size_t Size() const noexcept { return m_dense.size(); }	
		[[nodiscard]] bool Empty() const noexcept { return m_dense.empty(); }
		
		[[nodiscard]] const std::vector<EntityId>& GetEntities() const noexcept 
		{ 
			return m_dense; 
		}

		// --Eliminacion (Swap-with-last)--	
		//LAs sub clases deben llamar a esta base DESPUES de 
		//sincronizar sus propios arrays (ver componentPool::Remove)
		virtual void Remove(EntityId entity)
		{
			if (!Contains(entity)) return;	

			const EntityIndex sparseIdx = GetEntityIndex(entity);	
			const EntityId denseIdx = m_sparse[sparseIdx];	
			const EntityId last = m_dense.back();	

			//Mueve el ultimo elemento al hueco
			m_dense[denseIdx] = last;
			m_sparse[GetEntityIndex(last)] = denseIdx;	
			m_dense.pop_back();

			//Invalida la entrada eliminada	
			m_sparse[sparseIdx] = INVALID;	
		}

		virtual void Clear() 
		{
			m_sparse.clear();
			m_dense.clear();
		}	

	protected: 
		// Reseva espcacio en m_sparce y registra la entidad en m_dense
		// Devuelve en denseIndex asignado 
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

		std::vector<EntityId> m_sparse;	// mapeo de EntityIndex a EntityId (con versión)	
		std::vector<EntityId> m_dense;	// array compacto de EntityId (con versión)
	};
}