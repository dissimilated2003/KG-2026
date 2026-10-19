#pragma once


#include "Utils.h"

#include <random>
#include <string>


//Задание 2
//Вариант 1 – Memory Trainer 3D – 250 баллов

//Разработайте игру Memory Trainer 3D. Суть игры заключается в следующем:

//Пользователю показывается игровое поле M* N табличек(всего табличек четное число), 
// повернутых к нему оборотной стороной(текстура оборотной стороны одинакова для всех табличек).
// При щелчке по плитке она плавно переворачивается(вот поэтому - то в названии игры и используется 3D) 
// лицевой стороной, на которой нарисована некоторая картинка.Если до этого была открыта плитка 
// с тем же рисунком, то обе плитки удаляются с поля.В противном случае обе плитки плавно 
// поворачиваются обратно рубашкой вверх.

//Повторный щелчок по ячейке, повернутой к нам картинкой, вызывает ее обратный поворот рубашкой вверх.

//Цель игры – убрать все плитки с игрового поля.


//класс "Плитка"
class Tile
{
public:
	Tile(const glm::ivec2& cell, float width)
		: m_cell(cell)
		, m_size(width)
		, m_thickness(width / 10.0f)
	{}

	void FlipToFace()
	{
		if (!m_removed)
		{
			m_targetAngle = 180.0f;
		}
	}

	void FlipToBack()
	{
		if (!m_removed)
		{
			m_targetAngle = 0.0f;
		}
	}

	void Remove()
	{
		m_removed = true;
	}

	void Update(float dt)
	{
		if (m_removed)
		{
			return;
		}

		const float ROTATION_SPEED = 360.0f; //град/сек
		if (m_angle < m_targetAngle)
		{
			m_angle += ROTATION_SPEED * dt;
			if (m_angle > m_targetAngle)
			{
				m_angle = m_targetAngle;
			}
		}
		else if (m_angle > m_targetAngle)
		{
			m_angle -= ROTATION_SPEED * dt;
			if (m_angle < m_targetAngle)
			{
				m_angle = m_targetAngle;
			}
		}
	}

	bool IsAnimating() const
	{
		return std::abs(m_angle - m_targetAngle) > 0.01f;
	}

	bool IsFaceUp() const
	{
		return m_targetAngle > 90.0f;
	}

	bool IsRemoved() const
	{
		return m_removed;
	}

	const glm::ivec2 GetCell() const
	{
		return m_cell;
	}

	int GetPairID() const
	{
		return m_pairID;
	}

	void SetPairID(int pairID)
	{
		m_pairID = pairID;
	}

