#pragma once


#include "Utils.h"

#include <array>
#include <memory>
#include <random>
#include <string>


using Shape = std::array<std::array<int, 4>, 4>;

static glm::vec3 GetColorById(int id)
{
	switch (id)
	{
	case 1: return { 0.2f, 0.7f, 0.9f };   //I голубой
	case 2: return { 0.95f, 0.85f, 0.2f }; //O жёлтый
	case 3: return { 0.7f, 0.3f, 0.8f };   //T фиолетовый
	case 4: return { 0.3f, 0.8f, 0.3f };   //S зелёный
	case 5: return { 0.9f, 0.2f, 0.2f };   //Z красный
	case 6: return { 0.2f, 0.3f, 0.9f };   //J синий
	case 7: return { 0.95f, 0.6f, 0.2f };  //L оранжевый
	}

	return { 0.5f, 0.5f, 0.5f };
}


//интерфейс "Тетрамино", используется для работы с разными реализациями
//типов тетрамино: I, O, T, S, Z, J, L
class Tetromino
{
public:
	virtual ~Tetromino() = default;

	virtual const Shape& GetShape() const = 0;
	virtual int GetColor() const = 0;
	virtual glm::ivec2 GetPosition() const = 0;
	virtual void SetPosition(glm::ivec2 pos) = 0;
	virtual void RotateCW() = 0;
};


//промежуточный класс для создания наследников (им нужно всего лишь задать форму в 4х4 коробке)
class TetrominoBase : public Tetromino
{
public:
	const Shape& GetShape() const override 
	{ 
		return m_shape; 
	}

	int GetColor() const override 
	{ 
		return m_color; 
	}

	glm::ivec2 GetPosition() const override 
	{ 
		return m_pos; 
	}

	void SetPosition(glm::ivec2 p) override 
	{ 
		m_pos = p; 
	}

	void RotateCW() override
	{
		Shape rotated{};
		for (int y = 0; y < 4; ++y)
		{
			for (int x = 0; x < 4; ++x)
			{
				rotated[y][x] = m_shape[x][3 - y];
			}
		}
		m_shape = rotated;
	}

protected:
	Shape m_shape{};
	int m_color = 0;
	glm::ivec2 m_pos{ 0, 0 };
};


class ITetromino : public TetrominoBase
{
public:
	ITetromino()
	{
		m_color = 1;  //голубой
		m_shape = { {
			{{0,0,0,0}},
			{{1,1,1,1}},
			{{0,0,0,0}},
			{{0,0,0,0}},
		} };
	}
};


class OTetromino : public TetrominoBase
{
public:
	OTetromino()
	{
		m_color = 2;  //желтый
		m_shape = { {
			{{1,1,0,0}},
			{{1,1,0,0}},
			{{0,0,0,0}},
			{{0,0,0,0}},
		} };
	}

	void RotateCW() override {}
};


class TTetromino : public TetrominoBase
{
public:
	TTetromino()
	{
		m_color = 3;  //фиолетовый
		m_shape = { {
			{{0,1,0,0}},
			{{1,1,1,0}},
			{{0,0,0,0}},
			{{0,0,0,0}},
		} };
	}
};


class STetromino : public TetrominoBase
{
public:
	STetromino()
	{
		m_color = 4;  //зеленый
		m_shape = { {
			{{0,1,1,0}},
			{{1,1,0,0}},
			{{0,0,0,0}},
			{{0,0,0,0}},
		} };
	}
};


class ZTetromino : public TetrominoBase
{
public:
	ZTetromino()
	{
		m_color = 5;  //красный
		m_shape = { {
			{{1,1,0,0}},
			{{0,1,1,0}},
			{{0,0,0,0}},
			{{0,0,0,0}},
		} };
	}
};


class JTetromino : public TetrominoBase
{
public:
	JTetromino()
	{
		m_color = 6;  //синий
		m_shape = { {
			{{1,0,0,0}},
			{{1,1,1,0}},
			{{0,0,0,0}},
			{{0,0,0,0}},
		} };
	}
};


