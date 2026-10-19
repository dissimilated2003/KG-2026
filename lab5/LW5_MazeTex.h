#pragma once


#include "Utils.h"


// задание 1 вариант 6
// Программа визуализирует трехмерный лабиринт. На стены лабиринта нанесена текстура. 
// Реализовать в программе не менее 6 типов стен, различающихся текстурой. 
// Пользователь может передвигаться по лабиринту при помощи мыши и/или клавиатуры. 
// Проходить сквозь стены нельзя.

//управление:
// 
//      WASD:
//      W - движение вперед
//      A - поворот камеры влево
//      S - движение назад
//      D - поворот камеры вправо



//класс лабиринта, хранит вектор векторов идшников текстур
class TexturedMaze
{
public:
	TexturedMaze()
	{
		m_map =
		{
			{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // 1 - tex1 деревянная стена
			{3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2}, // 2 - tex2 каменная слоёная стена
			{3, 0, 4, 4, 4, 4, 4, 0, 3, 3, 3, 3, 3, 0, 2}, // 3 - tex3 кирпичная стена
			{3, 0, 4, 0, 0, 0, 4, 0, 0, 0, 0, 0, 3, 0, 2}, // 4 - tex4 мрамор
			{3, 0, 4, 4, 4, 0, 4, 0, 3, 3, 0, 3, 3, 0, 2}, // 5 - tex5 плитка
			{3, 0, 0, 0, 6, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2}, // 6 - tex6 типа потолок
			{3, 0, 6, 0, 6, 6, 6, 0, 2, 0, 2, 2, 2, 0, 2},
			{3, 0, 6, 0, 6, 0, 6, 0, 2, 0, 0, 0, 2, 0, 2},
			{3, 0, 6, 6, 6, 0, 6, 0, 2, 2, 2, 0, 2, 2, 2},
			{3, 0, 6, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 2},
			{5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5}
		};

		m_mapWidth = m_map[0].size();
		m_mapHeight = m_map.size();

		for (int i = 0; i < 6; ++i)
		{
			m_wallTextures[i] = 0;
		}

		m_floorTexture = 0;
	}

	void LoadTextures(const TextureLoader& loader)
	{
		m_wallTextures[0] = loader.LoadTexture2D(L"assets/LW5/tex1_wooden.jpg");
		m_wallTextures[1] = loader.LoadTexture2D(L"assets/LW5/tex2_stone.png");
		m_wallTextures[2] = loader.LoadTexture2D(L"assets/LW5/tex3_brick.png");
		m_wallTextures[3] = loader.LoadTexture2D(L"assets/LW5/tex4_marble.png");
		m_wallTextures[4] = loader.LoadTexture2D(L"assets/LW5/tex5_tile.png");
		m_wallTextures[5] = loader.LoadTexture2D(L"assets/LW5/tex6_material.jpg");

		m_floorTexture = loader.LoadTexture2D(L"assets/LW5/tex0_floor.jpg");
	}

	void Draw() const
	{
		DrawFloor();

		for (int z = 0; z < m_mapHeight; ++z)
		{
			for (int x = 0; x < m_mapWidth; ++x)
			{
				int cell = m_map[z][x];

				if (cell >= 1 && cell <= 6)
				{
					int texIndex = cell - 1;

					glPushMatrix();
					glTranslated(x + 0.5, 0.0, z + 0.5);
					glColor3f(1.0f, 1.0f, 1.0f);
					Texture2DHandle(m_wallTextures[texIndex]).Bind();
					DrawCube();
					glPopMatrix();
				}
			}
		}
	}

	bool IsWallAt(double x, double z, double radius) const
	{
		if (IsWall(x - radius, z - radius) ||
			IsWall(x + radius, z - radius) ||
			IsWall(x - radius, z + radius) ||
			IsWall(x + radius, z + radius))
		{
			return true;
		}

		return false;
	}

	bool IsWall(double x, double z) const
	{
		if (x < 0 || x >= m_mapWidth || z < 0 || z >= m_mapHeight)
		{
			return true;
		}

		return m_map[z][x] != 0;
	}

private:
	void DrawCube() const
	{
		glBegin(GL_QUADS);

		glNormal3f(0.0f, 0.0f, 1.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.0f, 0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.0f, 0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 1.0f, 0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 1.0f, 0.5f);

		glNormal3f(0.0f, 0.0f, -1.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, 0.0f, -0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 1.0f, -0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 1.0f, -0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, 0.0f, -0.5f);

		glNormal3f(0.0f, 1.0f, 0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 1.0f, -0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 1.0f, 0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 1.0f, 0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 1.0f, -0.5f);

		glNormal3f(0.0f, -1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.0f, -0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.0f, -0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, 0.0f, 0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, 0.0f, 0.5f);

		glNormal3f(1.0f, 0.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.0f, -0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 1.0f, -0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 1.0f, 0.5f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, 0.0f, 0.5f);

		glNormal3f(-1.0f, 0.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.0f, -0.5f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, 0.0f, 0.5f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 1.0f, 0.5f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 1.0f, -0.5f);
		glEnd();
	}

	void DrawFloor() const
	{
		glColor3f(1.0f, 1.0f, 1.0f);
		Texture2DHandle(m_floorTexture).Bind();
		glBegin(GL_QUADS);
		glNormal3f(0.0f, 1.0f, 0.0f);

		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, m_mapHeight);

		glTexCoord2f(m_mapWidth, 0.0f);
		glVertex3f(m_mapWidth, 0.0f, m_mapHeight);

		glTexCoord2f(m_mapWidth, m_mapHeight);
		glVertex3f(m_mapWidth, 0.0f, 0.0f);

		glTexCoord2f(0.0f, m_mapHeight);
		glVertex3f(0.0f, 0.0f, 0.0f);

		glEnd();
	}

