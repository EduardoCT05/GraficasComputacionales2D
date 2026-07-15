#pragma once
// Project Header
#include "Prerequisites.h"
// ============================================================
//  ECS :: Types.h
//  Tipos fundamentales del Entity Component System.
//
//  EntityID  = uint64_t que empaqueta:
//              bits [0..31]  → EntityIndex  (posición en el array)
//              bits [32..63] → EntityVersion (generación; invalida IDs viejos)
//
//  Al destruir una entidad su version sube 1, así cualquier
//  EntityID antiguo guardado en otro sitio queda inválido.
// ============================================================

namespace ECS {
	// ── Tipos primitivos ──────────────────────────────────────
	using EntityIndex = uint32_t;  // Índice de la entidad en el array
	using EntityVersion = uint32_t; // Versión de la entidad (para invalidar IDs antiguos)
	using EntityId = uint64_t;      // ID completo que empaqueta índice y versión
	using ComponentTypeId = uint32_t; // ID de tipo para componentes

	// Valor centinela para "ninguna entidad"
	inline constexpr EntityId NULL_ENTITY = std::numeric_limits<EntityId>::max();
	// ── Empaquetado / desempaquetado de EntityID ─────────────
	/**
 * @brief Extrae el índice de entidad almacenado dentro de un EntityID.
 *
 * Esta función obtiene los 32 bits inferiores de un identificador de entidad.
 * En este sistema, el EntityID está compuesto por dos partes:
 *
 * - Bits 0  - 31: índice de la entidad.
 * - Bits 32 - 63: versión de la entidad.
 *
 * El índice normalmente se utiliza para acceder a la posición de la entidad
 * dentro de un arreglo, pool, sparse set o estructura interna del ECS.
 *
 * @param id Identificador completo de la entidad.
 *
 * @return EntityIndex Índice de la entidad contenido en los 32 bits inferiores.
 *
 * @note La función no modifica ningún estado interno.
 * @note Se marca como noexcept porque no lanza excepciones.
 * @note Se marca como [[nodiscard]] para evitar ignorar accidentalmente el resultado.
 */
	[[nodiscard]] inline EntityIndex GetEntityIndex(EntityId id) noexcept
	{
		return static_cast<EntityIndex>(id & 0xFFFF'FFFFull);
	}

	/**
	 * @brief Extrae la versión de entidad almacenada dentro de un EntityID.
	 *
	 * Esta función obtiene los 32 bits superiores de un identificador de entidad.
	 * La versión se utiliza comúnmente para validar si una entidad sigue siendo válida
	 * o si el identificador apunta a una entidad que ya fue destruida y posiblemente
	 * reutilizada.
	 *
	 * En este sistema, el EntityID está compuesto por:
	 *
	 * - Bits 0  - 31: índice de la entidad.
	 * - Bits 32 - 63: versión de la entidad.
	 *
	 * @param id Identificador completo de la entidad.
	 *
	 * @return EntityVersion Versión de la entidad contenida en los 32 bits superiores.
	 *
	 * @note La función desplaza el identificador 32 bits a la derecha para aislar la versión.
	 * @note Se aplica una máscara de 32 bits para asegurar que solo se conserve la versión.
	 * @note Se marca como [[nodiscard]] porque ignorar la versión podría provocar validaciones incorrectas.
	 */
	[[nodiscard]] inline EntityVersion GetEntityVersion(EntityId id) noexcept
	{
		return static_cast<EntityVersion>((id >> 32) & 0xFFFF'FFFFull);
	}

	/**
	 * @brief Construye un EntityId a partir de un índice y una versión.
	 *
	 * Esta función combina un índice de entidad y una versión en un único identificador
	 * de 64 bits. El índice se almacena en los 32 bits inferiores, mientras que la
	 * versión se almacena en los 32 bits superiores.
	 *
	 * Distribución del EntityID generado:
	 *
	 * - Bits 0  - 31: EntityIndex.
	 * - Bits 32 - 63: EntityVersion.
	 *
	 * Este patrón es común en sistemas ECS para poder reutilizar índices sin perder
	 * la capacidad de detectar referencias inválidas o entidades destruidas.
	 *
	 * @param index Índice de la entidad.
	 * @param version Versión actual de la entidad.
	 *
	 * @return EntityID Identificador único compuesto por índice y versión.
	 *
	 * @note El índice se conserva en los bits inferiores.
	 * @note La versión se desplaza 32 bits a la izquierda antes de combinarse.
	 * @note Se utiliza el operador OR bit a bit para unir ambas partes.
	 */
	[[nodiscard]] inline EntityId MakeEntityId(EntityIndex index, EntityVersion version) noexcept
	{
		return (static_cast<EntityId>(version) << 32) | static_cast<EntityId>(index);
	}

	// ── Generador de IDs de tipo de componente ────────────────
	// Cada tipo T obtiene un ID único en tiempo de ejecución
	// la primera vez que se llama a GetComponentTypeID<T>().
	/**
 * @brief Genera el siguiente identificador único para un tipo de componente.
 *
 * Esta función mantiene un contador estático interno que se incrementa cada vez
 * que se solicita un nuevo ComponentTypeID. Su propósito es asignar un ID único
 * a cada tipo de componente registrado dentro del sistema ECS.
 *
 * El primer componente registrado recibirá el ID 0, el siguiente el ID 1,
 * después el ID 2, y así sucesivamente.
 *
 * @return ComponentTypeID Nuevo identificador único para un tipo de componente.
 *
 * @note El contador es estático, por lo que conserva su valor entre llamadas.
 * @note Se marca como [[nodiscard]] para evitar ignorar accidentalmente el ID generado.
 * @note Se marca como noexcept porque no lanza excepciones.
 *
 * @warning El orden de los IDs depende del orden en que se soliciten por primera vez.
 * @warning Esta versión no es segura para inicialización concurrente desde múltiples hilos.
 */
	[[nodiscard]] inline ComponentTypeId NextComponentTypeId() noexcept
	{
		static ComponentTypeId counter = 0;
		return counter++;
	}

	/**
	 * @brief Obtiene el identificador único asociado a un tipo de componente.
	 *
	 * Esta función plantilla asigna y devuelve un ComponentTypeID único para cada tipo
	 * de componente `T`. Cada especialización de la plantilla mantiene su propio ID
	 * estático, por lo que el identificador se genera una sola vez por tipo y se
	 * reutiliza en llamadas posteriores.
	 *
	 * Por ejemplo, si se llama a:
	 *
	 * @code
	 * GetComponentTypeID<TransformComponent>();
	 * GetComponentTypeID<SpriteComponent>();
	 * GetComponentTypeID<RigidbodyComponent>();
	 * @endcode
	 *
	 * cada tipo de componente recibirá un ID diferente.
	 *
	 * @tparam T Tipo de componente del cual se desea obtener el identificador.
	 *
	 * @return ComponentTypeID Identificador único asociado al tipo de componente `T`.
	 *
	 * @note El ID se genera únicamente la primera vez que se llama esta función para un tipo `T`.
	 * @note Las llamadas posteriores para el mismo tipo devuelven siempre el mismo ID.
	 * @note Es útil para indexar arreglos de componentes, bitsets, firmas de entidades o registros internos del ECS.
	 */
	template<typename T>
	[[nodiscard]] ComponentTypeId GetComponentTypeId() noexcept
	{
		static const ComponentTypeId id = NextComponentTypeId();
		return id;
	}
}