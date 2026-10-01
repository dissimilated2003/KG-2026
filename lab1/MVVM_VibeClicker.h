#pragma once

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>
#include <string_view>


class ClickerModel
{
public:
	ClickerModel() = default;

	void IncrementScore()
	{
		++m_score;
		if (m_onScoreChanged)
		{
			m_onScoreChanged();
		}
	}

	int GetScore() const
	{
		return m_score;
	}

	void SetScoreChangedCallback(std::function<void()> callback)
	{
		m_onScoreChanged = callback;
	}

private:
	int m_score{};
	std::function<void()> m_onScoreChanged;
};


class ClickerViewModel
{
public:
	ClickerViewModel(ClickerModel& clickerModel) : m_clickerModel(clickerModel)
	{
		m_clickerModel.SetScoreChangedCallback(
			[this]() {
				NotifyScoreChanged();
			}
		);
	}

	void OnButtonClicked()
	{
		m_clickerModel.IncrementScore();
	}

	std::string GetScoreText() const
	{
		return "Score: " + std::to_string(m_clickerModel.GetScore());
	}

	void SetScoreTextCallback(std::function<void(const std::string&)> callback)
	{
		m_onScoreTextChanged = callback;
		if (m_onScoreTextChanged)
		{
			m_onScoreTextChanged(GetScoreText());
		}
	}

private:
	ClickerModel& m_clickerModel;
	std::function<void(const std::string&)> m_onScoreTextChanged;

	void NotifyScoreChanged()
	{
		if (m_onScoreTextChanged)
		{
			m_onScoreTextChanged(GetScoreText());
		}
	}
};


class ClickerView
{
public:
	ClickerView(ClickerViewModel& clickerVModel) 
		: m_window(sf::VideoMode{ 400, 300 }, "MVVM Killer")
		, m_viewModel(clickerVModel)
	{
		SetTextFont();
		SetScoreText();
		SetButton();
		SetButtonText();
		SetCursors();
		m_viewModel.SetScoreTextCallback(
			[this](const std::string& text) {
				m_scoreText.setString(text);
			}
		);
	}

	void RunClicker()
	{
		while (m_window.isOpen())
		{
			HandleEvents();
			UpdateCursor();
			Render();
		}
	}

private:
	sf::RenderWindow m_window;
	sf::Font m_font;
	sf::Text m_scoreText;
	sf::RectangleShape m_button;
	sf::Text m_buttonText;
	ClickerViewModel& m_viewModel;

	sf::Cursor m_handCursor;
	sf::Cursor m_arrowCursor;
	bool m_isCursorHand{ false };

	static constexpr unsigned SCORE_TEXT_FONT_SIZE = 32;
	static constexpr unsigned BUTTON_TEXT_FONT_SIZE = 32;
	static constexpr const char* FONT_PATH = "C:\\Users\\Андрей\\Desktop\\BACKUP\\ООД\\lab4\\FabricPattern\\x64\\Debug\\impact2.ttf";

	void SetTextFont()
	{
		m_font.loadFromFile(FONT_PATH);
	}

	void SetScoreText()
	{
		m_scoreText.setFont(m_font);
		m_scoreText.setCharacterSize(SCORE_TEXT_FONT_SIZE);
		m_scoreText.setFillColor(sf::Color::White);
		m_scoreText.setPosition({ 150, 50 });
	}

	void SetButton()
	{
		m_button.setSize({ 150, 100 });
		m_button.setFillColor(sf::Color{ 10, 10, 180 });
		m_button.setPosition({ 125, 100 });
		m_button.setOutlineColor(sf::Color{ 148, 174, 201 });
		m_button.setOutlineThickness(2.0f);
	}

	void SetButtonText()
	{
		m_buttonText.setFont(m_font);
		m_buttonText.setString("Click me");
		m_buttonText.setCharacterSize(BUTTON_TEXT_FONT_SIZE);
		m_buttonText.setFillColor(sf::Color::White);
		m_buttonText.setPosition({ 150, 125 });
	}

	void SetCursors()
	{
		m_handCursor.loadFromSystem(sf::Cursor::Hand);
		m_arrowCursor.loadFromSystem(sf::Cursor::Arrow);
	}

	void HandleEvents()
	{
		sf::Event event{};
		while (m_window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				m_window.close();
			}
			if (event.type == sf::Event::MouseButtonPressed)
			{
				sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
				if (m_button.getGlobalBounds().contains(mousePos.x, mousePos.y))
				{
					m_viewModel.OnButtonClicked();
					
				}
			}
		}
	}

	void UpdateCursor()
	{
		sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
		bool isCursorOverButton = m_button.getGlobalBounds().contains(mousePos.x, mousePos.y);
		if (isCursorOverButton && !m_isCursorHand)
		{
			m_window.setMouseCursor(m_handCursor);
			m_isCursorHand = true;
		}
		else if (!isCursorOverButton && m_isCursorHand)
		{
			m_window.setMouseCursor(m_arrowCursor);
			m_isCursorHand = false;
		}
	}

	void Render()
	{
		m_window.clear(sf::Color::Black);
		m_window.draw(m_scoreText);
		m_window.draw(m_button);
		m_window.draw(m_buttonText);
		m_window.display();
	}
};


void RunClicker()
{
	ClickerModel clickerModel{};
	ClickerViewModel clickerViewModel{ clickerModel };
	ClickerView clickerView{ clickerViewModel };
	clickerView.RunClicker();
}