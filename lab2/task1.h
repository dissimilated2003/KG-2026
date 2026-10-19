#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <iostream>
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <algorithm>


std::string OpenFileDialog(sf::RenderWindow& window)
{
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    char szFile[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window.getSystemHandle();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter =
        "Images (*.png;*.jpg;*.jpeg;*.bmp)\0*.png;*.jpg;*.jpeg;*.bmp\0"
        "PNG (*.png)\0*.png\0"
        "JPEG (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0"
        "BMP (*.bmp)\0*.bmp\0"
        "All files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = "Открыть изображение";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn))
        return std::string(szFile);
    return std::string();
}


sf::Texture MakeCheckerTexture()
{
    const unsigned CELL = 16;
    const unsigned CELLS = 2;
    const unsigned SIZE = CELL * CELLS; // 32x32

    sf::Image img{};
    img.create(SIZE, SIZE, sf::Color::White);
    sf::Color darkGray(150, 150, 150);
    sf::Color lightGray(200, 200, 200);

    for (unsigned y = 0; y < SIZE; ++y)
    {
        for (unsigned x = 0; x < SIZE; ++x)
        {
            bool isLight = ((x / CELL) + (y / CELL)) % 2 == 0;
            img.setPixel(x, y, isLight ? lightGray : darkGray);
        }
    }

    sf::Texture tex;
    tex.create(SIZE, SIZE);
    tex.setRepeated(true);
    tex.update(img);
    return tex;
}


//класс MenuBar отвечает за кастомную отрисовку меню и выпадающего списка
class MenuBar
{
public:
    MenuBar(float width, float height, const sf::Font& font)
        : m_font(font), m_height(height)
    {
        m_items = { "File" };
        float x = 4.f;
        for (auto& name : m_items)
        {
            sf::Text t;
            t.setFont(m_font);
            t.setString(name);
            t.setCharacterSize(14);
            t.setFillColor(sf::Color::Black);
            t.setPosition(x + 8.f, 4.f);

            float itemW = t.getLocalBounds().width + 36.f;
            m_bounds.emplace_back(sf::FloatRect(x, 0.f, itemW, m_height));
            m_texts.push_back(t);
            x += itemW;
        }

        m_fileItems = { "Open...", "Exit" };
        m_fileOpenRect = sf::FloatRect(0.f, m_height, 180.f, 26.f);
        m_fileExitRect = sf::FloatRect(0.f, m_height + 26.f, 180.f, 26.f);
    }

    int HandleEvent(const sf::Event& ev, sf::RenderWindow& window)
    {
        (void)window;
        if (ev.type == sf::Event::MouseMoved)
        {
            sf::Vector2f mp(static_cast<float>(ev.mouseMove.x), static_cast<float>(ev.mouseMove.y));
            m_hover = -1;
            for (size_t k = 0; k < m_bounds.size(); ++k)
            {
                if (m_bounds[k].contains(mp))
                {
                    m_hover = static_cast<int>(k);
                    m_openFileMenu = (m_items[k] == "File");
                    break;
                }
            }

            m_hoverDropdown = -1;
            if (m_openFileMenu)
            {
                if (m_fileOpenRect.contains(mp))
                {
                    m_hoverDropdown = 0;
                }
                else if (m_fileExitRect.contains(mp))
                {
                    m_hoverDropdown = 1;
                }
            }

            m_hoverInsideOpenMenu = m_fileOpenRect.contains(mp) || m_fileExitRect.contains(mp);
        }
        else if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mp(static_cast<float>(ev.mouseButton.x), static_cast<float>(ev.mouseButton.y));
            for (size_t k = 0; k < m_bounds.size(); ++k)
            {
                if (m_bounds[k].contains(mp))
                {
                    m_openFileMenu = (m_items[k] == "File");
                    return 0;
                }
            }

            if (m_openFileMenu)
            {
                if (m_fileOpenRect.contains(mp)) // 1 - open
                {
                    m_openFileMenu = false;
                    return 1;
                }
                if (m_fileExitRect.contains(mp)) // 2 - exit
                {
                    m_openFileMenu = false;
                    return 2;
                }
                m_openFileMenu = false;
            }
        }

        return 0;
    }

    void Draw(sf::RenderTarget& rt)
    {
        sf::RectangleShape bar(sf::Vector2f(static_cast<float>(rt.getSize().x), m_height));
        bar.setFillColor(sf::Color(240, 240, 240));
        rt.draw(bar);

        sf::RectangleShape line(sf::Vector2f(static_cast<float>(rt.getSize().x), 1.0f));
        line.setPosition(0.0f, m_height - 1.0f);
        line.setFillColor(sf::Color(180, 180, 180));
        rt.draw(line);

        for (size_t k = 0; k < m_texts.size(); ++k)
        {
            if (static_cast<int>(k) == m_hover || (m_items[k] == "File" && m_openFileMenu))
            {
                sf::RectangleShape hl(sf::Vector2f(m_bounds[k].width, m_bounds[k].height));
                hl.setPosition(m_bounds[k].left, m_bounds[k].top);
                hl.setFillColor(sf::Color(200, 220, 250));
                rt.draw(hl);
            }
            rt.draw(m_texts[k]);
        }

        if (m_openFileMenu)
        {
            DrawDropdownItem(rt, m_fileOpenRect, "Open...", m_hoverDropdown == 0);
            DrawDropdownItem(rt, m_fileExitRect, "Exit", m_hoverDropdown == 1);
        }
    }

    bool IsMenuArea(const sf::Vector2i& p) const
    {
        if (p.y < static_cast<int>(m_height))
        {
            return true;
        }

        if (m_openFileMenu &&
            (m_fileOpenRect.contains(static_cast<float>(p.x), static_cast<float>(p.y)) ||
                m_fileExitRect.contains(static_cast<float>(p.x), static_cast<float>(p.y))))
        {
            return true;
        }

        return false;
    }

    float GetHeight() const 
    { 
        return m_height; 
    }

private:
    void DrawDropdownItem(sf::RenderTarget& rt, const sf::FloatRect& r,
        const std::string& label, bool hover)
    {
        sf::RectangleShape bg(sf::Vector2f(r.width, r.height));
        bg.setPosition(r.left, r.top);
        bg.setFillColor(hover ? sf::Color(200, 220, 250) : sf::Color(250, 250, 250));
        bg.setOutlineColor(sf::Color(180, 180, 180));
        bg.setOutlineThickness(1.0f);
        rt.draw(bg);

        sf::Text t;
        t.setFont(m_font);
        t.setString(label);
        t.setCharacterSize(14);
        t.setFillColor(sf::Color::Black);
        t.setPosition(r.left + 12.0f, r.top + 4.0f);
        rt.draw(t);
    }

    const sf::Font& m_font{};
    float m_height{};
    int m_hover = -1;
    int m_hoverDropdown = -1;
    bool m_openFileMenu = false;
    bool m_hoverInsideOpenMenu = false;

    std::vector<std::string> m_items{};
    std::vector<sf::Text> m_texts{};
    std::vector<sf::FloatRect> m_bounds{};
    std::vector<std::string> m_fileItems{};

    sf::FloatRect m_fileOpenRect{};
    sf::FloatRect m_fileExitRect{};
};


