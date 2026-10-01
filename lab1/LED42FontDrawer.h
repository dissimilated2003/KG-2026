#pragma once

#include <SFML/Graphics.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <unordered_map>

class LED42Letter
{
public:
	LED42Letter() = default;

	LED42Letter(sf::Color color, float offsetX, float offsetY, char displayChar)
		: m_color(color)
	{
		m_segments.resize(M_MAX_SEGMENTS);
		BuildScheme(offsetX, offsetY, displayChar);
	}

	explicit LED42Letter(sf::Color color) : m_color(color) 
	{
		m_segments.resize(M_MAX_SEGMENTS);
	}

	void Draw(sf::RenderTarget& target)
	{
		for (const auto& segment : m_segments)
		{
			target.draw(segment);
		}
		//target.display();
	}

private:
	std::vector<sf::RectangleShape> m_segments;
	sf::Color m_color;
	static constexpr unsigned char M_MAX_SEGMENTS = 42;

	// жёсткая прошивка на композицию сегментов
	std::vector<std::pair<float, float>> M_SEGS_POSITIONS = {
		{109.0f, 100.0f}, {114.0f, 100.0f}, {122.0f, 100.0f}, {127.0f, 100.0f},
		{105.0f, 104.0f}, {110.0f, 105.0f}, {118.0f, 104.0f}, {131.0f, 104.0f},
		{104.0f, 111.0f}, {111.0f, 111.0f}, {117.0f, 111.0f}, {130.0f, 111.0f},
		{103.0f, 118.0f}, {112.0f, 118.0f}, {116.0f, 118.0f}, {129.0f, 118.0f},
		{107.0f, 124.0f}, {112.0f, 124.0f}, {119.0f, 124.0f}, {124.0f, 124.0f},
		{102.0f, 128.0f}, {115.0f, 128.0f}, {120.0f, 129.0f}, {128.0f, 128.0f},
		{101.0f, 135.0f}, {114.0f, 135.0f}, {121.0f, 135.0f}, {127.0f, 135.0f},
		{100.0f, 142.0f}, {113.0f, 142.0f}, {122.0f, 142.0f}, {126.0f, 142.0f},
		{104.0f, 148.0f}, {109.0f, 148.0f}, {117.0f, 148.0f}, {122.0f, 148.0f},
		// LED36
		{127.0f, 105.0f}, {124.0f, 111.0f}, {121.0f, 118.0f},
		{111.0f, 129.0f}, {108.0f, 135.0f}, {105.0f, 142.0f}
		// LED42
	};

	// жёсткая прошивка на размеры сегментов
	std::vector<std::pair<float, float>> M_SEGS_ASPECTS = {
		{4.0f, 4.0f}, {4.0f, 4.0f}, {4.0f, 4.0f}, {4.0f, 4.0f},
		{4.0f, 6.0f}, {3.0f, 5.0f}, {4.0f, 6.0f}, {4.0f, 6.0f},
		{4.0f, 6.0f}, {3.0f, 6.0f}, {4.0f, 6.0f}, {4.0f, 6.0f},
		{4.0f, 6.0f}, {3.0f, 5.0f}, {4.0f, 6.0f}, {4.0f, 6.0f},
		{4.0f, 4.0f}, {4.0f, 4.0f}, {4.0f, 4.0f}, {4.0f, 4.0f},
		{4.0f, 6.0f}, {4.0f, 6.0f}, {3.0f, 5.0f}, {4.0f, 6.0f},
		{4.0f, 6.0f}, {4.0f, 6.0f}, {3.0f, 6.0f}, {4.0f, 6.0f},
		{4.0f, 6.0f}, {4.0f, 6.0f}, {3.0f, 5.0f}, {4.0f, 6.0f},
		{4.0f, 4.0f}, {4.0f, 4.0f}, {4.0f, 4.0f}, {4.0f, 4.0f},
		// LED36
		{3.0f, 5.0f}, {3.0f, 6.0f}, {3.0f, 5.0f},
		{3.0f, 5.0f}, {3.0f, 6.0f}, {3.0f, 5.0f}
		// LED42
	};

