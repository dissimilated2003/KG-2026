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


std::string SaveFileDialog(sf::RenderWindow& window)
{
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    char szFile[MAX_PATH] = { 0 };

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = window.getSystemHandle();
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter =
        "PNG (*.png)\0*.png\0"
        "BMP (*.bmp)\0*.bmp\0"
        "JPEG (*.jpg)\0*.jpg\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrTitle = "Сохранить как";
    ofn.lpstrDefExt = "png";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileNameA(&ofn))
        return std::string(szFile);
    return std::string();
}


bool ColorDialog(sf::RenderWindow& window, sf::Color& outColor)
{
    CHOOSECOLORA cc;
    ZeroMemory(&cc, sizeof(cc));

    static COLORREF customColors[16] = { 0 };

    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = window.getSystemHandle();
    cc.lpCustColors = customColors;
    cc.rgbResult = RGB(outColor.r, outColor.g, outColor.b);
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColorA(&cc))
    {
        outColor = sf::Color(GetRValue(cc.rgbResult),
            GetGValue(cc.rgbResult),
            GetBValue(cc.rgbResult));
        return true;
    }
    return false;
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

    sf::Texture tex{};
    tex.create(SIZE, SIZE);
    tex.setRepeated(true);
    tex.update(img);
    return tex;
}


// класс отвечающий за отрисовку меню
class MenuBar
{
public:
    MenuBar(float width, 
        float height, 
        const sf::Font& font
    )
        : m_font(font)
        , m_height(height)
    {
        m_items = { "File", "Tools" };
        float x = 4.0f;
        for (auto& name : m_items)
        {
            sf::Text t;
            t.setFont(m_font);
            t.setString(name);
            t.setCharacterSize(14);
            t.setFillColor(sf::Color::Black);
            t.setPosition(x + 8.f, 4.f);

            float itemW = t.getLocalBounds().width + 50.f;
            m_bounds.emplace_back(sf::FloatRect(x, 0.f, itemW, m_height));
            m_texts.push_back(t);
            x += itemW;
        }

        m_fileItems = { "New", "Open...", "Save As", "Exit" };

        m_fileNewRect = sf::FloatRect(0.f, m_height, 180.f, MENU_ITEM_HEIGHT);
        m_fileOpenRect = sf::FloatRect(0.f, m_height + MENU_ITEM_HEIGHT, 180.f, MENU_ITEM_HEIGHT);
        m_fileSaveAsRect = sf::FloatRect(0.f, m_height + MENU_ITEM_HEIGHT * 2, 180.f, MENU_ITEM_HEIGHT);
        m_fileExitRect = sf::FloatRect(0.f, m_height + MENU_ITEM_HEIGHT * 3, 180.f, MENU_ITEM_HEIGHT);

        float toolsX = m_bounds[1].left;
        m_toolsColorRect = sf::FloatRect(toolsX, m_height, 180.f, MENU_ITEM_HEIGHT);
    }

    //   1 - new, 2 - open, 3 - save as, 4 - exit, 5 - color
    int HandleEvent(const sf::Event& ev)
    {
        if (ev.type == sf::Event::MouseMoved)
        {
            sf::Vector2f mp(static_cast<float>(ev.mouseMove.x), static_cast<float>(ev.mouseMove.y));
            m_hover = -1;
            for (size_t k = 0; k < m_bounds.size(); ++k)
            {
                if (m_bounds[k].contains(mp))
                {
                    m_hover = static_cast<int>(k);
                    m_openMenu = m_items[k];
                    break;
                }
            }

            m_hoverDropdown = -1;
            if (m_openMenu == "File")
            {
                if (m_fileNewRect.contains(mp))
                {
                    m_hoverDropdown = 0;
                }
                else if (m_fileOpenRect.contains(mp))
                {
                    m_hoverDropdown = 1;
                }
                else if (m_fileSaveAsRect.contains(mp))
                {
                    m_hoverDropdown = 2;
                }
                else if (m_fileExitRect.contains(mp)) 
                {
                    m_hoverDropdown = 3;
                }
            }
            else if (m_openMenu == "Tools")
            {
                if (m_toolsColorRect.contains(mp))
                {
                    m_hoverDropdown = 0;
                }
            }

            bool overTop = (m_hover != -1);
            bool overDropdown = (m_hoverDropdown != -1);
            if (!overTop && !overDropdown)
            {
                m_openMenu = "";
            }
        }
        else if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mp(static_cast<float>(ev.mouseButton.x), static_cast<float>(ev.mouseButton.y));
            for (size_t k = 0; k < m_bounds.size(); ++k)
            {
                if (m_bounds[k].contains(mp))
                {
                    m_openMenu = (m_openMenu == m_items[k]) ? "" : m_items[k];
                    return 0;
                }
            }

            if (m_openMenu == "File")
            {
                if (m_fileNewRect.contains(mp)) 
                { 
                    m_openMenu = ""; return 1; 
                }
                if (m_fileOpenRect.contains(mp)) 
                { 
                    m_openMenu = ""; return 2; 
                }
                if (m_fileSaveAsRect.contains(mp)) 
                { 
                    m_openMenu = ""; return 3; 
                }
                if (m_fileExitRect.contains(mp)) 
                { 
                    m_openMenu = ""; return 4; 
                }
            }
            else if (m_openMenu == "Tools")
            {
                if (m_toolsColorRect.contains(mp)) 
                { 
                    m_openMenu = ""; return 5; 
                }
            }

            if (!m_openMenu.empty())
            {
                m_openMenu = "";
            }
        }

