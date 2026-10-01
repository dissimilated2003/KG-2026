#pragma once

#include <SFML/Graphics.hpp>

#include <map>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>
#include <cctype>
#ifdef _WIN32
#include <windows.h>
#endif

static const char* FONT_PATH = "C:\\Users\\Андрей\\Desktop\\BACKUP\\ООД\\lab4\\FabricPattern\\x64\\Debug\\impact2.ttf";

class Viselitsa 
{
public:
    Viselitsa() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), L"Виселица") 
    {
        Setup();
    }

    // единственная неповторимая публичная функция, запускающая игру
    void Run()
    {
        while (window.isOpen())
        {
            sf::Event event;
            while (window.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    window.close();
                }

                ProcessInput(event);
            }

            Update();
            Render();
        }
    }

private:
    static constexpr int WINDOW_WIDTH = 1200;
    static constexpr int WINDOW_HEIGHT = 720;
    static constexpr int MAX_MISTAKES = 7;

    // это контейнер для подгрузки из словаря
    struct WordData
    {
        std::wstring word;
        std::wstring hint;
        WordData(std::wstring _word, std::wstring _hint) : word(_word), hint(_hint) {}
    };
    std::vector<WordData> m_words;

    // ядро виселицы
    std::wstring m_currentWord;
    std::wstring m_guessedWord;
    std::vector<wchar_t> m_wrongLetters;
    std::vector<wchar_t> m_correctLetters;
    unsigned m_mistakes = 0;
    bool m_gameOver = false;
    bool m_gameWon = false;

    // в главных ролях графики:
    sf::RenderWindow window;
    sf::Font m_font;
    sf::Text m_hintText;

    // кнопка куда жмать:
    struct LetterButton
    {
        sf::RectangleShape shape;
        sf::Text text;
        wchar_t letter;
        bool isPressed = false;
        bool isCorrect = false;
        bool isWrong = false;
    };
    std::vector<LetterButton> m_alphabetButtons;

    // виселица с челом:
    std::vector<sf::RectangleShape> m_gallowsParts;
    std::vector<sf::CircleShape>    m_circleManParts;
    std::vector<sf::RectangleShape> m_rectangleManParts;
    // текста:
    sf::Text m_guessedWordText;
    sf::Text m_gameStateText;
    sf::Text m_restartText;
    // кнопка рестарта:
    sf::RectangleShape m_restartButton;

    // функция билдит текст-подсказку
    void BuildHintText()
    {
        m_hintText.setFont(m_font);
        m_hintText.setCharacterSize(24);
        m_hintText.setFillColor(sf::Color(100, 100, 100));
        m_hintText.setPosition(100, 550);
        m_hintText.setStyle(sf::Text::Regular);
    }

    // вспомогательный костылеход, позволяющий в словаре писать буквами любого регистра
    std::wstring ToUpperWstring(const std::wstring& str) 
    {
        std::wstring result = str;
        for (wchar_t& c : result) 
        {
            if (c >= L'а' && c <= L'я') 
            {
                c = c - (L'а' - L'А');
            }
            else if (c == L'ё') 
            {
                c = L'Ё';
            }
            
        }
        return result;
    }

    bool LoadWordsFromDictionary(const std::string& filename)
    {
        std::ifstream file(filename);
        if (!file.is_open()) 
        {
            std::cerr << "Cannot open file: " << filename << std::endl;
            return false;
        }

        m_words.clear();
        std::string line{};
        while (std::getline(file, line)) 
        {
            if (line.empty()) continue;
            // это хирургическое удаление BOM из UTF-8 файла
            if (line.size() >= 3 &&
                (unsigned char)line[0] == 0xEF &&
                (unsigned char)line[1] == 0xBB &&
                (unsigned char)line[2] == 0xBF) 
            {
                line = line.substr(3);
            }
            
            size_t delimiterPos = line.find('|');
            if (delimiterPos == std::string::npos) continue;

            std::string wordPart = line.substr(0, delimiterPos);
            std::string hintPart = line.substr(delimiterPos + 1);
            // обрезка проблельных символов
            wordPart.erase(0, wordPart.find_first_not_of(" \t\r\n"));
            wordPart.erase(wordPart.find_last_not_of(" \t\r\n") + 1);
            hintPart.erase(0, hintPart.find_first_not_of(" \t\r\n"));
            hintPart.erase(hintPart.find_last_not_of(" \t\r\n") + 1);

            std::wstring word = StringToWideString(wordPart);
            std::wstring hint = StringToWideString(hintPart);

            word = ToUpperWstring(word);

            if (!word.empty() && !hint.empty()) 
            {
                m_words.push_back(Viselitsa::WordData{ word, hint });
            }
        }

        file.close();
        return !m_words.empty();
    }

    // самописная утилитина для substr широких строк
    std::wstring StringToWideString(const std::string& str) 
    {
        if (str.empty()) return L"";

        int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
        std::wstring wstr(sizeNeeded, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wstr[0], sizeNeeded);
        return wstr;
    }

    // метод, вызывающийся в конструкторе виселицы
    void Setup() 
    {    
        if (!m_font.loadFromFile(FONT_PATH)) 
        {
            std::cerr << "Failed to find a font\n";
        }

        if (!LoadWordsFromDictionary("dictionary.txt"))
        { 
            std::cout << "Failed to find a dictionary\n";
        }

        BuildHintText();
        ResetGame();
        BuildAlphabetButtons();
        BuildGallows();
        SetupTexts();
    }

    // функция билдит модель клавиатуры, на которую надо будет нажимать
    void BuildAlphabetButtons() 
    {
        std::wstring russianAlphabet = L"АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ";
        int buttonSize = 50;
        int buttonSpacing = 10;
        int startX = 450;
        int startY = 250;

        for (size_t i = 0; i < russianAlphabet.length(); i++) 
        {
            LetterButton button;
            wchar_t letter = russianAlphabet[i];

            button.shape.setSize(sf::Vector2f(buttonSize, buttonSize));
            button.shape.setFillColor(sf::Color(200, 200, 200));
            button.shape.setOutlineColor(sf::Color::Black);
            button.shape.setOutlineThickness(2);

            int row = i / 10;
            int col = i % 10;
            button.shape.setPosition(startX + col * (buttonSize + buttonSpacing),
                startY + row * (buttonSize + buttonSpacing));

            button.text.setFont(m_font);
            button.text.setString(std::wstring(1, letter));
            button.text.setCharacterSize(24);
            button.text.setFillColor(sf::Color::Black);

            sf::FloatRect textRect = button.text.getLocalBounds();
            button.text.setOrigin(textRect.left + textRect.width / 2.0f,
                textRect.top + textRect.height / 2.0f);
            button.text.setPosition(button.shape.getPosition().x + buttonSize / 2,
                button.shape.getPosition().y + buttonSize / 2);

            button.letter = letter;
            m_alphabetButtons.push_back(button);
        }
    }

    // функция билдит виселицу
    void BuildGallows() 
    {
        sf::RectangleShape base(sf::Vector2f(200, 20));
        base.setPosition(100, 500);
        base.setFillColor(sf::Color(139, 69, 19));
        m_gallowsParts.push_back(base);

        sf::RectangleShape pillar(sf::Vector2f(20, 300));
        pillar.setPosition(190, 200);
        pillar.setFillColor(sf::Color(139, 69, 19));
        m_gallowsParts.push_back(pillar);

        sf::RectangleShape top(sf::Vector2f(150, 20));
        top.setPosition(190, 200);
        top.setFillColor(sf::Color(139, 69, 19));
        m_gallowsParts.push_back(top);

        sf::RectangleShape rope(sf::Vector2f(3, 50));
        rope.setPosition(320, 220);
        rope.setFillColor(sf::Color(100, 100, 100));
        m_gallowsParts.push_back(rope);
    }

    // функция билдит человечка по кусочками, постепенно вешая его в зав. от кол-ва ошибок
    void BulidManParts() 
    {
        m_circleManParts.clear();
        m_rectangleManParts.clear();

        if (m_mistakes >= 1) 
        {
            sf::CircleShape head(25);
            head.setPosition(295, 270);
            head.setFillColor(sf::Color::Transparent);
            head.setOutlineColor(sf::Color::Black);
            head.setOutlineThickness(3);
            m_circleManParts.push_back(head);
        }

        if (m_mistakes >= 2) 
        {
            sf::RectangleShape body(sf::Vector2f(5, 80));
            body.setPosition(320, 320);
            body.setFillColor(sf::Color::Black);
            m_rectangleManParts.push_back(body);
        }

        if (m_mistakes >= 3) 
        {
            sf::RectangleShape leftArm(sf::Vector2f(40, 5));
            leftArm.setPosition(292, 370);
            leftArm.rotate(-45);
            leftArm.setFillColor(sf::Color::Black);
            m_rectangleManParts.push_back(leftArm);
        }

        if (m_mistakes >= 4) 
        { 
            sf::RectangleShape rightArm(sf::Vector2f(40, 5));
            rightArm.setPosition(325, 340);
            rightArm.rotate(45);
            rightArm.setFillColor(sf::Color::Black);
            m_rectangleManParts.push_back(rightArm);
        }

        if (m_mistakes >= 5) 
        {
            sf::RectangleShape leftLeg(sf::Vector2f(40, 5));
            leftLeg.setPosition(292, 425);
            leftLeg.rotate(-45);
            leftLeg.setFillColor(sf::Color::Black);
            m_rectangleManParts.push_back(leftLeg);
        }

        if (m_mistakes >= 6) 
        { 
            sf::RectangleShape rightLeg(sf::Vector2f(40, 5));
            rightLeg.setPosition(325, 395);
            rightLeg.rotate(45);
            rightLeg.setFillColor(sf::Color::Black);
            m_rectangleManParts.push_back(rightLeg);
        }

        if (m_mistakes >= 7) 
        { 
            sf::CircleShape leftEye(3);
            leftEye.setPosition(310, 280);
            leftEye.setFillColor(sf::Color::Black);
            m_circleManParts.push_back(leftEye);

            sf::CircleShape rightEye(3);
            rightEye.setPosition(330, 280);
            rightEye.setFillColor(sf::Color::Black);
            m_circleManParts.push_back(rightEye);

            sf::RectangleShape mouth(sf::Vector2f(20, 3));
            mouth.setPosition(315, 300);
            mouth.setFillColor(sf::Color::Black);
            m_rectangleManParts.push_back(mouth);
        }
    }

    // функция билдит тексты
    void SetupTexts() 
    {
        // текст загаданного слова, который высвечивается при кончине
        m_guessedWordText.setFont(m_font);
        m_guessedWordText.setCharacterSize(48);
        m_guessedWordText.setFillColor(sf::Color::Black);
        m_guessedWordText.setPosition(400, 50);

        // текст "осталось попыток"
        m_gameStateText.setFont(m_font);
        m_gameStateText.setCharacterSize(30);
        m_gameStateText.setFillColor(sf::Color::Red);
        m_gameStateText.setPosition(400, 120);

        // кнопка рестарта
        m_restartButton.setSize(sf::Vector2f(200, 50));
        m_restartButton.setFillColor(sf::Color{ 217, 232, 251 });
        m_restartButton.setOutlineThickness(2.0f);
        m_restartButton.setOutlineColor(sf::Color{ 138, 159, 197 });
        m_restartButton.setPosition(WINDOW_WIDTH - 250, 20);
        m_restartText.setFont(m_font);
        m_restartText.setString(L"Новая игра");
        m_restartText.setCharacterSize(24);
        m_restartText.setFillColor(sf::Color::Black);
        sf::FloatRect textRect = m_restartText.getLocalBounds();
        m_restartText.setOrigin(textRect.left + textRect.width / 2.0f,
            textRect.top + textRect.height / 2.0f);
        m_restartText.setPosition(WINDOW_WIDTH - 150, 45);
    }

    void ResetGame() 
    {
        if (m_words.empty()) 
        {
            std::cerr << "No loaded words!" << std::endl;
            return;
        }
        
        srand(static_cast<unsigned>(time(nullptr)));
        int index = rand() % m_words.size();
        m_currentWord = m_words[index].word;

        m_hintText.setString(L"Подсказка: " + m_words[index].hint);
        m_guessedWord = std::wstring(m_currentWord.length(), L'_'); // замазка

        m_wrongLetters.clear();
        m_correctLetters.clear();
        m_mistakes = 0;
        m_gameOver = false;
        m_gameWon = false;

        for (auto& button : m_alphabetButtons) 
        {
            button.isPressed = false;
            button.isCorrect = false;
            button.isWrong = false;
            button.shape.setOutlineColor(sf::Color::Black);
        }

        m_circleManParts.clear();
        m_rectangleManParts.clear();
    }

    // функция-обработчик нажатий мыши
    void ProcessInput(const sf::Event& event) 
    {
        if (event.type == sf::Event::MouseButtonPressed) 
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            if (m_restartButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) 
            {
                ResetGame();
                return;
            }

            if (m_gameOver) return;

            for (auto& button : m_alphabetButtons) 
            {
                if (button.shape.getGlobalBounds().contains(mousePos.x, mousePos.y) && !button.isPressed) 
                {
                    button.isPressed = true;
                    CheckLetter(button.letter);
                    break;
                }
            }
        }
    }

    // функция, проверяющая буквы
    void CheckLetter(wchar_t letter) 
    {
        if (m_gameOver) return;

        bool found = false;
        std::wstring upperWord = m_currentWord;
        std::wstring upperLetter(1, letter);
        for (size_t i = 0; i < m_currentWord.length(); i++) 
        {
            if (m_currentWord[i] == letter) 
            {
                m_guessedWord[i] = letter;
                found = true;
            }
        }

        if (found) 
        {
            m_correctLetters.push_back(letter);
            for (auto& button : m_alphabetButtons) 
            {
                if (button.letter == letter) 
                {
                    button.isCorrect = true;
                    break;
                }
            }

            if (m_guessedWord == m_currentWord) 
            {
                m_gameWon = true;
                m_gameOver = true;
            }
        }
        else 
        {
            m_wrongLetters.push_back(letter);
            m_mistakes++;

            for (auto& button : m_alphabetButtons) 
            {
                if (button.letter == letter) 
                {
                    button.isWrong = true;
                    break;
                }
            }

            if (m_mistakes >= MAX_MISTAKES) 
            {
                m_gameOver = true;
                m_guessedWord = m_currentWord;
            }
        }

        BulidManParts();
    }

    // функция, которая обновляет отображение загаданного слова и нажатых кнопок
    void Update() 
    {
        std::wstring displayWord;
        for (wchar_t c : m_guessedWord) 
        {
            displayWord += c;
            displayWord += ' ';
        }
        m_guessedWordText.setString(displayWord);

        if (m_gameWon) 
        {
            m_gameStateText.setString(L"Вы живы, учитесь на ПС!");
            m_gameStateText.setFillColor(sf::Color{ 0, 171, 43 });
        }
        else if (m_gameOver) 
        {
            m_gameStateText.setString(L"Вы повешены! Слово: " + m_currentWord);
            m_gameStateText.setFillColor(sf::Color{ 204, 1, 1 });
        }
        else 
        {
            m_gameStateText.setString(L"Осталось попыток: " + std::to_wstring(MAX_MISTAKES - m_mistakes));
            m_gameStateText.setFillColor(sf::Color{ 138, 159, 197 });
        }

        for (auto& button : m_alphabetButtons) 
        {
            if (button.isCorrect) 
            {
                button.shape.setFillColor(sf::Color{173, 255, 194});
                button.shape.setOutlineThickness(2.0f);
                button.shape.setOutlineColor(sf::Color{0, 171, 43});
            }
            else if (button.isWrong) 
            {
                button.shape.setFillColor(sf::Color{255, 189, 191});
                button.shape.setOutlineThickness(2.0f);
                button.shape.setOutlineColor(sf::Color{ 176, 0, 6 });
            }
            else if (button.isPressed) 
            {
                button.shape.setFillColor(sf::Color::Yellow);
            }
            else {
                button.shape.setFillColor(sf::Color(200, 200, 200));
            }
        }
    }

    // функция, которая всё рендерит на окне
    void Render() 
    {
        window.clear(sf::Color::White);

        for (const auto& part : m_gallowsParts) 
        {
            window.draw(part);
        }

        for (const auto& part : m_circleManParts) 
        {
            window.draw(part);
        }

        for (const auto& part : m_rectangleManParts) 
        {
            window.draw(part);
        }

        // буковки кнопочки
        window.draw(m_hintText);
        for (const auto& button : m_alphabetButtons) 
        {
            window.draw(button.shape);
            window.draw(button.text);
        }

        window.draw(m_guessedWordText);
        window.draw(m_gameStateText);

        window.draw(m_restartButton);
        window.draw(m_restartText);

        window.display();
    }

    
};

void RunViselitsa() 
{
    Viselitsa game;
    game.Run();
}