//класс ImageView отвечает за состояние показываемого изображения
class ImageView
{
public:
    void LoadFromFile(const std::string& path, const sf::FloatRect& viewArea)
    {
        if (!m_texture.loadFromFile(path))
        {
            m_loaded = false;
            return;
        }

        m_texture.setSmooth(true);
        m_sprite.setTexture(m_texture, true);
        m_loaded = true;

        FitToArea(viewArea);
    }

    void FitToArea(const sf::FloatRect& viewArea)
    {
        if (!m_loaded)
        {
            return;
        }

        sf::Vector2u ts = m_texture.getSize();
        float sx = viewArea.width / static_cast<float>(ts.x);
        float sy = viewArea.height / static_cast<float>(ts.y);
        float s = std::min({ sx, sy, 1.0f });
        m_scale = { s, s };

        m_center.x = viewArea.left + viewArea.width / 2.0f;
        m_center.y = viewArea.top + viewArea.height / 2.0f;

        ClampToArea(viewArea);
    }

    void ClampToArea(const sf::FloatRect& viewArea)
    {
        if (!m_loaded)
        {
            return;
        }

        sf::Vector2u ts = m_texture.getSize();
        float w = ts.x * m_scale.x;
        float h = ts.y * m_scale.y;

        float halfW = w / 2.0f;
        float halfH = h / 2.0f;

        m_center.x = std::clamp(m_center.x, viewArea.left + halfW, viewArea.left + viewArea.width - halfW);
        m_center.y = std::clamp(m_center.y, viewArea.top + halfH, viewArea.top + viewArea.height - halfH);
    }