        return 0;
    }

    void Draw(sf::RenderTarget& rt)
    {
        sf::RectangleShape bar(sf::Vector2f(static_cast<float>(rt.getSize().x), m_height));
        bar.setFillColor(sf::Color(240, 240, 240));
        rt.draw(bar);

        sf::RectangleShape line(sf::Vector2f(static_cast<float>(rt.getSize().x), 1.f));
        line.setPosition(0.f, m_height - 1.0f);
        line.setFillColor(sf::Color(180, 180, 180));
        rt.draw(line);

        for (size_t k = 0; k < m_texts.size(); ++k)
        {
            bool highlighted = (static_cast<int>(k) == m_hover) ||
                (m_items[k] == m_openMenu);
            if (highlighted)
            {
                sf::RectangleShape hl(sf::Vector2f(m_bounds[k].width, m_bounds[k].height));
                hl.setPosition(m_bounds[k].left, m_bounds[k].top);
                hl.setFillColor(sf::Color(200, 220, 250));
                rt.draw(hl);
            }
            rt.draw(m_texts[k]);
        }

        if (m_openMenu == "File")
        {
            DrawDropdownItem(rt, m_fileNewRect, "New", m_hoverDropdown == 0);
            DrawDropdownItem(rt, m_fileOpenRect, "Open...", m_hoverDropdown == 1);
            DrawDropdownItem(rt, m_fileSaveAsRect, "Save As...", m_hoverDropdown == 2);
            DrawDropdownItem(rt, m_fileExitRect, "Exit", m_hoverDropdown == 3);
        }
        else if (m_openMenu == "Tools")
        {
            DrawDropdownItem(rt, m_toolsColorRect, "Color...", m_hoverDropdown == 0);
        }
    }

    bool IsMenuArea(const sf::Vector2i& p) const
    {
        if (p.y < static_cast<int>(m_height))
            return true;

        if (m_openMenu == "File" &&
            (m_fileNewRect.contains((float)p.x, (float)p.y) ||
                m_fileOpenRect.contains((float)p.x, (float)p.y) ||
                m_fileSaveAsRect.contains((float)p.x, (float)p.y) ||
                m_fileExitRect.contains((float)p.x, (float)p.y)))
        {
            return true;
        }

        if (m_openMenu == "Tools" && m_toolsColorRect.contains((float)p.x, (float)p.y))
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
    const float MENU_ITEM_HEIGHT = 26.0f;

    float m_height{};
    int m_hover = -1;
    int m_hoverDropdown = -1;
    std::string m_openMenu = "";

    std::vector<std::string> m_items{};
    std::vector<sf::Text> m_texts{};
    std::vector<sf::FloatRect> m_bounds{};
    std::vector<std::string> m_fileItems{};

    sf::FloatRect m_fileNewRect{};
    sf::FloatRect m_fileOpenRect{};
    sf::FloatRect m_fileSaveAsRect{};
    sf::FloatRect m_fileExitRect{};

    sf::FloatRect m_toolsColorRect{};
};


// холст, хранит состояние рисования, операции для работы с растровыми изображениями
class Canvas
{
public:
    void CreateBlank(unsigned width, unsigned height, const sf::FloatRect& viewArea)
    {
        m_image.create(width, height, sf::Color::White);
        m_texture.loadFromImage(m_image);
        m_sprite.setTexture(m_texture, true);
        m_loaded = true;

        RecalculateFit(viewArea);
    }