class LTetromino : public TetrominoBase
{
public:
	LTetromino()
	{
		m_color = 7;  //оранжевый
		m_shape = { {
			{{0,0,1,0}},
			{{1,1,1,0}},
			{{0,0,0,0}},
			{{0,0,0,0}},
		} };
	}
};


//фабрика порождающая рандомный тетрамино
std::unique_ptr<Tetromino> CreateRandomTetromino(std::mt19937& rng)
{
	std::uniform_int_distribution<int> dist(0, 6);
	switch (dist(rng))
	{
	case 0: return std::make_unique<ITetromino>();
	case 1: return std::make_unique<OTetromino>();
	case 2: return std::make_unique<TTetromino>();
	case 3: return std::make_unique<STetromino>();
	case 4: return std::make_unique<ZTetromino>();
	case 5: return std::make_unique<JTetromino>();
	case 6: return std::make_unique<LTetromino>();
	}

	return nullptr;
}


//стакан 10х20 - главное окно игры
class Arena
{
public:
	static constexpr int WIDTH = 10;
	static constexpr int HEIGHT = 20;

	Arena()
	{
		Clear();
	}

	void Clear()
	{
		for (int y = 0; y < HEIGHT; ++y)
		{
			for (int x = 0; x < WIDTH; ++x)
			{
				m_grid[y][x] = 0;
			}
		}
	}

	bool IsEmptyCell(int x, int y) const
	{
		if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT)
		{
			return false;
		}

		return m_grid[y][x] == 0;
	}

	bool CanPlace(const Tetromino& t, glm::ivec2 offset = { 0, 0 }) const
	{
		const Shape& shape = t.GetShape();
		glm::ivec2 pos = t.GetPosition() + offset;

		for (int sy = 0; sy < 4; ++sy)
		{
			for (int sx = 0; sx < 4; ++sx)
			{
				if (shape[sy][sx])
				{
					int gx = pos.x + sx;
					int gy = pos.y + sy;
					if (!IsEmptyCell(gx, gy))
					{
						return false;
					}
				}
			}
		}

		return true;
	}

	void Place(const Tetromino& t)
	{
		const Shape& shape = t.GetShape();
		glm::ivec2 pos = t.GetPosition();

		for (int sy = 0; sy < 4; ++sy)
		{
			for (int sx = 0; sx < 4; ++sx)
			{
				if (shape[sy][sx])
				{
					int gx = pos.x + sx;
					int gy = pos.y + sy;
					if (gx >= 0 && gx <= WIDTH && gy >= 0 && gy <= HEIGHT)
					{
						m_grid[gy][gx] = t.GetColor();
					}
				}
			}
		}
	}

	int ClearFullLines()
	{
		int linesCleared = 0;
		for (int y = 0; y < HEIGHT; ++y)
		{
			bool full = true;
			for (int x = 0; x < WIDTH; ++x)
			{
				if (m_grid[y][x] == 0)
				{
					full = false;
					break;
				}
			}

			if (full)
			{
				for (int yy = y; yy < HEIGHT - 1; ++yy) //сдвиг на строку вниз
				{
					for (int x = 0; x < WIDTH; ++x)
					{
						m_grid[yy][x] = m_grid[yy + 1][x];
					}
				}

				for (int x = 0; x < WIDTH; ++x)
				{
					m_grid[HEIGHT - 1][x] = 0;
				}

				++linesCleared;
				--y;
			}
		}

		return linesCleared;
	}

	void Draw() const
	{
		//сетка стакана
		for (int y = 0; y < HEIGHT; ++y)
		{
			for (int x = 0; x < WIDTH; ++x)
			{
				DrawCell(x, y, m_grid[y][x]);
			}
		}
		//рамка стакана
		glColor3f(0.5f, 0.5f, 0.5f);
		glLineWidth(2.0f);
		glBegin(GL_LINE_LOOP);
		glVertex2f(0, 0);
		glVertex2f(WIDTH, 0);
		glVertex2f(WIDTH, HEIGHT);
		glVertex2f(0, HEIGHT);
		glEnd();
		glLineWidth(1.0f);
	}