    bool OnMousePressed(const sf::Vector2f& mousePos, const sf::FloatRect& viewArea)
    {
        if (!m_loaded)
        {
            return false;
        }
        if (!viewArea.contains(mousePos))
        {
            return false;
        }
        if (!HitTest(mousePos, viewArea))
        {
            return false;
        }

        m_dragging = true;
        m_dragOffset = mousePos - m_center;
        return true;
    }

    void OnMouseMoved(const sf::Vector2f& mousePos, const sf::FloatRect& viewArea)
    {
        if (!m_loaded || !m_dragging)
        {
            return;
        }

        m_center = mousePos - m_dragOffset;
        ClampToArea(viewArea);
    }

    void OnMouseReleased()
    {
        m_dragging = false;
    }
    
    void OnMouseWheelScrolled(float delta, const sf::FloatRect& viewArea)
    {
        if (!m_loaded)
        {
            return;
        }

        float factor = (delta > 0.f) ? 1.1f : 0.9f;
        m_scale *= factor;
        if (m_scale.x < 0.05f)
        {
            m_scale = { 0.05f, 0.05f };
        }

        ClampToArea(viewArea);
    }

    void Draw(sf::RenderTarget& rt, const sf::FloatRect& viewArea)
    {
        if (!m_loaded)
        {
            return;
        }

        sf::Vector2u ts = m_texture.getSize();

        sf::View oldView = rt.getView();
        sf::View clipView(sf::FloatRect(viewArea.left, viewArea.top, viewArea.width, viewArea.height));
        clipView.setViewport(sf::FloatRect(
            viewArea.left / rt.getSize().x,
            viewArea.top / rt.getSize().y,
            viewArea.width / rt.getSize().x,
            viewArea.height / rt.getSize().y
        ));
        rt.setView(clipView);

        m_sprite.setOrigin(ts.x / 2.0f, ts.y / 2.0f);
        m_sprite.setScale(m_scale);
        m_sprite.setPosition(m_center);
        rt.draw(m_sprite);

        rt.setView(oldView);
    }

    bool IsLoaded() const 
    { 
        return m_loaded; 
    }

private:
    bool HitTest(const sf::Vector2f& p, const sf::FloatRect& viewArea) const
    {
        if (!m_loaded)
        {
            return false;
        }

        sf::Vector2u ts = m_texture.getSize();
        float w = ts.x * m_scale.x;
        float h = ts.y * m_scale.y;

        sf::FloatRect r(m_center.x - w / 2.0f, m_center.y - h / 2.0f, w, h);
        return r.contains(p) && viewArea.contains(p);
    }

    sf::Texture m_texture{};
    sf::Sprite m_sprite{};
    bool m_loaded = false;

    sf::Vector2f m_center{};
    sf::Vector2f m_scale{ 1.0f, 1.0f };

    bool m_dragging = false;
    sf::Vector2f m_dragOffset{};
};


//класс, инкапсулирующий ответственность за запуск приложения
class Application
{
public:
    Application()
        : m_window(sf::VideoMode(WIN_W, WIN_H), "Image view", sf::Style::Default)
        , m_menu(static_cast<float>(WIN_W), MENU_ITEM_HEIGHT, m_font)
        , m_checkerTexture(MakeCheckerTexture())
        , m_checkerSprite(m_checkerTexture)
        , m_viewArea(0.f, MENU_ITEM_HEIGHT,
            static_cast<float>(WIN_W),
            static_cast<float>(WIN_H) - MENU_ITEM_HEIGHT)
    {
        m_window.setVerticalSyncEnabled(false);
        m_font.loadFromFile("C:/Windows/Fonts/arial.ttf");
    }

    void Run()
    {
        while (m_window.isOpen())
        {
            ProcessEvents();
            Render();
        }
    }

private:
    static constexpr unsigned WIN_W = 1000;
    static constexpr unsigned WIN_H = 700;
    static constexpr float MENU_ITEM_HEIGHT = 26.0f;


