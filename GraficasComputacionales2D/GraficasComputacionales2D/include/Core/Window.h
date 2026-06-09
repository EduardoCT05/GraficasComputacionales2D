#pragma once
#include "prerequisites.h"

class
	Window {
public:
	Window() = default;
	Window(int width, int height, const std::string& title);
	~Window() = default;

	//void
	//handLevents(EngineGUI& engineGUI);

	bool
		isOpen() const;

	void
		clear(const sf::Color& color = sf::Color::Black);

	void
		draw(const sf::Drawable& drawable,
			const sf::RenderStates& states = sf::RenderStates::Default);

	void
		display();

	void
		close();

	void
		update();

	void
		render();

	void
		destroy();

public:
	std::unique_ptr<sf::RenderWindow> m_window = nullptr;
private:
	sf::View m_view;
	sf::Time m_deltaTime;
	sf::Clock m_clock;
};
