#pragma once
#include "prerequisites.h"	
//=============================================================================
//   ECS:: Types.h
//   Tipos fundamentales del Entity Component System.
// 
//   EntityId = uint64_t que empaqueta :
//              bits[0..31] EntityIndex(posición en el array) 
//				bits[32..63] EntityVersion(generación; invalida IDs viejos) I
// 
//   Al destruir una entidad su version sube 1, así cualquier
//   EntityID antiguo guardado en otro sitio queda inválido.
//=============================================================================

namespace ECS {
	// --Tipos primitivos--
	using EntityIndex = uint32_t;  // Posición de la entidad en el array de entidades	
	using EntityVersion = uint32_t;	//versión de la entidad (para invalidar IDs antiguos)	
	using EntityId = uint64_t;	// ID completo que empaqueta indice y versión de la entidad	
	using ComponentTypeId = uint32_t;	// ID de tipo para componentes 

	// valor centinela para "ninguna entidad"	
	inline constexpr EntityId NULL_ENTITY = std::numeric_limits<EntityId>::max(); //	

	// -- Empaquetado y desempaquetado de EntityId --	
	[[nodiscard]] inline EntityIndex GetEntityIndex(EntityId id) noexcept {
		return static_cast<EntityIndex>(id & 0xFFFFFFFF);
	}

	[[nodiscard]] inline EntityVersion GetEntityVersion(EntityId id) noexcept {
		return static_cast<EntityVersion>((id >> 32) & 0xFFFFFFFF);
	}

	[[nodiscard]] inline EntityId CreateEntityID(EntityIndex index, EntityVersion version) noexcept {
		return (static_cast<EntityId>(version) << 32) | index;
	}

	[[nodiscard]] inline ComponentTypeId GetUniqueComponentTypeId() noexcept 
	{
		static ComponentTypeId counter = 0;
		return counter++;
	}

	template<typename T>
	[[nodiscard]] inline ComponentTypeId GetComponentTypeId() noexcept 
	{
		static ComponentTypeId id = NextComponentTypeId();
		return id;
	}
}