// 1. Librerias de C
#include <cmath>

// 2. Librerias de C++
#include <algorithm>
#include <array>
#include <iostream>
#include <iterator>
#include <map>
#include <vector>

// 3. Librerias de terceros (SFML)
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Window/Keyboard.hpp>

// 4. Encabezados del proyecto
#include "Core/CShape.h"
#include "Core/Window.h"
#include "ECS/Components/Camera.h"
#include "ECS/Components/LapCounter.h"
#include "ECS/Components/Obstacle.h"
#include "ECS/Components/Path.h"
#include "ECS/Components/Physics.h"
#include "ECS/Components/Render.h"
#include "ECS/Components/Sprite.h"
#include "ECS/Components/SteeringTarget.h"
#include "ECS/Components/Transform.h"
#include "ECS/Registry.h"
#include "ECS/Systems/CameraSystem.h"
#include "ECS/Systems/IASystem.h"
#include "ECS/Systems/RenderSystem.h"
#include "ECS/Systems/UISystem.h"
#include "Prerequisites.h"

namespace {
    Window g_window(Window(1024, 768, "Labrid Engine - Mario Kart 2D"));
    ECS::Registry registry;

    // Variable global simple para activar/desactivar el Debug Visual
    bool g_showDebugVisuals = false;
}

void destroy() {
    ImGui::SFML::Shutdown();
}

void DrawMSAASettings() {
    ImGui::Begin("Herramientas de Motor");

    const unsigned int currentLevel = g_window.m_window->getSettings().antiAliasingLevel;

    ImGui::Text("Current MSAA Level: %ux", currentLevel);
    ImGui::Separator();

    static constexpr std::array<unsigned int, 4> msaaLevels{ 0, 2, 4, 8 };
    static constexpr const char* msaaLabels[]{
      "Disabled", "2x MSAA", "4x MSAA", "8x MSAA"
    };

    static int selectedIndex = [&]() {
        const auto iterator = std::find(msaaLevels.begin(), msaaLevels.end(), currentLevel);
        if (iterator == msaaLevels.end()) return 0;
        return static_cast<int>(std::distance(msaaLevels.begin(), iterator));
        }();

    ImGui::SetNextItemWidth(150.0f);
    ImGui::Combo(
        "MSAA Level", &selectedIndex, msaaLabels, static_cast<int>(std::size(msaaLabels))
    );

    ImGui::Separator();

    // EL BOTON DE DEPURACION QUE PIDIO EL PROFE
    ImGui::Text("Depuracion de Inteligencia Artificial");
    ImGui::Checkbox("Mostrar Lineas de Decision (Debug Mode)", &g_showDebugVisuals);
    ImGui::TextWrapped("Visualiza el Path Following y los vectores de decision de la IA en tiempo real.");

    ImGui::End();
}

// Funcion auxiliar para dibujar vectores de la IA (Corregida para compatibilidad estricta SFML)
void DrawVector(sf::RenderWindow& window, sf::Vector2f start, sf::Vector2f dir, sf::Color color) {
    if (dir.x == 0.f && dir.y == 0.f) return;

    sf::VertexArray line(sf::PrimitiveType::Lines, 2);
    line[0].position = start;
    line[0].color = color;
    line[1].position = start + (dir * 0.5f); // Escalado visual para que no cruce toda la pantalla
    line[1].color = color;

    window.draw(line);
}