	void Draw(GLuint backTex, GLuint faceTex) const
	{
		if (m_removed)
		{
			return;
		}

		const float HALF_CELL = m_size * 0.5f;
		const float HALF_THICKNESS = m_thickness * 0.5f;

		float cellx = m_cell.x * (m_size + CELL_GAP) - s_offset.x;
		float celly = 0.0f;
		float cellz = m_cell.y * (m_size + CELL_GAP) - s_offset.y;

		glPushMatrix();
		glTranslated(cellx, celly, cellz);

		glRotated(m_angle, 0.0, 0.0, 1.0);

		//glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);

		glColor3f(0.4f, 0.4f, 0.45f);
		glBegin(GL_QUADS);
		//верхняя грань
		glNormal3f(0.0f, 1.0f, 0.0f);
		glVertex3f(-HALF_CELL, HALF_THICKNESS, -HALF_CELL);
		glVertex3f(HALF_CELL, HALF_THICKNESS, -HALF_CELL);
		glVertex3f(HALF_CELL, HALF_THICKNESS, HALF_CELL);
		glVertex3f(-HALF_CELL, HALF_THICKNESS, HALF_CELL);
		//нижняя
		glNormal3f(0.0f, -1.0f, 0.0f);
		glVertex3f(-HALF_CELL, -HALF_THICKNESS, HALF_CELL);
		glVertex3f(HALF_CELL, -HALF_THICKNESS, HALF_CELL);
		glVertex3f(HALF_CELL, -HALF_THICKNESS, -HALF_CELL);
		glVertex3f(-HALF_CELL, -HALF_THICKNESS, -HALF_CELL);
		//дальняя боковая
		glNormal3f(0.0f, 0.0f, 1.0f);
		glVertex3f(-HALF_CELL, -HALF_THICKNESS, HALF_CELL);
		glVertex3f(HALF_CELL, -HALF_THICKNESS, HALF_CELL);
		glVertex3f(HALF_CELL, HALF_THICKNESS, HALF_CELL);
		glVertex3f(-HALF_CELL, HALF_THICKNESS, HALF_CELL);
		//ближняя боковая
		glNormal3f(0.0f, 0.0f, -1.0f);
		glVertex3f(HALF_CELL, -HALF_THICKNESS, -HALF_CELL);
		glVertex3f(-HALF_CELL, -HALF_THICKNESS, -HALF_CELL);
		glVertex3f(-HALF_CELL, HALF_THICKNESS, -HALF_CELL);
		glVertex3f(HALF_CELL, HALF_THICKNESS, -HALF_CELL);
		//правая боковая
		glNormal3f(1.0f, 0.0f, 0.0f);
		glVertex3f(HALF_CELL, -HALF_THICKNESS, HALF_CELL);
		glVertex3f(HALF_CELL, -HALF_THICKNESS, -HALF_CELL);
		glVertex3f(HALF_CELL, HALF_THICKNESS, -HALF_CELL);
		glVertex3f(HALF_CELL, HALF_THICKNESS, HALF_CELL);
		//левая боковая
		glNormal3f(-1.0f, 0.0f, 0.0f);
		glVertex3f(-HALF_CELL, -HALF_THICKNESS, -HALF_CELL);
		glVertex3f(-HALF_CELL, -HALF_THICKNESS, HALF_CELL);
		glVertex3f(-HALF_CELL, HALF_THICKNESS, HALF_CELL);
		glVertex3f(-HALF_CELL, HALF_THICKNESS, -HALF_CELL);
		glEnd();

		glEnable(GL_TEXTURE_2D);
		//glEnable(GL_LIGHTING);

		//задние стороны (рубашки), изначально смотрят вверх
		glColor3f(1.0f, 1.0f, 1.0f);
		Texture2DHandle(backTex).Bind();

		glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-HALF_CELL, HALF_THICKNESS + 0.001f, HALF_CELL);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(HALF_CELL, HALF_THICKNESS + 0.001f, HALF_CELL);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(HALF_CELL, HALF_THICKNESS + 0.001f, -HALF_CELL);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-HALF_CELL, HALF_THICKNESS + 0.001f, -HALF_CELL);
		glEnd();

		//лицевые текстуры, смотрят вниз изначально
		glColor3f(1.0f, 1.0f, 1.0f);
		Texture2DHandle(faceTex).Bind();

		glBegin(GL_QUADS);
		glNormal3f(0.0f, -1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-HALF_CELL, -HALF_THICKNESS - 0.001f, -HALF_CELL);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(HALF_CELL, -HALF_THICKNESS - 0.001f, -HALF_CELL);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(HALF_CELL, -HALF_THICKNESS - 0.001f, HALF_CELL);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-HALF_CELL, -HALF_THICKNESS - 0.001f, HALF_CELL);
		glEnd();

		glPopMatrix();
	}

	static void SetOffset(glm::vec2 offset)
	{
		s_offset = offset;
	}

	bool ContainsPoint(double wx, double wz) const
	{
		if (m_removed)
		{
			return false;
		}

		float half = m_size * 0.5f + CELL_GAP * 0.5f;

		float cx = m_cell.x * (m_size + CELL_GAP) - s_offset.x;
		float cz = m_cell.y * (m_size + CELL_GAP) - s_offset.y;

		return std::abs(wx - cx) <= half && std::abs(wz - cz) <= half;
	}

	static constexpr float CELL_GAP = 0.1f;

private:

	glm::ivec2 m_cell{};
	float m_size;
	float m_thickness;

	float m_angle = 0.0f;
	float m_targetAngle = 0.0f;
	int m_pairID = -1;

	bool m_removed = false;

	static inline glm::vec2 s_offset{ 0.0f };
};


//сетка MxN плиток
class TilesTable
{
public:
	static constexpr int COLS = 5;
	static constexpr int ROWS = 4;
	static constexpr int PAIRS = (COLS * ROWS) / 2;

	TilesTable(float tileWidth)
		: m_tileWidth(tileWidth)
	{
		float totalW = COLS * tileWidth + (COLS - 1) * Tile::CELL_GAP;
		float totalH = ROWS * tileWidth + (ROWS - 1) * Tile::CELL_GAP;
		glm::vec2 offset = { totalW * 0.5f - tileWidth * 0.5f, totalH * 0.5f - tileWidth * 0.5f };
		Tile::SetOffset(offset);

		for (int y = 0; y < ROWS; ++y)
		{
			for (int x = 0; x < COLS; ++x)
			{
				m_tiles.emplace_back(glm::ivec2{ x, y }, tileWidth);
			}
		}

		AssignPairs();
	}
	
