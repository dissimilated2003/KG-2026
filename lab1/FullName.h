#pragma once

#include <SFML/Graphics.hpp>

#include <vector>

class Letter
{
public:
    explicit Letter(float phase, sf::Color color)
        : m_phase(phase)
        , m_color(color)
    {}

    void AddRectangle(float x, float y, float width, float height)
    {
        sf::RectangleShape rectangle(sf::Vector2f(width, height));
        rectangle.setPosition({ x, y });
        rectangle.setFillColor(m_color);
        rectangle.setOutlineColor(m_color);
        m_rectangles.push_back(rectangle);
        m_rectOriginalPos.push_back({ x, y });
    }

    void AddPolygon(const std::vector<sf::Vector2f>& points)
    {
        sf::ConvexShape polygon{};
        polygon.setPointCount(points.size());
        for (size_t k = 0; k < points.size(); ++k)
        {
            polygon.setPoint(k, points[k]);
        }
        polygon.setFillColor(m_color);
        polygon.setOutlineColor(m_color);
        m_polygons.push_back(polygon);
        m_polyOriginalPoints.push_back(points);
    }

    // перевернуть
    void Animate(float deltaTime)
    {
        m_time += deltaTime;
        float t = std::fmod(m_time * M_SPEED + m_phase, M_JUMP_PERIOD);
        if (t > 1.0f) // от 0 до 1 беру с параболы
        {
            t = M_JUMP_PERIOD - t;
        }

        float offsetY = -4.0f * M_JUMP_HEIGHT * (t - 0.5f) * (t - 0.5f) + M_JUMP_HEIGHT;
        // y(t) = y0 + v0*t - (g * t^2) / 2
        
        for (size_t i = 0; i < m_rectangles.size(); ++i)
        {
            sf::Vector2f origPos = m_rectOriginalPos[i];
            m_rectangles[i].setPosition(origPos.x, origPos.y + offsetY);
        }

        for (size_t i = 0; i < m_polygons.size(); ++i)
        {
            const auto& origPoints = m_polyOriginalPoints[i];
            sf::ConvexShape& poly = m_polygons[i];
            poly.setPointCount(origPoints.size());
            for (size_t j = 0; j < origPoints.size(); ++j)
            {
                poly.setPoint(j, sf::Vector2f(origPoints[j].x, origPoints[j].y + offsetY));
            }
        }
    }

    void Draw(sf::RenderWindow& window)
    {
        for (const auto& rectangle : m_rectangles)
        {
            window.draw(rectangle);
        }

        for (const auto& polygon : m_polygons)
        {
            window.draw(polygon);
        }
    }

private:
    std::vector<sf::ConvexShape> m_polygons{};
    std::vector<sf::RectangleShape> m_rectangles{};
    float m_phase{};
    float m_time{ 0.0f };
    sf::Color m_color{};

    std::vector<sf::Vector2f> m_rectOriginalPos{};
    std::vector<std::vector<sf::Vector2f>> m_polyOriginalPoints{};
    static constexpr float M_SPEED = 2.0f;
    static constexpr float M_JUMP_PERIOD = 2.0f;
    static constexpr float M_JUMP_HEIGHT = 100.0f;
};

class LettersManager
{
public:
    explicit LettersManager() : m_window(sf::VideoMode(1200, 720), L"Буквенные прыжки")
    {
        m_window.setFramerateLimit(60);
        // фаза mod 2
        m_letters.push_back(CreateLetterB(50.0f, 0.0f, 30.9f, sf::Color{ 68, 94, 171 }));
        m_letters.push_back(CreateLetterA(0.0f, 0.0f, 20.6f, sf::Color{ 125, 129, 141 }));
        m_letters.push_back(CreateLetterO(-50.0f, 0.0f, 10.3f, sf::Color{ 148, 174, 201 }));
    }

    void RunAnimation()
    {
        while (m_window.isOpen())
        {
            float deltaTime = m_clock.restart().asSeconds();

            ProcessEvents();
            AnimationUpdate(deltaTime);
            Render();
        }
    }

private:
    sf::RenderWindow m_window{};
    sf::Clock m_clock{};
    std::vector<Letter> m_letters{};