    void LoadFromFile(const std::string& path, const sf::FloatRect& viewArea)
    {
        if (!m_image.loadFromFile(path))
        {
            m_loaded = false;
            return;
        }

        m_texture.loadFromImage(m_image);
        m_sprite.setTexture(m_texture, true);
        m_loaded = true;
        RecalculateFit(viewArea);
    }

    bool SaveToFile(const std::string& path) const
    {
        return m_image.saveToFile(path);
    }

    bool IsLoaded() const 
    { 
        return m_loaded; 
    }

    void RecalculateFit(const sf::FloatRect& viewArea)
    {
        sf::Vector2u ts = m_image.getSize();
        float sx = viewArea.width / static_cast<float>(ts.x);
        float sy = viewArea.height / static_cast<float>(ts.y);
        float s = std::min({ sx, sy, 1.0f });
        m_scale = { s, s };

        m_center.x = viewArea.left + viewArea.width / 2.0f;
        m_center.y = viewArea.top + viewArea.height / 2.0f;
    }

    void ClampToArea(const sf::FloatRect& viewArea)
    {
        if (!m_loaded)
        {
            return;
        }

        sf::Vector2u ts = m_image.getSize();
        float w = ts.x * m_scale.x;
        float h = ts.y * m_scale.y;
        float halfW = w / 2.0f;
        float halfH = h / 2.0f;

        m_center.x = std::clamp(m_center.x, viewArea.left + halfW, viewArea.left + viewArea.width - halfW);
        m_center.y = std::clamp(m_center.y, viewArea.top + halfH, viewArea.top + viewArea.height - halfH);
    }

    bool OnMousePressed(const sf::Vector2f& mousePos,
        const sf::FloatRect& viewArea,
        const sf::Color& brushColor,
        int brushThickness)
    {
        if (!m_loaded)
        {
            return false;
        }

        sf::Vector2i pixel{};
        if (!WindowToImageCoordinates(mousePos, pixel))
        {
            return false;
        }

        m_painting = true;
        m_insideImage = true;
        m_lastPixel = pixel;
        PaintPixel(pixel.x, pixel.y, brushColor, brushThickness);
        m_texture.update(m_image);

        return true;
    }

    void OnMouseMoved(const sf::Vector2f& mousePos, const sf::Color& brushColor, int brushThickness)
    {
        if (!m_loaded || !m_painting)
        {
            return;
        }

        sf::Vector2i pixel{};
        bool isInsideImage = WindowToImageCoordinates(mousePos, pixel);
        if (!isInsideImage)
        {
            m_insideImage = false;
            return;
        }
        if (!m_insideImage)
        {
            m_lastPixel = pixel;
            m_insideImage = true;
            return;
        }

        DrawLine(m_lastPixel, pixel, brushColor, brushThickness);
        m_lastPixel = pixel;
    }

    void OnMouseReleased()
    {
        m_painting = false;
    }

    bool IsPainting() const 
    { 
        return m_painting; 
    }

    void Draw(sf::RenderTarget& rt, const sf::FloatRect& viewArea) const
    {
        if (!m_loaded)
        {
            return;
        }

        sf::Vector2u ts = m_image.getSize();

        sf::View oldView = rt.getView();
        sf::View clipView(sf::FloatRect(viewArea.left, viewArea.top, viewArea.width, viewArea.height));
        clipView.setViewport(sf::FloatRect(
            viewArea.left / rt.getSize().x,
            viewArea.top / rt.getSize().y,
            viewArea.width / rt.getSize().x,
            viewArea.height / rt.getSize().y
        ));
        rt.setView(clipView);

        sf::Sprite spriteCopy = m_sprite;
        spriteCopy.setOrigin(ts.x / 2.f, ts.y / 2.f);
        spriteCopy.setScale(m_scale);
        spriteCopy.setPosition(m_center);
        rt.draw(spriteCopy);

        rt.setView(oldView);
    }

private:
    sf::Vector2f GetTopLeft() const
    {
        sf::Vector2u ts = m_image.getSize();
        return { 
            m_center.x - ts.x * m_scale.x / 2.0f,
            m_center.y - ts.y * m_scale.y / 2.0f 
        };
    }

    bool WindowToImageCoordinates(const sf::Vector2f& winPos,
        sf::Vector2i& outPixel) const
    {
        if (!m_loaded)
        {
            return false;
        }

        sf::Vector2f topLeft = GetTopLeft();
        float ix = (winPos.x - topLeft.x) / m_scale.x;
        float iy = (winPos.y - topLeft.y) / m_scale.y;

        sf::Vector2u ts = m_image.getSize();
        if (ix < 0.f || iy < 0.f || ix >= static_cast<float>(ts.x) || iy >= static_cast<float>(ts.y))
        {
            return false;
        }

        outPixel = { static_cast<int>(ix), static_cast<int>(iy) };
        return true;
    }