	void LoadTextures(const TextureLoader& loader)
	{
		m_backTexture = loader.LoadTexture2D(L"assets/LW5/MemoryTrainer/back.png");
		//если менять размер например с 5x4 на 6x5, то надо еще загрузить 
		// чтобы хватидо картинок до 6x5/2 = 15шт и еще тут прописать путь к ним
		//цикл с std::wstring почему-то бросает assertion failed
		m_faceTextures[0] = loader.LoadTexture2D(L"assets/LW5/MemoryTrainer/pic_0.png");
		m_faceTextures[1] = loader.LoadTexture2D(L"assets/LW5/MemoryTrainer/pic_1.png");
		m_faceTextures[2] = loader.LoadTexture2D(L"assets/LW5/MemoryTrainer/pic_2.png");
		m_faceTextures[3] = loader.LoadTexture2D(L"assets/LW5/MemoryTrainer/pic_3.png");
		m_faceTextures[4] = loader.LoadTexture2D(L"assets/LW5/MemoryTrainer/pic_4.png");
		m_faceTextures[5] = loader.LoadTexture2D(L"assets/LW5/MemoryTrainer/pic_5.png");
		m_faceTextures[6] = loader.LoadTexture2D(L"assets/LW5/MemoryTrainer/pic_6.png");
		m_faceTextures[7] = loader.LoadTexture2D(L"assets/LW5/MemoryTrainer/pic_7.png");
		m_faceTextures[8] = loader.LoadTexture2D(L"assets/LW5/MemoryTrainer/pic_8.png");
		m_faceTextures[9] = loader.LoadTexture2D(L"assets/LW5/MemoryTrainer/pic_9.png");
	}

	void Restart()
	{
		for (auto& t : m_tiles)
		{
			t = Tile(t.GetCell(), m_tileWidth);
		}
		AssignPairs();
		m_selectedIndex = -1;
		m_pendingPairCheck = false;
		m_waitTimer = 0.0f;
	}

	bool AnyAnimating() const
	{
		for (const auto& t : m_tiles)
		{
			if (t.IsAnimating())
			{
				return true;
			}
		}

		return false;
	}

	void Update(float dt)
	{
		for (auto& t : m_tiles)
		{
			t.Update(dt);
		}

		if (m_pendingPairCheck)
		{
			m_waitTimer -= dt;
		}
	}

	int FindTileByPoint(double wx, double wz) const
	{
		for (size_t i = 0; i < m_tiles.size(); ++i)
		{
			if (m_tiles[i].ContainsPoint(wx, wz))
			{
				return static_cast<int>(i);
			}
		}
			
		return -1;
	}

	void OnTileClick(int index)
	{
		if (index < 0)
		{
			return;
		}

		Tile& t = m_tiles[index];
		if (t.IsRemoved() || t.IsFaceUp())
		{
			return;
		}

		//если сейчас открыта одна плитка и это не она сама
		if (m_selectedIndex == -1)
		{
			m_selectedIndex = index;
			t.FlipToFace();
		}
		else
		{
			//повторный клик по той же
			if (m_selectedIndex == index)
			{
				return;
			}
			//ждет закрытия несовпавшей пары
			if (m_pendingPairCheck)
			{
				return; 
			}
			//открыть и сравнить вторую плитку
			t.FlipToFace();
			m_pendingIndex = index;

			m_pendingPairCheck = true;
			m_waitTimer = 0.5f;
		}
	}

	void ResolvePairIfNeeded()
	{
		if (!m_pendingPairCheck)
		{
			return;
		}
		if (AnyAnimating())
		{
			return;
		}
		if (m_waitTimer > 0.0f)
		{
			return;
		}

		if (m_tiles[m_selectedIndex].GetPairID() == m_tiles[m_pendingIndex].GetPairID())
		{
			//нашли пару - выпиливаем
			m_tiles[m_selectedIndex].Remove();
			m_tiles[m_pendingIndex].Remove();
			m_selectedIndex = -1;
			m_pendingIndex = -1;
			m_pendingPairCheck = false;
		}
		else
		{
			//не пара - закрываем
			m_tiles[m_selectedIndex].FlipToBack();
			m_tiles[m_pendingIndex].FlipToBack();
			m_selectedIndex = -1;
			m_pendingIndex = -1;
			m_pendingPairCheck = false;
		}
	}

	bool IsSolved() const
	{
		for (const auto& t : m_tiles)
		{
			if (!t.IsRemoved())
			{
				return false;
			}
		}

		return true;
	}

	void Draw() const
	{
		for (const auto& t : m_tiles)
		{
			int faceIdx = (t.GetPairID() >= 0) ? t.GetPairID() : 0;

			t.Draw(m_backTexture, m_faceTextures[faceIdx]);
		}
	}

private:
	float m_tileWidth;
	std::vector<Tile> m_tiles;
	GLuint m_backTexture = 0;
	GLuint m_faceTextures[PAIRS]{};

