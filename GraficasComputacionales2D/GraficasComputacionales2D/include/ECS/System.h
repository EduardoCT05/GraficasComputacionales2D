#pragma once
//=============================================================================
// ECS System.h
// Clase base para todos los sistemas del motor.
//
// Un sistema contiene únicamente lógica, nunca datos.
// Los datos viven en los componentes.
//
// Ciclo de vida: 
// OnStart ->  llamado una vez al registar el sistema 
// OnUpdate ->  llamado cada frame
// OnStop ->  llamado al destruir o desregistrar el sistema
//=============================================================================

namespace ECS {

	// Forward declaration para evitar inclusión circular
	class Registry;

	class System {
	public:
		virtual ~System() = default;

		// Inicialización, reservar recursos, suscribirse a eventos, etc 
		virtual void OnStart(Registry& /*Registry*/) {}

		// Lógica frame a frame. Obligatorio implementar en subclases
		virtual void OnUpdate(Registry& registry, float deltaTime) = 0;

		// Limpieza al destruir o desregistrar el sistema	
		virtual void OnDestroy(Registry& /*Registry*/) {}

		// Activa o	desactiva el sistema sin destruirlo
		virtual void SetEnable(bool enabled) noexcept { m_enabled = enabled; }

		// Retorna true si el sistema está activo.
		[[nodiscard]] bool IsEnabled() const noexcept { return m_enabled; }

	private:
		bool m_enabled = true;
	};

} // namespace ECS