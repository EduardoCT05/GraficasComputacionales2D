#include "Core/CShape.h"
#include "Core/Window.h"	

CShape::CShape(ShapeType shapeType) : m_shapeType(shapeType), m_shape(createShape(shapeType)) {
}

void 
CShape::draw(Window& window) {
	if (m_shape) {
		window.draw(*m_shape);
	}
}	

sf::Shape* CShape::getShape() 
{
	return m_shape.get();
}	

std::unique_ptr<sf::Shape>
CShape::createShape(ShapeType shapeType) {

	switch (shapeType)
	{
	case EMPTY:
	{
		return nullptr;
	}
	case CIRCLE:
	{
		auto circle = std::make_unique<sf::CircleShape>(50.f);
		circle->setFillColor(sf::Color::Green);
		circle->setPosition({ 100.0f, 100.0f });
		return circle;
	}
	case RECTANGLE:
	{
		auto rectangle = std::make_unique<sf::RectangleShape>(sf::Vector2f(120.f, 60.f));
		rectangle->setFillColor(sf::Color::Blue);
		rectangle->setPosition({ 200.0f, 200.0f });
		return rectangle;
	}
	case TRIANGLE:
	{
		auto triangle = std::make_unique<sf::ConvexShape>(3);
		triangle->setPoint(0, sf::Vector2f(0.f, 0.f));
		triangle->setPoint(1, sf::Vector2f(100.f, 0.f));
		triangle->setPoint(2, sf::Vector2f(50.f, 86.6f)); // height of an equilateral triangle
		triangle->setFillColor(sf::Color::Red);
		triangle->setPosition({ 300.0f, 300.0f });
		return triangle;
	}
	case POLYGON:
	{
		auto polygon = std::make_unique<sf::ConvexShape>(5);
		polygon->setPoint(0, sf::Vector2f(0.f, 0.f));
		polygon->setPoint(1, sf::Vector2f(100.f, 0.f));
		polygon->setPoint(2, sf::Vector2f(120.f, 50.f));
		polygon->setPoint(3, sf::Vector2f(60.f, 100.f));
		polygon->setPoint(4, sf::Vector2f(-20.f, 50.f));
		polygon->setFillColor(sf::Color::Yellow);
		polygon->setPosition({ 400.0f, 400.0f });
		return polygon;
	}
	case LINE:
	{
		auto line = std::make_unique<sf::RectangleShape>(sf::Vector2f(100.f, 5.f));
		line->setFillColor(sf::Color::Magenta);
		line->setPosition({ 500.0f, 500.0f });
		return line;
	}
	default:
		return nullptr;
	}
}