    void PaintPixel(int x, int y, const sf::Color& color, int thickness)
    {
        sf::Vector2u ts = m_image.getSize();
        int r = thickness / 2;

        for (int dy = -r; dy <= r; ++dy)
        {
            for (int dx = -r; dx <= r; ++dx)
            {
                int nx = x + dx;
                int ny = y + dy;
                if (nx < 0 || ny < 0 || nx >= static_cast<int>(ts.x) || ny >= static_cast<int>(ts.y))
                {
                    continue;
                }

                m_image.setPixel(static_cast<unsigned>(nx), static_cast<unsigned>(ny), color);
            }
        }
    }

    void DrawLine(sf::Vector2i a, sf::Vector2i b, const sf::Color& color, int thickness)
    {
        int dx = std::abs(b.x - a.x);
        int dy = -std::abs(b.y - a.y);

        int sx = (a.x < b.x) ? 1 : -1;
        int sy = (a.y < b.y) ? 1 : -1;
        int err = dx + dy;

        while (true)
        {
            PaintPixel(a.x, a.y, color, thickness);
            if (a.x == b.x && a.y == b.y)
            {
                break;
            }

            int e2 = 2 * err;
            if (e2 >= dy) 
            { 
                err += dy; 
                a.x += sx; 
            }
            if (e2 <= dx) 
            { 
                err += dx; 
                a.y += sy; 
            }
        }

        m_texture.update(m_image);
    }

    sf::Image m_image{};
    sf::Texture m_texture{};
    sf::Sprite m_sprite{};
    bool m_loaded = false;
    sf::Vector2f m_center{};
    sf::Vector2f m_scale{ 1.0f, 1.0f };

    bool m_painting = false;
    bool m_insideImage = false;
    sf::Vector2i m_lastPixel{ 0, 0 };
};


//курсор как типа кисточка в паинте
struct BrushCursor
{
    sf::Color color = sf::Color::Black;
    int thickness = 4;

    void Draw(sf::RenderTarget& rt, sf::Vector2f pos) const
    {
        const float CROSS = 13.0f;
        const float RADIUS = thickness / 2.0f;

        sf::RectangleShape hLine(sf::Vector2f(CROSS * 2.0f, 2.0f));
        hLine.setOrigin(CROSS, 0.f);
        hLine.setPosition(pos.x, pos.y/* - RADIUS - 3.0f*/);
        hLine.setFillColor(sf::Color{12,12,12});
        rt.draw(hLine);

        sf::RectangleShape vLine(sf::Vector2f(2.0f, CROSS * 2.0f));
        vLine.setOrigin(0.f, CROSS);
        vLine.setPosition(pos.x/* - RADIUS - 3.0f*/, pos.y);
        vLine.setFillColor(sf::Color{ 12,12,12 });
        rt.draw(vLine);

        sf::CircleShape dot(RADIUS);
        dot.setOrigin(RADIUS, RADIUS);
        dot.setPosition(pos);
        dot.setFillColor(color);
        dot.setOutlineColor(sf::Color::Black);
        dot.setOutlineThickness(1.0f);
        rt.draw(dot);
    }
};


class Application
{
public:
    Application()
        : m_window(sf::VideoMode(WIN_W, WIN_H), "Image painter", sf::Style::Default)
        , m_menu(static_cast<float>(WIN_W), MENU_ITEM_HEIGHT, m_font)
        , m_checkerTexture(MakeCheckerTexture())
        , m_checkerSprite(m_checkerTexture)
        , m_viewArea(0.f, MENU_ITEM_HEIGHT,
            static_cast<float>(WIN_W),
            static_cast<float>(WIN_H) - MENU_ITEM_HEIGHT)
    {
        m_window.setVerticalSyncEnabled(false);
        m_window.setMouseCursor(sf::Cursor());
        m_font.loadFromFile("C:/Windows/Fonts/arial.ttf");

        m_brush.color = sf::Color::Black;
        m_brush.thickness = BRUSH_THICKNESS;

        m_canvas.CreateBlank(WIN_W, WIN_H, m_viewArea);
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
    static constexpr unsigned BRUSH_THICKNESS = 6;


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

        int action = m_menu.HandleEvent(ev);
        if (action != 0)
        {
            OnMenuAction(action);
            return;
        }

        if (ev.type == sf::Event::MouseButtonPressed &&
            ev.mouseButton.button == sf::Mouse::Left)
        {
            if (m_menu.IsMenuArea({ ev.mouseButton.x, ev.mouseButton.y }))
            {
                return;
            }

            sf::Vector2f mp(static_cast<float>(ev.mouseButton.x), static_cast<float>(ev.mouseButton.y));
            m_canvas.OnMousePressed(mp, m_viewArea, m_brush.color, m_brush.thickness);
            return;
        }

        if (ev.type == sf::Event::MouseButtonReleased &&
            ev.mouseButton.button == sf::Mouse::Left)
        {
            m_canvas.OnMouseReleased();
            return;
        }

        if (ev.type == sf::Event::MouseMoved)
        {
            sf::Vector2f mp(static_cast<float>(ev.mouseMove.x),
                static_cast<float>(ev.mouseMove.y));
            m_canvas.OnMouseMoved(mp, m_brush.color, m_brush.thickness);
            return;
        }
    }

