#pragma once

#include "ECS/Types.h"

// ============================================================
//  ECS :: Components/Camera.h
//
//  Camera — componente de DATOS PUROS. Convierte a una entidad
//  en una cámara. Su POSICIÓN vive en el Transform de la misma
//  entidad (el centro de la vista). Aquí solo van los ajustes
//  propios de la cámara.
//
//  El CameraSystem busca la cámara activa, interpola su
//  Transform hacia el objetivo (si lo hay) y aplica la vista
//  a la ventana.
// ============================================================

namespace ECS {

	struct Camera {
		// Zoom: > 1 acerca (ves menos mundo), < 1 aleja (ves más).
		float zoom{ 1.f };

		// Solo UNA cámara debería estar activa a la vez.
		bool active{ true };

		// Entidad a seguir. NULL_ENTITY = cámara libre (no sigue a nadie).
		EntityId followTarget{ NULL_ENTITY };

		// Velocidad de seguimiento. Mayor = se pega más rápido al objetivo.
		// Es framerate-independent (ver CameraSystem), no es px/seg.
		float followSpeed{ 5.f };
	};

} // namespace ECS