	std::unordered_map<char, std::vector<unsigned short>> M_LETTER_SEG_COMPOSES = {
		{'0', {1, 2, 3, 4, 8, 12, 16, 24, 28, 32, 36, 35, 34, 33, 29, 25, 21, 13, 9, 5}},
		{'1', {7, 11, 15, 22, 26, 30}},
		{'2', {1, 2, 3, 4, 8, 12, 16, 20, 19, 18, 17, 21, 25, 29, 33, 34, 35, 36}},
		{'3', {1, 2, 3, 4, 8, 12, 16, 20, 19, 18, 17, 24, 28, 32, 36, 35, 34, 33}},
		{'4', {5, 9, 13, 17, 18, 19, 20, 8, 12, 16, 24, 28, 32}},
		{'5', {1, 2, 3, 4, 5, 9, 13, 17, 18, 19, 20, 24, 28, 32, 36, 35, 34, 33}},
		{'6', {1, 2, 3, 4, 5, 9, 13, 21, 25, 29, 33, 34, 35, 36, 32, 28, 24, 20, 19, 18, 17}},
		{'7', {1, 2, 3, 4, 8, 12, 16, 24, 28, 32}},
		{'8', {1, 2, 3, 4, 8, 12, 16, 24, 28, 32, 36, 35, 34, 33, 29, 25, 21, 13, 9, 5, 17, 18, 19, 20}},
		{'9', {1, 2, 3, 4, 8, 12, 16, 24, 28, 32, 36, 35, 34, 33, 13, 9, 5}},
		{'-', {17, 18, 19, 20}},
		{'_', {33, 34, 35, 36}},
		{'+', {11, 15, 22, 26, 17, 18, 19, 20}},
		{'.', {42}},
		{',', {41, 42, 33}},
		{' ', {}},
		{'*', {11, 15, 22, 26, 17, 18, 19, 20, 10, 14, 23, 27, 38, 39, 40, 41}},
		{'^', {1, 6, 10, 14, 5, 9, 13}},
		{':', {11, 26}},
		{';', {11, 41, 42, 33}},
		{'/', {37, 38, 39, 40, 41, 42}},
		{'\\', {6, 10, 14, 23, 27, 31}},
		{'(', {4, 3, 7, 11, 15, 22, 26, 30, 35, 36}},
		{')', {1, 2, 7, 11, 15, 22, 26, 30, 34, 33}},
		{'{', {4, 3, 7, 11, 15, 17, 18, 22, 26, 30, 35, 36}},
		{'}', {1, 2, 7, 11, 15, 19, 20, 22, 26, 30, 34, 33}},
		{'a', {1, 2, 3, 4, 17, 18, 19, 20, 5, 9, 13, 21, 25, 29, 8, 12, 16, 24, 28, 32}},
		{'b', {1, 2, 3, 4, 7, 11, 15, 22, 26, 30, 19, 20, 33, 34, 35, 36, 8, 12, 16, 24, 28, 32}},
		{'c', {1, 2, 3, 4, 5, 9, 13, 21, 25, 29, 33, 34, 35, 36}},
		{'d', {1, 2, 3, 4, 7, 11, 15, 22, 26, 30, 33, 34, 35, 36, 8, 12, 16, 24, 28, 32}},
		{'e', {1, 2, 3, 4, 5, 9, 13, 17, 18, 19, 20, 21, 25, 29, 33, 34, 35, 36}},
		{'f', {1, 2, 3, 4, 5, 9, 13, 17, 18, 19, 20, 21, 25, 29}},
		{'g', {1, 2, 3, 4, 5, 9, 13, 21, 25, 29, 33, 34, 35, 36, 32, 28, 24}},
		{'h', {5, 9 ,13, 21, 25, 29, 17, 18, 19, 20, 8, 12, 16, 24, 28, 32}},
		{'i', {1, 2, 3, 4, 7, 11, 15, 22, 26, 30, 33, 34, 35, 36}},
		{'j', {8, 12, 16, 24, 28, 32, 36, 35, 34, 33}},
		{'k', {5, 9, 13, 21, 25, 29, 17, 18, 19, 20, 37, 38, 39, 24, 28, 32}},
		{'l', {5, 9, 13, 21, 25, 29, 33, 34, 35, 36}},
		{'m', {5, 9, 13, 21, 25, 29, 6, 10, 14, 37, 38, 39, 8, 12, 16, 24, 28, 32}},
		{'n', {5, 9, 13, 21, 25, 29, 6, 10, 14, 23, 27, 31, 8, 12, 16, 24, 28, 32}},
		{'o', {1, 2, 3, 4, 8, 12, 16, 24, 28, 32, 36, 35, 34, 33, 29, 25, 21, 13, 9, 5}},
		{'p', {1, 2, 3, 4, 8, 12, 16, 20, 19, 18, 17, 5, 9 ,13, 21, 25, 29}},
		{'q', {1, 2, 3, 4, 8, 12, 16, 24, 28, 32, 36, 35, 34, 33, 29, 25, 21, 13, 9, 5, 23, 27, 31}},
		{'r', {1, 2, 3, 4, 8, 12, 16, 17, 18, 19, 20, 5, 9 ,13, 21, 25, 29, 23, 27, 31}},
		{'s', {1, 2, 3, 4, 8, 5, 9, 13, 17, 18, 19, 20, 24, 28, 29, 32, 36, 35, 34, 33}},
		{'t', {1, 2, 3, 4, 7, 11, 15, 22, 26, 30 }},
		{'u', {5, 9, 13, 21, 25, 29, 33, 34, 35, 36, 8, 12, 16, 24, 28, 32}},
		{'v', {6, 10, 14, 23, 27, 31, 8, 12, 16, 24, 28, 32}},
		{'w', {5, 9, 13, 21, 25, 29, 42, 41, 40, 23, 27, 31, 32, 28, 24, 16, 12, 8}},
		{'x', {6, 10, 14, 23, 27, 31, 37, 38, 39, 40, 41, 42}},
		{'y', {5, 9, 13, 17, 18, 19, 20, 16, 12, 8, 22, 26, 30}},
		{'z', {1, 2, 3, 4, 37, 38, 39, 40, 41, 42, 33, 34, 35, 36}}
	};