    void OnResized(unsigned w, unsigned h)
    {
        m_window.setView(sf::View(sf::FloatRect(0.f, 0.f, static_cast<float>(w), static_cast<float>(h))));

        m_viewArea.width = static_cast<float>(w);
        m_viewArea.height = static_cast<float>(h);

        m_canvas.RecalculateFit(m_viewArea);
        m_canvas.ClampToArea(m_viewArea);
    }

    void OnMenuAction(int action)
    {
        switch (action)
        {
        case 1:
            m_canvas.CreateBlank(m_window.getSize().x, m_window.getSize().y, m_viewArea);
            m_canvas.ClampToArea(m_viewArea);
            break;

        case 2:
        {
            std::string path = OpenFileDialog(m_window);
            if (!path.empty())
            {
                m_canvas.LoadFromFile(path, m_viewArea);
                m_canvas.ClampToArea(m_viewArea);
            }
            break;
        }

        case 3:
        {
            if (!m_canvas.IsLoaded())
            {
                break;
            }

            std::string path = SaveFileDialog(m_window);
            if (!path.empty() && !m_canvas.SaveToFile(path))
            {
                MessageBoxA(m_window.getSystemHandle(),
                    "Не удалось сохранить файл",
                    "Ошибка",
                    MB_OK | MB_ICONERROR);
            }
            break;
        }

        case 4:
            m_window.close();
            break;

        case 5:
            ColorDialog(m_window, m_brush.color);
            break;
        }
    }

    void Render()
    {
        m_window.clear(sf::Color(120, 120, 120));
        int clipH = static_cast<int>(m_window.getSize().y) - static_cast<int>(m_menu.GetHeight());
        m_checkerSprite.setTextureRect(sf::IntRect(0, 0, static_cast<int>(m_window.getSize().x), clipH));
        m_checkerSprite.setPosition(0.f, m_menu.GetHeight());
        m_window.draw(m_checkerSprite);
        m_canvas.Draw(m_window, m_viewArea);
        m_menu.Draw(m_window);

        if (m_canvas.IsLoaded())
        {
            sf::Vector2i mp = sf::Mouse::getPosition(m_window);
            m_brush.Draw(m_window, sf::Vector2f(static_cast<float>(mp.x), static_cast<float>(mp.y)));
        }

        m_window.display();
    }

    sf::RenderWindow m_window;
    sf::Font m_font;
    MenuBar m_menu;
    sf::Texture m_checkerTexture;
    sf::Sprite m_checkerSprite;
    Canvas m_canvas;
    BrushCursor m_brush;
    sf::FloatRect m_viewArea;
};


// задание 2 (100 баллов) - Разработайте приложение, позволяющее рисовать при помощи мыши на
// растровом изображении. Изображение пользователь может либо создать с нуля (File-> New),
// либо выбрать существующий файл изображения (File->Open). Программа должна поддерживать хотя бы
// один из стандартных форматов изображений: PNG, JPEG, BMP. Рисование начинается при нажатии
// левой кнопки мыши внутри клиентской области и продолжается при перемещении мыши при нажатой кнопке
// мыши аналогично инструментам «Кисть» или «Карандаш» в графическом редакторе Paint.
// Толщину рисуемых линий выберите на своё усмотрение.
//
// Пункт меню File->Save As позволяет пользователю сохранить рисунок в одном из следующих форматов:
// PNG, BMP, JPEG.
//
// Бонус - 20 баллов
// Бонус начисляется за возможность выбора цвета рисования. Используйте либо стандартный
// диалог выбора цвета, либо разработайте собственный.
void RunTask2()
{
    Application app;
    app.Run();
}