private:
	int m_grid[HEIGHT][WIDTH]{};

	void DrawCell(int x, int y, int color) const
	{
		if (color == 0)
		{
			glColor3f(0.85f, 0.85f, 0.85f);
			glBegin(GL_LINE_LOOP);
			glVertex2f(x, y);
			glVertex2f(x + 1, y);
			glVertex2f(x + 1, y + 1);
			glVertex2f(x, y + 1);
			glEnd();
			return;
		}

		glm::vec3 col = GetColorById(color);
		glColor3f(col.r, col.g, col.b);
		glBegin(GL_QUADS);
		glVertex2f(x, y);
		glVertex2f(x + 1, y);
		glVertex2f(x + 1, y + 1);
		glVertex2f(x, y + 1);
		glEnd();		
	}
};


//боковая панель, содержащая информацию по прогрессу
class InfoPanel
{
public:
	InfoPanel() = default;

	void SetStats(int level, int linesLeft, int score)
	{
		m_level = level;
		m_linesLeft = linesLeft;
		m_score = score;
	}

	void SetNextTetromino(const Tetromino* t)
	{
		m_next = t;
	}

	void Draw() const
	{
		const float PANEL_X = Arena::WIDTH + 0.5f;
		const float TOP_Y = Arena::HEIGHT - 1.0f;

		glColor3f(0, 0, 0);

		DrawPanelText(PANEL_X, TOP_Y, "Level: " + std::to_string(m_level));
		DrawPanelText(PANEL_X, TOP_Y - 2.0f, "Lines left: " + std::to_string(m_linesLeft));
		DrawPanelText(PANEL_X, TOP_Y - 4.0f, "Score: " + std::to_string(m_score));
		DrawPanelText(PANEL_X, TOP_Y - 6.0f, "Next:");

		if (m_next)
		{
			DrawNextPreview(*m_next, PANEL_X + 2.0f, TOP_Y - 8.0f);
		}
	}

private:
	int m_level = 1;
	int m_linesLeft = 0;
	int m_score = 0;

	const Tetromino* m_next = nullptr;


	static void DrawPanelText(float x, float y, const std::string& text)
	{
		glRasterPos2f(x, y);
		for (char ch : text)
		{
			glutBitmapCharacter(GLUT_BITMAP_9_BY_15, ch);
		}
	}

	static void DrawNextPreview(const Tetromino& t, float originX, float originY)
	{
		const Shape& shape = t.GetShape();
		for (int sy = 0; sy < 4; ++sy)
		{
			for (int sx = 0; sx < 4; ++sx)
			{
				if (shape[sy][sx])
				{
					float x = originX + sx;
					float y = originY + sy;

					glm::vec3 color = GetColorById(t.GetColor());
					glColor3f(color.r, color.g, color.b);
					glBegin(GL_QUADS);
					glVertex2f(x, y);
					glVertex2f(x + 1, y);
					glVertex2f(x + 1, y + 1);
					glVertex2f(x, y + 1);
					glEnd();
				}
			}
		}
	}
};


//класс, отвечающий за обработку ввода нажатий
class MotionController
{
public:
	MotionController(Arena& arena) : m_arena(&arena) {}

	void OnSpecialKey(int key)
	{
		switch (key)
		{
		case GLUT_KEY_LEFT:
			MoveLeft();
			break;
		case GLUT_KEY_RIGHT:
			MoveRight();
			break;
		case GLUT_KEY_UP:
			Rotate();
			break;
		case GLUT_KEY_DOWN:
			SoftDrop();
			break;
		}
	}

	void OnKey(unsigned char key)
	{
		if (key == 'p' || key == 'P' || key == 0xE7 || key == 0xC7)
		{
			m_paused = !m_paused;
		}
	}

	void MoveLeft()
	{
		if (!m_current)
		{
			return;
		}

		if (m_arena->CanPlace(*m_current, { -1, 0 }))
		{
			m_current->SetPosition(m_current->GetPosition() + glm::ivec2(-1, 0));
		}
	}

	void MoveRight()
	{
		if (!m_current)
		{
			return;
		}

		if (m_arena->CanPlace(*m_current, { 1, 0 }))
		{
			m_current->SetPosition(m_current->GetPosition() + glm::ivec2(1, 0));
		}
	}