	// -------- METHODS -------------------------
	void BuildScheme(float offsetX, float offsetY, char displayChar)
	{
		for (size_t k = 0; k < M_MAX_SEGMENTS; ++k)
		{
			m_segments[k].setPosition(M_SEGS_POSITIONS[k].first + offsetX, 
				M_SEGS_POSITIONS[k].second + offsetY);
			m_segments[k].setSize({ M_SEGS_ASPECTS[k].first, M_SEGS_ASPECTS[k].second });
			m_segments[k].setFillColor(sf::Color{0, 20, 21});
			m_segments[k].setOutlineColor(sf::Color{ 0, 20, 21 });
		}

		auto it = M_LETTER_SEG_COMPOSES.find(displayChar);
		if (it != M_LETTER_SEG_COMPOSES.end())
		{
			for (unsigned short segNum : it->second)
			{
				short index = segNum - 1;
				if (index >= 0 && index < M_MAX_SEGMENTS)
				{
					m_segments[index].setFillColor(m_color);
					m_segments[index].setOutlineColor(m_color);
				}
			}
		}
	}
};

class LED42Message
{
public:
	LED42Message() = default;
	LED42Message(const std::string& message) : m_message(message) 
	{
		BuildMessageRepresentation();
	}

	void GetMessage(const std::string& fileName = "input.txt")
	{
		std::ifstream file{ fileName };
		std::string message{};
		if (file.is_open())
		{
			std::getline(file, message);
		}
		file.close();
		for (char& ch : message)
		{
			ch = std::tolower(static_cast<unsigned char>(ch));
		}
		m_message = message;
		BuildMessageRepresentation();
	}

	void Draw(sf::RenderTarget& target)
	{
		for (auto& letter : m_letters)
		{
			letter.Draw(target);
		}
	}

private:
	std::string m_message{};
	std::vector<LED42Letter> m_letters{};

	void BuildMessageRepresentation()
	{
		m_letters.clear();

		float startOffset = 0.0f;
		for (size_t k = 0; k < m_message.size(); ++k)
		{
			m_letters.push_back(LED42Letter{ sf::Color{0, 150, 120}, startOffset, 0.0f, m_message[k] });
			startOffset += 40.0f;
		}
	}
};

LED42Message BuildMessageByCtor(const std::string& message)
{
	return LED42Message{ message };
}

LED42Message BuildMessageByFile(const std::string& fileName)
{
	LED42Message msg{};
	msg.GetMessage(fileName);
	return msg;
}

void RenderLED42Message(LED42Message msg)
{
	sf::RenderWindow window(sf::VideoMode({ 1200, 720 }), "LED42 Display");
	

	sf::RenderTexture texture;
	texture.create(1200, 720);
	texture.clear(sf::Color::Black);
	msg.Draw(texture);
	texture.display();

	sf::Sprite sprite(texture.getTexture());
	while (window.isOpen())
	{
		sf::Event event{};
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear(sf::Color::Black);
		window.draw(sprite);
		window.display();

		sf::sleep(sf::milliseconds(10));
	}
}