	std::vector<std::vector<int>> m_map;
	int m_mapWidth;
	int m_mapHeight;

	GLuint m_wallTextures[6];
	GLuint m_floorTexture;
};


//перемещающийся по лабиринту объект
class MotionController
{
public:
	MotionController(float startX, float startZ, float startAngle, const TexturedMaze& maze)
		: m_posX(startX)
		, m_posZ(startZ)
		, m_angle(startAngle)
		, m_maze(&maze)
	{}

	void OnKeyDown(unsigned char key)
	{
		switch (key)
		{
		case 'w': case 'W': m_Wkey = true; break;
		case 'a': case 'A': m_Akey = true; break;
		case 's': case 'S': m_Skey = true; break;
		case 'd': case 'D': m_Dkey = true; break;
		}

		switch (key)
		{
		case 0xF6: m_Wkey = true; break;   // ц
		case 0xF4: m_Akey = true; break;   // ф
		case 0xFB: m_Skey = true; break;   // ы
		case 0xE2: m_Dkey = true; break;   // в

		case 0xD6: m_Wkey = true; break;   // Ц
		case 0xD4: m_Akey = true; break;   // Ф
		case 0xDB: m_Skey = true; break;   // Ы
		case 0xC2: m_Dkey = true; break;   // В
		}
	}


	void OnKeyUp(unsigned char key)
	{
		switch (key)
		{
		case 'w': case 'W': m_Wkey = false; break;
		case 'a': case 'A': m_Akey = false; break;
		case 's': case 'S': m_Skey = false; break;
		case 'd': case 'D': m_Dkey = false; break;
		}

		switch (key)
		{
		case 0xF6: m_Wkey = false; break;
		case 0xF4: m_Akey = false; break;
		case 0xFB: m_Skey = false; break;
		case 0xE2: m_Dkey = false; break;

		case 0xD6: m_Wkey = false; break;
		case 0xD4: m_Akey = false; break;
		case 0xDB: m_Skey = false; break;
		case 0xC2: m_Dkey = false; break;
		}
	}


	void Update(float dt)
	{
		if (m_Akey)
		{
			m_angle += ROTATION_SPEED * dt;
		}
		if (m_Dkey)
		{
			m_angle -= ROTATION_SPEED * dt;
		}

		if (m_angle >= 360.0f)
		{
			m_angle -= 360.0f;
		}
		if (m_angle < 0.0f)
		{
			m_angle += 360.0f;
		}

		float rad = glm::radians(m_angle);
		float dirX = -std::sin(rad);
		float dirZ = -std::cos(rad);

		float dx = 0.0f;
		float dz = 0.0f;

		if (m_Wkey)
		{
			dx += dirX * MOVE_SPEED * dt;
			dz += dirZ * MOVE_SPEED * dt;
		}
		if (m_Skey)
		{
			dx -= dirX * MOVE_SPEED * dt;
			dz -= dirZ * MOVE_SPEED * dt;
		}

		//обработка коллизий
		float newX = m_posX + dx;
		if (!m_maze->IsWallAt(newX, m_posZ, COLLISION_RADIUS))
		{
			m_posX = newX;
		}
		float newZ = m_posZ + dz;
		if (!m_maze->IsWallAt(m_posX, newZ, COLLISION_RADIUS))
		{
			m_posZ = newZ;
		}
	}

	float GetPosX() const { return m_posX; }
	float GetPosZ() const { return m_posZ; }
	float GetAngle() const { return m_angle; }

private:
	static constexpr float MOVE_SPEED = 2.5f;
	static constexpr float ROTATION_SPEED = 90.0f;
	static constexpr float COLLISION_RADIUS = 0.2f;

	bool m_Wkey{};
	bool m_Akey{};
	bool m_Skey{};
	bool m_Dkey{};

	float m_posX{};
	float m_posZ{};
	float m_angle{};

	const TexturedMaze* m_maze;
};


class Skybox
{
public:
	Skybox()
	{
		for (int i = 0; i < 6; ++i)
		{
			m_textures[i] = 0;
		}
	}