	void Rotate()
	{
		if (!m_current) return;
		auto saved = m_current->GetShape();
		m_current->RotateCW();

		if (!m_arena->CanPlace(*m_current))
		{
			m_current->RotateCW();
			m_current->RotateCW();
			m_current->RotateCW();
		}
	}

	void SoftDrop()
	{
		m_softDropThisFrame = true;
	}

	void SetCurrent(ITetromino* t) 
	{ 
		m_current = t; 
	}

	bool IsPaused() const 
	{ 
		return m_paused; 
	}

private:
	Arena* m_arena;
	Tetromino* m_current = nullptr;
	bool m_paused = false;
	bool m_softDropThisFrame = false;
};


//класс приложения "Тетрис"
class TetrisGame : public GLApplication
{
public:
	TetrisGame() : GLApplication("Tetris", 450, 600, false, false) {}

protected:
	void OnInit() override
	{
		glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
		glDisable(GL_DEPTH_TEST);

		m_rng.seed(std::random_device{}());
		m_next = CreateRandomTetromino(m_rng);
		SpawnTetromino();
	}
	
	void OnReshape(int width, int height) override
	{
		glViewport(0, 0, width, height);

		float sceneW = Arena::WIDTH + 6.0f;
		float sceneH = static_cast<float>(Arena::HEIGHT);
		float windowAspect = static_cast<float>(width) / static_cast<float>(height);
		float sceneAspect = sceneW / sceneH;

		float left{}, right{}, bottom{}, top{};

		if (windowAspect > sceneAspect)
		{
			//окно шире сцены - расширить по x, оставить y
			float newW = sceneH * windowAspect;
			float offset = (newW - sceneW) / 2.0f;
			left = -offset;
			right = sceneW + offset;
			bottom = 0;
			top = sceneH;
		}
		else
		{
			//окно выше сцены - расширить по y
			float newH = sceneW / windowAspect;
			float offset = (newH - sceneH) / 2.0f;
			left = 0;
			right = sceneW;
			bottom = -offset;
			top = sceneH + offset;
		}

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(left, right, bottom, top);
		glMatrixMode(GL_MODELVIEW);
	}

	void OnDisplay() override
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		m_arena.Draw();
		if (m_current)
		{
			DrawTetromino(*m_current);
		}

		m_panel.SetStats(m_level, m_linesLeft, m_score);
		m_panel.SetNextTetromino(m_next.get());
		m_panel.Draw();

		if (m_paused)
		{
			DrawPanelText(10.5f, 9, "PAUSE");
		}
		if (m_gameOver)
		{
			DrawPanelText(10.5f, 9, "GAME OVER");
		}
	}

	void OnKeyboard(unsigned char key, int x, int y) override
	{
		if (m_gameOver)
		{
			if (key == 'r' || key == 'R')
			{
				Restart();
			}
			if (key == 27) exit(0);
			return;
		}

		if (key == 'p' || key == 'P' || key == 0xE7 || key == 0xC7)
		{
			m_paused = !m_paused;
			return;
		}
	}

	void OnSpecial(int key, int x, int y) override
	{
		if (m_paused || m_gameOver || !m_current)
		{
			return;
		}

		switch (key)
		{
		case GLUT_KEY_LEFT:
			if (m_arena.CanPlace(*m_current, { -1, 0 }))
			{
				m_current->SetPosition(m_current->GetPosition() + glm::ivec2(-1, 0));
			}
			break;
		case GLUT_KEY_RIGHT:
			if (m_arena.CanPlace(*m_current, { 1, 0 }))
			{
				m_current->SetPosition(m_current->GetPosition() + glm::ivec2(1, 0));
			}
			break;
		case GLUT_KEY_UP:
			RotateCurrent();
			break;
		case GLUT_KEY_DOWN:
			m_softDropRequested = true;
			break;
		}
		PostRedisplay();
	}

	void OnIdle() override
	{
		static int lastTime = 0;
		int now = glutGet(GLUT_ELAPSED_TIME);
		float dt = (now - lastTime) / 1000.0f;
		lastTime = now;
		if (dt > 0.2f)
		{
			dt = 0.2f;
		}

		if (!m_paused && !m_gameOver)
		{
			m_fallTimer += dt;
			float fallInterval = 1.0f / m_fallSpeed;
			if (m_softDropRequested)
			{
				fallInterval = 0.05f; //быстрое падение по стрелке внищ
				m_softDropRequested = false;
			}

			if (m_fallTimer >= fallInterval)
			{
				m_fallTimer = 0.0f;
				StepDown();
			}
		}

		PostRedisplay();
	}