	int m_selectedIndex = -1;
	int m_pendingIndex = -1;
	bool m_pendingPairCheck = false;
	float m_waitTimer = 0.0f;


	void AssignPairs()
	{
		//сгенерить массив пар 0,0,1,1,2,2..
		std::vector<int> ids;
		for (int i = 0; i < PAIRS; ++i)
		{
			ids.push_back(i);
			ids.push_back(i);
		}

		std::mt19937 rng(static_cast<unsigned>(std::time(nullptr))); //смешать чтобы в разных местах были
		std::shuffle(ids.begin(), ids.end(), rng);

		for (size_t i = 0; i < m_tiles.size(); ++i)
		{
			m_tiles[i].SetPairID(ids[i]);
		}
	}
};


//класс прилолжения "Тренажер памяти"
class MemoryTrainerApp : public GLApplication
{
public:
	MemoryTrainerApp() 
		: GLApplication("Memory Trainer 3D", 800, 600, true, false)
		, m_table(1.0f)   //плитки 1x1 юнит
	{}

protected:
	void OnInit() override
	{
		glClearColor(0.25f, 0.3f, 0.4f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_NORMALIZE);

		//свет слева сверху
		GLfloat lightAmbient[] = { 0.3f, 0.3f, 0.35f, 1.0f };
		GLfloat lightDiffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };
		GLfloat lightSpecular[] = { 1.5f, 0.5f, 0.5f, 1.0f };
		GLfloat lightPos[] = { -5.0f, 10.0f, 5.0f, 1.0f };

		glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

		TextureLoader loader;
		loader.SetWrapMode(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		m_table.LoadTextures(loader);
	}

	void OnDisplay() override
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		//камера смотрит с небольшим наклоном чтобы видеть объем
		gluLookAt(
			0.0, 5.0, 4.0,
			0.0, -0.4, 0.0,
			0.0, 1.0, 0.0
		);

		m_table.Draw();
	}

	void OnReshape(int width, int height) override
	{
		glViewport(0, 0, width, height);
		double aspect = static_cast<double>(width) / static_cast<double>(height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45.0, aspect, 0.1, 100.0);
		glMatrixMode(GL_MODELVIEW);

		m_viewportWidth = width;
		m_viewportHeight = height;
	}

	void OnKeyboard(unsigned char key, int x, int y) override
	{
		if (key == 'r' || key == 'R' || key == 0xCB || key == 0xEB)
		{
			m_table.Restart();
		}
		if (key == 27)//esc
		{
			exit(0);
		}
	}

	void OnMouse(int button, int state, int x, int y) override
	{
		if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
		{
			return;
		}

		GLdouble modelview[16], projection[16];
		GLint viewport[4];
		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
		glGetIntegerv(GL_VIEWPORT, viewport);

		double winX = x;
		double winY = viewport[3] - y;

		//две точки луча - ближняя z=0 и дальняя z=1
		GLdouble nearX{}, nearY{}, nearZ{};
		GLdouble farX{}, farY{}, farZ{};
		//gluUnProject - перевод экранных координат в мировые
		gluUnProject(
			winX, winY, 0.0, 
			modelview, projection, viewport,
			&nearX, &nearY, &nearZ
		);
		gluUnProject(
			winX, winY, 1.0, 
			modelview, projection, viewport,
			&farX, &farY, &farZ
		);

		//луч r: near + t * (far - near) = 0
		double dirX = farX - nearX;
		double dirY = farY - nearY;
		double dirZ = farZ - nearZ;

		if (std::abs(dirY) < 1e-6)
		{
			return; //если луч "параллелен полу"
		}

		double t = -nearY / dirY;
		if (t < 0.0)
		{
			return; //пересечение за камерой
		}

		double worldX = nearX + t * dirX;
		double worldZ = nearZ + t * dirZ;

		int tileIndex = m_table.FindTileByPoint(worldX, worldZ);
		if (tileIndex >= 0)
		{
			m_table.OnTileClick(tileIndex);
			PostRedisplay();
		}
	}

	void OnIdle() override
	{
		static int lastTime = 0;
		int now = glutGet(GLUT_ELAPSED_TIME);
		float dt = (now - lastTime) / 1000.0f;
		lastTime = now;
		if (dt > 0.1f)
		{
			dt = 0.1f;
		}

		m_table.Update(dt);
		m_table.ResolvePairIfNeeded();

		if (m_table.IsSolved())
		{
			m_table.Restart();
		}

		PostRedisplay();
	}

private:
	TilesTable m_table;
	int m_viewportWidth = 800;
	int m_viewportHeight = 600;
};



void RunLW5Task2()
{
	GdiplusInitializer init;
	MemoryTrainerApp app;
	glewInit();
	try
	{
		app.MainLoop();
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
	}
}