	//https://opengameart.org/content/clouds-skybox-1
	void Load(const TextureLoader& loader)
	{
		m_textures[0] = loader.LoadTexture2D(L"assets/LW5/clouds1_east.bmp");
		m_textures[1] = loader.LoadTexture2D(L"assets/LW5/clouds1_west.bmp");
		m_textures[2] = loader.LoadTexture2D(L"assets/LW5/clouds1_up.bmp");
		m_textures[3] = loader.LoadTexture2D(L"assets/LW5/clouds1_down.bmp");
		m_textures[4] = loader.LoadTexture2D(L"assets/LW5/clouds1_south.bmp");
		m_textures[5] = loader.LoadTexture2D(L"assets/LW5/clouds1_north.bmp");
	}

	void Draw(float x, float y, float z) const
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_FOG);
		glDepthMask(GL_FALSE);

		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);

		const float S = 50.0f;

		//+x восток
		Texture2DHandle(m_textures[0]).Bind();
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x + S, y - S, z - S);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x + S, y - S, z + S);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x + S, y + S, z + S);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x + S, y + S, z - S);
		glEnd();

		//-x запад
		Texture2DHandle(m_textures[1]).Bind();
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x - S, y - S, z + S);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x - S, y - S, z - S);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x - S, y + S, z - S);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x - S, y + S, z + S);
		glEnd();

		//+y верх
		Texture2DHandle(m_textures[2]).Bind();
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x - S, y + S, z - S);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x - S, y + S, z + S);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x + S, y + S, z + S);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x + S, y + S, z - S);
		glEnd();

		//-y низ
		Texture2DHandle(m_textures[3]).Bind();
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x - S, y - S, z - S);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x + S, y - S, z - S);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x + S, y - S, z + S);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x - S, y - S, z + S);
		glEnd();

		//+z юг
		Texture2DHandle(m_textures[4]).Bind();
		glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x - S, y - S, z + S);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x + S, y - S, z + S);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x + S, y + S, z + S);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x - S, y + S, z + S);
		glEnd();

		//-z север
		Texture2DHandle(m_textures[5]).Bind();
		glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(x + S, y - S, z - S);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(x - S, y - S, z - S);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(x - S, y + S, z - S);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(x + S, y + S, z - S);
		glEnd();

		glDepthMask(GL_TRUE);
		glEnable(GL_FOG);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
	}

private:
	GLuint m_textures[6];
};


//главный класс приложения
class TexMazeApplication : public GLApplication
{
public:
	TexMazeApplication()
		: GLApplication("Textured 3D Maze", 800, 600, true, false)
		, m_motion(
			START_X, 
			START_Z, 
			START_ANGLE, 
			m_maze
		)
	{}

protected:
	void OnInit() override
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_NORMALIZE);

		//туман
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_EXP2);
		glFogf(GL_FOG_DENSITY, 0.08f);
		GLfloat fogColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		glFogfv(GL_FOG_COLOR, fogColor);
		glFogf(GL_FOG_START, 3.0f);
		glFogf(GL_FOG_END, 12.0f);
		glHint(GL_FOG_HINT, GL_NICEST);		

		GLfloat lightAmbient[] = { 0.4f, 0.4f, 0.45f, 1.0f };
		GLfloat lightDiffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };
		GLfloat lightSpecular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

		TextureLoader loader;
		m_maze.LoadTextures(loader);
		m_skybox.Load(loader);
	}

	void OnDisplay() override
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glPushMatrix();
		glRotatef(-m_motion.GetAngle(), 0.0f, 1.0f, 0.0f);
		m_skybox.Draw(0.0f, 0.0f, 0.0f);
		glPopMatrix();

		glLoadIdentity();
		glRotatef(-m_motion.GetAngle(), 0.0f, 1.0f, 0.0f);
		glTranslatef(-m_motion.GetPosX(), -EYE_HEIGHT, -m_motion.GetPosZ());

		GLfloat lightpos[] = { m_motion.GetPosX(), EYE_HEIGHT, m_motion.GetPosZ(), 1.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

		m_maze.Draw();
	}

	void OnReshape(int width, int height) override
	{
		glViewport(0, 0, width, height);
		double aspect = double(width) / double(height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(FOV, aspect, Z_NEAR, Z_FAR);
		glMatrixMode(GL_MODELVIEW);
	}

	void OnKeyboard(unsigned char key, int x, int y) override
	{
		if (key == 27)
		{
			exit(0);   //esc
		}
		m_motion.OnKeyDown(key);
	}

	void OnKeyboardUp(unsigned char key, int x, int y) override
	{
		m_motion.OnKeyUp(key);
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

		m_motion.Update(dt);
		PostRedisplay();
	}

private:
	static constexpr float START_ANGLE = 270.0f;
	static constexpr float EYE_HEIGHT = 0.5f;
	static constexpr float FOV = 75.0f;
	static constexpr float Z_NEAR = 0.05f;
	static constexpr float Z_FAR = 200.0f;
	static constexpr float START_X = 1.5f;
	static constexpr float START_Z = 1.5f;

	TexturedMaze m_maze;
	MotionController m_motion;
	Skybox m_skybox;
};



void RunLW5Task1()
{
	GdiplusInitializer init{};
	TexMazeApplication app{};
	try
	{
		app.MainLoop();
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << "\n";
	}
}