private:
	Arena m_arena;
	InfoPanel m_panel;
	std::unique_ptr<Tetromino> m_current;
	std::unique_ptr<Tetromino> m_next;
	std::mt19937 m_rng;

	int m_level = 1;
	int m_linesLeft = 5;
	int m_score = 0;

	float m_fallTimer = 0.0f;
	float m_fallSpeed = 1.0f;   // клеток в секунду

	bool m_paused = false;
	bool m_gameOver = false;
	bool m_softDropRequested = false;

	
	void SpawnTetromino()
	{
		m_current = std::move(m_next);
		m_next = CreateRandomTetromino(m_rng);
		m_current->SetPosition({ m_arena.WIDTH / 2 - 2, m_arena.HEIGHT - 2 });
		if (!m_arena.CanPlace(*m_current))
		{
			m_gameOver = true;
		}
	}

	static int ScoreFor(int lines) //кол-во очков по заданию
	{
		switch (lines)
		{
		case 1: return 10;
		case 2: return 30;
		case 3: return 70;
		case 4: return 150;
		}
		return 0;
	}

	void LevelUp()
	{
		++m_level;
		m_fallSpeed += 1.0f;        //+1 скорость (бета)
		m_linesLeft = 5 + m_level;  //+1 к заданию
		m_arena.Clear();
	}

	void RotateCurrent()
	{
		m_current->RotateCW();
		if (!m_arena.CanPlace(*m_current))
		{
			m_current->RotateCW();
			m_current->RotateCW();
			m_current->RotateCW();
		}
	}

	void Restart()
	{
		m_arena.Clear();
		m_level = 1;
		m_score = 0;
		m_linesLeft = 5;
		m_fallSpeed = 1.0f;
		m_gameOver = false;
		m_paused = false;
		SpawnTetromino();
	}

	void StepDown()
	{
		if (m_arena.CanPlace(*m_current, { 0, -1 }))
		{
			m_current->SetPosition(m_current->GetPosition() + glm::ivec2(0, -1));
		}
		else
		{
			m_arena.Place(*m_current);
			int cleared = m_arena.ClearFullLines();
			if (cleared > 0)
			{
				m_score += ScoreFor(cleared);
				m_linesLeft -= cleared;
				if (m_linesLeft <= 0)
				{
					LevelUp();
				}
			}

			SpawnTetromino();
		}
	}

	void DrawTetromino(const Tetromino& t)
	{
		const Shape& shape = t.GetShape();
		glm::ivec2 pos = t.GetPosition();
		glm::vec3 c = GetColorById(t.GetColor());

		for (int sy = 0; sy < 4; ++sy)
		{
			for (int sx = 0; sx < 4; ++sx)
			{
				if (shape[sy][sx])
				{
					float x = pos.x + sx;
					float y = pos.y + sy;
					glColor3f(c.r, c.g, c.b);
					glBegin(GL_QUADS);
					glVertex2f(x, y);
					glVertex2f(x + 1, y);
					glVertex2f(x + 1, y + 1);
					glVertex2f(x, y + 1);
					glEnd();
				}
			}
		}
	}

	static void DrawPanelText(float x, float y, const std::string& text)
	{
		glRasterPos2f(x, y);
		for (char ch : text)
		{
			glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, ch);
		}
	}
};


//разделить ответственность логики и отрисовки (ArenaView рисует)
void RunLW3Task3()
{
	TetrisGame tetris;
	tetris.MainLoop();
}