    void ProcessEvents()
    {
        sf::Event event{};
        while (m_window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                m_window.close();
            }
        }
    }

    void AnimationUpdate(float deltaTime)
    {
        for (auto& letter : m_letters)
        {
            letter.Animate(deltaTime);
        }
    }

    void Render()
    {
        m_window.clear(sf::Color(15, 35, 124));
        for (auto& letter : m_letters)
        {
            letter.Draw(m_window);
        }

        m_window.display();
    }

    Letter CreateLetterB(float offsetX, float offsetY, float phase, sf::Color color) {
        Letter letter{ phase, color };

        letter.AddRectangle(100.0f + offsetX, 100.0f + offsetY, 200.0f, 50.0f);
        letter.AddRectangle(100.0f + offsetX, 100.0f + offsetY, 50.0f, 400.0f);
        letter.AddRectangle(100.0f + offsetX, 275.0f + offsetY, 200.0f, 50.0f);
        letter.AddRectangle(100.0f + offsetX, 450.0f + offsetY, 200.0f, 50.0f);
        letter.AddRectangle(300.0f + offsetX, 150.0f + offsetY, 50.0f, 100.0f);
        letter.AddRectangle(300.0f + offsetX, 350.0f + offsetY, 50.0f, 100.0f);

        std::vector<sf::Vector2f> rightTopSideTriangle = {
            sf::Vector2f{300.0f + offsetX, 100.0f + offsetY},
            sf::Vector2f{350.0f + offsetX, 150.0f + offsetY},
            sf::Vector2f{300.0f + offsetX, 150.0f + offsetY}
        };
        std::vector<sf::Vector2f> rightBottomSideTriangle = {
            sf::Vector2f{300.0f + offsetX, 450.0f + offsetY},
            sf::Vector2f{350.0f + offsetX, 450.0f + offsetY},
            sf::Vector2f{300.0f + offsetX, 500.0f + offsetY}
        };
        std::vector<sf::Vector2f> middleTopSideTriangle = {
            sf::Vector2f{300.0f + offsetX, 250.0f + offsetY},
            sf::Vector2f{350.0f + offsetX, 250.0f + offsetY},
            sf::Vector2f{300.0f + offsetX, 300.0f + offsetY}
        };
        std::vector<sf::Vector2f> middleBottomSideTriangle = {
            sf::Vector2f{300.0f + offsetX, 300.0f + offsetY},
            sf::Vector2f{350.0f + offsetX, 350.0f + offsetY},
            sf::Vector2f{300.0f + offsetX, 350.0f + offsetY}
        };
        std::vector<sf::Vector2f> middleTopInnerTriangle = {
            sf::Vector2f{300.0f + offsetX, 235.0f + offsetY},
            sf::Vector2f{300.0f + offsetX, 285.0f + offsetY},
            sf::Vector2f{250.0f + offsetX, 285.0f + offsetY}
        };
        std::vector<sf::Vector2f> middleBottomInnerTriangle = {
            sf::Vector2f{250.0f + offsetX, 315.0f + offsetY},
            sf::Vector2f{300.0f + offsetX, 315.0f + offsetY},
            sf::Vector2f{300.0f + offsetX, 365.0f + offsetY}
        };

        letter.AddPolygon(rightTopSideTriangle);
        letter.AddPolygon(rightBottomSideTriangle);
        letter.AddPolygon(middleTopSideTriangle);
        letter.AddPolygon(middleBottomSideTriangle);
        letter.AddPolygon(middleTopInnerTriangle);
        letter.AddPolygon(middleBottomInnerTriangle);

        return letter;
    }

    Letter CreateLetterA(float offsetX, float offsetY, float phase, sf::Color color) {
        Letter letter{ phase, color };

        std::vector<sf::Vector2f> leftParallelogram = {
            sf::Vector2f{450.0f + offsetX, 500.0f + offsetY},
            sf::Vector2f{550.0f + offsetX, 100.0f + offsetY},
            sf::Vector2f{600.0f + offsetX, 100.0f + offsetY},
            sf::Vector2f{500.0f + offsetX, 500.0f + offsetY}
        };
        std::vector<sf::Vector2f> rightParallelogram = {
            sf::Vector2f{650.0f + offsetX, 500.0f + offsetY},
            sf::Vector2f{550.0f + offsetX, 100.0f + offsetY},
            sf::Vector2f{600.0f + offsetX, 100.0f + offsetY},
            sf::Vector2f{700.0f + offsetX, 500.0f + offsetY}
        };
        letter.AddPolygon(leftParallelogram);
        letter.AddPolygon(rightParallelogram);
        letter.AddRectangle(500.0f + offsetX, 350.0f + offsetY, 150.0f, 50.0f);

        return letter;
    }

    Letter CreateLetterO(float offsetX, float offsetY, float phase, sf::Color color) {
        Letter letter{ phase, color };

        letter.AddRectangle(850.0f + offsetX, 100.0f + offsetY, 150.0f, 50.0f);
        letter.AddRectangle(850.0f + offsetX, 450.0f + offsetY, 150.0f, 50.0f);
        letter.AddRectangle(800.0f + offsetX, 150.0f + offsetY, 50.0f, 300.0f);
        letter.AddRectangle(1000.0f + offsetX, 150.0f + offsetY, 50.0f, 300.0f);

        std::vector<sf::Vector2f> leftTopTriangle = {
            sf::Vector2f{850.0f + offsetX, 100.0f + offsetY},
            sf::Vector2f{850.0f + offsetX, 150.0f + offsetY},
            sf::Vector2f{800.0f + offsetX, 150.0f + offsetY}
        };
        std::vector<sf::Vector2f> rightTopTriangle = {
            sf::Vector2f{1000.0f + offsetX, 100.0f + offsetY},
            sf::Vector2f{1050.0f + offsetX, 150.0f + offsetY},
            sf::Vector2f{1000.0f + offsetX, 150.0f + offsetY}
        };
        std::vector<sf::Vector2f> rightBottomTriangle = {
            sf::Vector2f{1000.0f + offsetX, 450.0f + offsetY},
            sf::Vector2f{1050.0f + offsetX, 450.0f + offsetY},
            sf::Vector2f{1000.0f + offsetX, 500.0f + offsetY}
        };
        std::vector<sf::Vector2f> leftBottomTriangle = {
            sf::Vector2f{800.0f + offsetX, 450.0f + offsetY},
            sf::Vector2f{850.0f + offsetX, 450.0f + offsetY},
            sf::Vector2f{850.0f + offsetX, 500.0f + offsetY}
        };

        letter.AddPolygon(leftTopTriangle);
        letter.AddPolygon(rightTopTriangle);
        letter.AddPolygon(rightBottomTriangle);
        letter.AddPolygon(leftBottomTriangle);

        return letter;
    }
};