int main() {
    if (!ImGui::SFML::Init(*g_window.m_window)) return -1;

    sf::Image collisionMask;
    if (!collisionMask.loadFromFile("Textures/pista_mariokart.png")) {
        std::cerr << "Error: No se pudo cargar la mascara de colision.\n";
    }

    registry.AddSystem<ECS::IASystem>();
    registry.AddSystem<ECS::CameraSystem>(g_window);
    registry.AddSystem<ECS::RenderSystem>(g_window);
    registry.AddSystem<ECS::UISystem>();

    sf::Clock deltaClock;

    // --- ENTIDAD 1: PISTA ---
    ECS::EntityId pista = registry.CreateEntity();
    registry.AddComponent<ECS::Sprite>(
        pista, ECS::Sprite::Make("Textures/pista_mariokart.png", sf::Vector2f(0.f, 0.f))
    );

    // --- ENTIDAD 2: RUTA ---
    ECS::EntityId ruta = registry.CreateEntity();
    auto& path = registry.AddComponent<ECS::Path>(ruta);
    path.radius = 12.f;
    path.controlPoints = {
      {564.000f, 162.000f},  {1186.000f, 184.000f}, {1338.000f, 300.000f},
      {1270.000f, 550.000f}, {1378.000f, 720.000f}, {1258.000f, 874.000f},
      {1062.000f, 742.000f}, {978.000f, 554.000f},  {898.000f, 426.000f},
      {566.000f, 536.000f},  {484.000f, 768.000f},  {296.000f, 840.000f},
      {182.000f, 660.000f},  {262.000f, 488.000f},  {220.000f, 300.000f},
      {260.000f, 180.000f}
    };
    path.GenerateSmoothPoints();

    // --- ENTIDAD 3: JUGADOR (TOAD) ---
    ECS::EntityId player = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(player, sf::Vector2f{ 564.f, 130.f }, 0.f, sf::Vector2f{ 1.5f, 1.5f });
    registry.AddComponent<ECS::Render>(player, ECS::Render::Make(RECTANGLE, sf::Color::White, "Textures/Toad.png"));
    registry.AddComponent<ECS::Physics>(player);
    registry.AddComponent<ECS::Obstacle>(player); // Obstaculo para la IA

    auto& playerLap = registry.AddComponent<ECS::LapCounter>(player);
    playerLap.trackEntity = ruta;

    // --- ENTIDAD 4: BOT 1 (BOWSER) ---
    ECS::EntityId bot1 = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(bot1, sf::Vector2f{ 564.f, 190.f }, 0.f, sf::Vector2f{ 1.5f, 1.5f });
    registry.AddComponent<ECS::Render>(bot1, ECS::Render::Make(RECTANGLE, sf::Color::White, "Textures/Bowser.png"));
    registry.AddComponent<ECS::Obstacle>(bot1); // Obstaculo para los demas

    auto& botPhys1 = registry.AddComponent<ECS::Physics>(bot1);
    botPhys1.maxSpeed = 230.f; botPhys1.maxForce = 2500.f; botPhys1.mass = 1.0f;

    auto& st1 = registry.AddComponent<ECS::SteeringTarget>(bot1);
    st1.enabled = true; st1.behavior = ECS::SteeringBehavior::PathFollowing; st1.followEntity = ruta;
    st1.obstacleAvoidForce = 400.f; // Fuerza un poco mas alta para esquivar mejor
    st1.obstacleDetectionRadius = 150.f;

    auto& lap1 = registry.AddComponent<ECS::LapCounter>(bot1);
    lap1.trackEntity = ruta;

    // --- ENTIDAD 5: BOT 2 (MARIO / RED) ---
    // Si no tienes Mario.png, usara la forma base en ROJO
    ECS::EntityId bot2 = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(bot2, sf::Vector2f{ 500.f, 130.f }, 0.f, sf::Vector2f{ 1.5f, 1.5f });
    registry.AddComponent<ECS::Render>(bot2, ECS::Render::Make(RECTANGLE, sf::Color::Red, "Textures/Toad.png"));
    registry.AddComponent<ECS::Obstacle>(bot2);

    auto& botPhys2 = registry.AddComponent<ECS::Physics>(bot2);
    botPhys2.maxSpeed = 220.f; botPhys2.maxForce = 2300.f; botPhys2.mass = 1.0f;

    auto& st2 = registry.AddComponent<ECS::SteeringTarget>(bot2);
    st2.enabled = true; st2.behavior = ECS::SteeringBehavior::PathFollowing; st2.followEntity = ruta;
    st2.obstacleAvoidForce = 400.f; st2.obstacleDetectionRadius = 150.f;

    auto& lap2 = registry.AddComponent<ECS::LapCounter>(bot2);
    lap2.trackEntity = ruta;

    // --- ENTIDAD 6: BOT 3 (LUIGI / GREEN) ---
    ECS::EntityId bot3 = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(bot3, sf::Vector2f{ 500.f, 190.f }, 0.f, sf::Vector2f{ 1.5f, 1.5f });
    registry.AddComponent<ECS::Render>(bot3, ECS::Render::Make(RECTANGLE, sf::Color::Green, "Textures/Bowser.png"));
    registry.AddComponent<ECS::Obstacle>(bot3);

    auto& botPhys3 = registry.AddComponent<ECS::Physics>(bot3);
    botPhys3.maxSpeed = 240.f; botPhys3.maxForce = 2600.f; botPhys3.mass = 1.0f;

    auto& st3 = registry.AddComponent<ECS::SteeringTarget>(bot3);
    st3.enabled = true; st3.behavior = ECS::SteeringBehavior::PathFollowing; st3.followEntity = ruta;
    st3.obstacleAvoidForce = 400.f; st3.obstacleDetectionRadius = 150.f;

    auto& lap3 = registry.AddComponent<ECS::LapCounter>(bot3);
    lap3.trackEntity = ruta;

    // --- CAMARA ---
    ECS::EntityId cam = registry.CreateEntity();
    registry.AddComponent<ECS::Transform>(cam, sf::Vector2f{ 512.f, 384.f });
    auto& camComp = registry.AddComponent<ECS::Camera>(cam);
    camComp.followTarget = player;
    camComp.followSpeed = 5.f; camComp.zoom = 1.0f;

    bool gameOver = false;
    std::string winnerMessage = "";

    while (g_window.isOpen()) {
        while (const std::optional event = g_window.m_window->pollEvent()) {
            ImGui::SFML::ProcessEvent(*g_window.m_window, *event);
            if (event->is<sf::Event::Closed>()) g_window.close();
            else if (const auto* resized = event->getIf<sf::Event::Resized>()) g_window.handleResize(resized->size);
        }

        const float dt = deltaClock.restart().asSeconds();
        ImGui::SFML::Update(*g_window.m_window, sf::seconds(dt));
        g_window.clear(sf::Color::Black);

        DrawMSAASettings();

        std::map<ECS::EntityId, sf::Vector2f> oldPositions;
        registry.GetView<ECS::Transform>().Each([&](ECS::EntityId id, ECS::Transform& t) {
            oldPositions[id] = t.position;
            });

        if (!gameOver) {
            auto* pPhys = registry.TryGetComponent<ECS::Physics>(player);
            auto* pTrans = registry.TryGetComponent<ECS::Transform>(player);

            if (pPhys && pTrans) {
                float turnSpeed = 160.f;
                float moveSpeed = 300.f;

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
                    pTrans->rotation -= turnSpeed * dt;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
                    pTrans->rotation += turnSpeed * dt;

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                    float rad = pTrans->rotation * 3.14159f / 180.f;
                    pPhys->velocity.x = std::cos(rad) * moveSpeed;
                    pPhys->velocity.y = std::sin(rad) * moveSpeed;
                }
                else {
                    pPhys->velocity.x *= 0.94f;
                    pPhys->velocity.y *= 0.94f;
                }
                pTrans->position += pPhys->velocity * dt;
            }
        }

        registry.UpdateSystems(dt);

        // --- SISTEMA DE COLISIONES ---
        float mapW = static_cast<float>(collisionMask.getSize().x);
        float mapH = static_cast<float>(collisionMask.getSize().y);

        registry.GetView<ECS::Transform, ECS::Physics>().Each(
            [&](ECS::EntityId id, ECS::Transform& t, ECS::Physics& p) {
                if (t.position.x < 30.f || t.position.x > mapW - 30.f ||
                    t.position.y < 30.f || t.position.y > mapH - 30.f) {
                    t.position = oldPositions[id]; p.velocity = { 0.f, 0.f }; return;
                }

                unsigned int px = static_cast<unsigned int>(t.position.x);
                unsigned int py = static_cast<unsigned int>(t.position.y);
                sf::Color c = collisionMask.getPixel(sf::Vector2u(px, py));

                bool isMetaZone = (px > 500 && px < 750 && py > 100 && py < 220);
                bool isGrass = (c.g > c.r + 30 && c.g > c.b + 30);
                bool isRedKerb = (c.r > 150 && c.g < 100 && c.b < 100);
                bool isBlueBlock = (c.b > 150 && c.r < 100 && c.g < 100);
                bool isYellowBlock = (c.r > 150 && c.g > 150 && c.b < 100);
                bool isBlackTire = (c.r < 50 && c.g < 50 && c.b < 50) && !isMetaZone;

                bool isWall = (isRedKerb || isBlueBlock || isYellowBlock || isBlackTire) && !isMetaZone;

                if (isWall) {
                    t.position = oldPositions[id]; p.velocity.x *= -0.4f; p.velocity.y *= -0.4f;
                }
                else if (isGrass) {
                    p.velocity.x *= 0.85f; p.velocity.y *= 0.85f;
                }
            });

        // --- RENDERIZADO DEL DEPURADOR VISUAL (GRAPHICAL DEBUGGING) ---
        if (g_showDebugVisuals) {
            // 1. Dibujar Path
            if (path.points.size() > 1) {
                sf::VertexArray pathLines(sf::PrimitiveType::LineStrip, path.points.size());
                for (size_t i = 0; i < path.points.size(); ++i) {
                    pathLines[i].position = path.points[i];
                    pathLines[i].color = sf::Color(255, 255, 0, 150); // Amarillo semi-transparente
                }
                g_window.m_window->draw(pathLines);
            }

            // 2. Dibujar Vectores de Decision de la IA
            registry.GetView<ECS::Transform, ECS::Physics, ECS::SteeringTarget>().Each(
                [&](ECS::EntityId id, ECS::Transform& t, ECS::Physics& p, ECS::SteeringTarget& st) {
                    if (id == player) return; // Solo dibujamos los de la IA

                    // Linea Verde: Hacia donde se esta moviendo fisicamente (Velocidad)
                    DrawVector(*g_window.m_window, t.position, p.velocity, sf::Color::Green);

                    // Linea Azul/Cyan: Hacia donde ESTA PENSANDO ir (Fuerza Deseada de Aceleracion)
                    DrawVector(*g_window.m_window, t.position, p.acceleration * 100.f, sf::Color::Cyan);
                });
        }

        // --- SCOREBOARD Y VUELTAS ---
        auto* pLap = registry.TryGetComponent<ECS::LapCounter>(player);

        ImGui::SetNextWindowPos(ImVec2(10, 250), ImGuiCond_Always);
        ImGui::Begin("Carrera", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

        if (pLap) ImGui::Text("Tú (Toad): %d Vueltas", pLap->laps);

        registry.GetView<ECS::LapCounter, ECS::SteeringTarget>().Each(
            [&](ECS::EntityId id, ECS::LapCounter& l, ECS::SteeringTarget&) {
                ImGui::Text("Bot %llu: %d Vueltas", static_cast<unsigned long long>(id), l.laps);
                if (l.laps >= 3 && !gameOver) {
                    gameOver = true;
                    winnerMessage = "¡PERDISTE! Gano el Bot " + std::to_string(id);
                }
            });

        if (pLap && pLap->laps >= 3 && !gameOver) {
            gameOver = true;
            winnerMessage = "¡GANASTE LA CARRERA!";
        }
        ImGui::End();

        if (gameOver) ImGui::OpenPopup("ResultadoGrande");

        ImGui::SetNextWindowPos(ImVec2(g_window.m_window->getSize().x / 2.0f - 180.f, g_window.m_window->getSize().y / 2.0f - 100.f), ImGuiCond_Always);
        if (ImGui::BeginPopupModal("ResultadoGrande", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::SetWindowFontScale(2.0f);
            if (winnerMessage == "¡GANASTE LA CARRERA!") {
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "%s", winnerMessage.c_str());
            }
            else {
                ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", winnerMessage.c_str());
            }
            ImGui::SetWindowFontScale(1.0f);
            ImGui::Separator();
            if (ImGui::Button("Cerrar Juego", ImVec2(180, 50))) g_window.close();
            ImGui::EndPopup();
        }

        ImGui::SFML::Render(*g_window.m_window);
        g_window.display();
    }

    destroy();
    return 0;
}