    void ProcessEvents()
    {
        sf::Event ev;
        while (m_window.pollEvent(ev))
        {
            HandleEvent(ev);
        }
    }

    void HandleEvent(const sf::Event& ev)
    {
        if (ev.type == sf::Event::Closed)
        {
            m_window.close();
            return;
        }

        if (ev.type == sf::Event::Resized)
        {
            OnResized(ev.size.width, ev.size.height);
            return;
        }

        int action = m_menu.HandleEvent(ev, m_window);
        if (action != 0)
        {
            OnMenuAction(action);
            return;
        }

        if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mp(static_cast<float>(ev.mouseButton.x), static_cast<float>(ev.mouseButton.y));
            if (!m_menu.IsMenuArea(sf::Vector2i(ev.mouseButton.x, ev.mouseButton.y)))
            {
                m_image.OnMousePressed(mp, m_viewArea);
            }

            return;
        }

        if (ev.type == sf::Event::MouseButtonReleased &&
            ev.mouseButton.button == sf::Mouse::Left)
        {
            m_image.OnMouseReleased();
            return;
        }

        if (ev.type == sf::Event::MouseMoved)
        {
            sf::Vector2f mp(static_cast<float>(ev.mouseMove.x),
                static_cast<float>(ev.mouseMove.y));
            m_image.OnMouseMoved(mp, m_viewArea);
            return;
        }

        if (ev.type == sf::Event::MouseWheelScrolled)
        {
            if (ev.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
                m_image.OnMouseWheelScrolled(ev.mouseWheelScroll.delta, m_viewArea);
            return;
        }
    }

    void OnResized(unsigned w, unsigned h)
    {
        m_window.setView(sf::View(sf::FloatRect(
            0.f, 
            0.f,
            static_cast<float>(w),
            static_cast<float>(h)))
        );

        m_viewArea.width = static_cast<float>(w);
        m_viewArea.height = static_cast<float>(h) - MENU_ITEM_HEIGHT;

        m_image.FitToArea(m_viewArea);
    }

    void OnMenuAction(int action)
    {
        if (action == 1) //open...
        {
            std::string path = OpenFileDialog(m_window);
            if (!path.empty())
            {
                m_image.LoadFromFile(path, m_viewArea);
                m_image.ClampToArea(m_viewArea);
            }
        }
        else if (action == 2) //exit
        {
            m_window.close();
        }
    }

    void Render()
    {
        m_window.clear(sf::Color(120, 120, 120));

        // шахматка
        int clipH = static_cast<int>(m_window.getSize().y) - static_cast<int>(m_menu.GetHeight());
        m_checkerSprite.setTextureRect(sf::IntRect(
            0, 0,
            static_cast<int>(m_window.getSize().x),
            clipH));
        m_checkerSprite.setPosition(0.f, m_menu.GetHeight());

        m_window.draw(m_checkerSprite);
        m_image.Draw(m_window, m_viewArea);
        m_menu.Draw(m_window);

        m_window.display();
    }

    sf::RenderWindow m_window;
    sf::Font m_font;
    MenuBar m_menu;

    sf::Texture m_checkerTexture;
    sf::Sprite m_checkerSprite;

    ImageView m_image;
    sf::FloatRect m_viewArea;
};


// задание 1 (50 баллов) - Разработайте приложение для просмотра растровых изображений.
// При помощи меню File -> Open должен открыться стандартный диалог выбора файла.
// Должна поддерживаться возможность открытия хотя бы одного из стандартных типов файлов
// изображений: PNG, JPEG, BMP. Для файлов формата PNG-файлов должна поддерживаться полупрозрачность
// (под изображением нарисуйте просвечивающую сквозь него подложку, например, в виде шахматной доски).
//
// При изменении размеров окна изображение должно центрироваться в клиентской области окна.
// За пределы окна изображение выходить не должно – при нехватке места оно должно ресайзиться
// с сохранением оригинальных пропорций.
//
// Бонус – 20 баллов за возможность перемещения изображения внутри окна при помощи мыши.
// Вместо автоматического изменения размеров и центрирования изображения при изменении размеров окна
// реализовать возможность перемещения изображения картинки при помощи курсора мыши методом drag and drop
void RunTask1()
{
    Application app;
    app.Run();
}
