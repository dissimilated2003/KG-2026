#pragma once

#include "Utils.h"


//задание 3 - Разработайте приложение, визуализирующее одну из следующих трехмерных сцен:
//вариант 2 - прогулка по трехмерному лабиринту

//управление:
// 
//      WASD:
//      W - движение вперед
//      A - поворот камеры влево
//      S - движение назад
//      D - поворот камеры вправо



//лабиринт
class Maze
{
public:
    Maze()
    {
    }

    static constexpr int WIDTH = 16;
    static constexpr int HEIGHT = 16;
    static constexpr float CELL_SIZE = 1.0f;
    static constexpr float WALL_HEIGHT = 1.0f;

    bool IsWall(int cx, int cy) const
    {
        if (cx < 0 || cy < 0 || cx >= WIDTH || cy >= HEIGHT)
        {
            return true;
        }
        return m_grid[cy][cx] != 0;
    }

    bool IsWallAt(float worldX, float worldZ, float radius) const
    {
        int cx0 = static_cast<int>(std::floor((worldX - radius) / CELL_SIZE));
        int cx1 = static_cast<int>(std::floor((worldX + radius) / CELL_SIZE));
        int cy0 = static_cast<int>(std::floor((worldZ - radius) / CELL_SIZE));
        int cy1 = static_cast<int>(std::floor((worldZ + radius) / CELL_SIZE));

        for (int cy = cy0; cy <= cy1; ++cy)
        {
            for (int cx = cx0; cx <= cx1; ++cx)
            {
                if (IsWall(cx, cy))
                {
                    return true;
                }
            }
        }

        return false;
    }

    void Draw() const
    {
        DrawFloorAndCeiling();
        for (int y = 0; y < HEIGHT; ++y)
        {
            for (int x = 0; x < WIDTH; ++x)
            {
                if (m_grid[y][x])
                {
                    DrawWallCube(x, y);
                }
            }
        }
    }

private:
    int m_grid[HEIGHT][WIDTH] = {
            { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 },
            { 1,0,0,0,1,0,0,0,0,0,1,0,0,0,0,1 },
            { 1,1,0,1,1,0,1,1,1,0,1,0,1,1,0,1 },
            { 1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1 },
            { 1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1 },
            { 1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1 },
            { 1,0,1,1,1,1,1,0,1,0,1,1,1,1,0,1 },
            { 1,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1 },
            { 1,0,1,0,1,0,1,1,1,1,1,0,1,1,0,1 },
            { 1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,1 },
            { 1,1,1,0,1,1,1,0,1,1,1,1,1,0,1,1 },
            { 1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1 },
            { 1,0,1,1,1,1,1,0,1,0,1,1,1,0,1,1 },
            { 1,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1 },
            { 1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1 },
            { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 }
    };

    void DrawFloorAndCeiling() const
    {
        float totalW = WIDTH * CELL_SIZE;
        float totalH = HEIGHT * CELL_SIZE;

        glDisable(GL_LIGHTING);
        glColor3f(0.95f, 0.87f, 0.78f);
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(totalW, 0, 0);
        glVertex3f(totalW, 0, totalH);
        glVertex3f(0, 0, totalH);
        glEnd();

        glColor3f(0.60f, 0.85f, 0.95f);
        glBegin(GL_QUADS);
        glNormal3f(0, -1, 0);
        glVertex3f(0, WALL_HEIGHT, 0);
        glVertex3f(0, WALL_HEIGHT, totalH);
        glVertex3f(totalW, WALL_HEIGHT, totalH);
        glVertex3f(totalW, WALL_HEIGHT, 0);
        glEnd();
        glEnable(GL_LIGHTING);
    }

    void DrawWallCube(int cx, int cy) const
    {
        float x0 = cx * CELL_SIZE;
        float x1 = x0 + CELL_SIZE;
        float z0 = cy * CELL_SIZE;
        float z1 = z0 + CELL_SIZE;
        float y0 = 0.0f;
        float y1 = WALL_HEIGHT;

        glColor3f(0.60f, 0.60f, 0.62f);
        glBegin(GL_QUADS);
        //передняя сторона
        glNormal3f(0, 0, -1);
        glVertex3f(x0, y0, z0);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x0, y1, z0);

        //задняя сторона
        glNormal3f(0, 0, 1);
        glVertex3f(x1, y0, z1);
        glVertex3f(x0, y0, z1);
        glVertex3f(x0, y1, z1);
        glVertex3f(x1, y1, z1);

        //левая сторона
        glNormal3f(-1, 0, 0);
        glVertex3f(x0, y0, z1);
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x0, y1, z1);

        //правая сторона
        glNormal3f(1, 0, 0);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y1, z0);

        //верхняя сторона
        glNormal3f(0, 1, 0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x1, y1, z1);
        glVertex3f(x0, y1, z1);

        //нижняя сторона
        glNormal3f(0, -1, 0);
        glVertex3f(x0, y0, z1);
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y0, z0);
        glVertex3f(x0, y0, z0);
        glEnd();
    }
};


//перемещающийся по лабиринту объект
class MotionController
{
public:
	MotionController(float startX, float startZ, float startAngle, const Maze& maze)
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

    const Maze* m_maze;
};



class MazeApplication : public GLApplication
{
public:
    MazeApplication(const char* title, int width, int height)
        : GLApplication(title, width, height)
        , m_motion(START_X, START_Z, START_ANGLE, m_maze)
    {}

protected:
    virtual void OnInit()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        glEnable(GL_LIGHT2);


        glEnable(GL_NORMALIZE);

        GLfloat lightAmbient[] = {0.4f, 0.4f, 0.45f, 1.0f};
        GLfloat lightDiffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };
        GLfloat lightSpecular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
        glLightfv(GL_LIGHT2, GL_SPECULAR, lightSpecular);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    }

    virtual void OnDisplay()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glRotatef(-m_motion.GetAngle(), 0.0f, 1.0f, 0.0f);
        glTranslatef(-m_motion.GetPosX(), -EYE_HEIGHT, -m_motion.GetPosZ());

        GLfloat lightpos[] = { m_motion.GetPosX(), EYE_HEIGHT, m_motion.GetPosZ(), 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

        m_maze.Draw();
    }

    virtual void OnReshape(int width, int height)
    {
        glViewport(0, 0, width, height);
        double aspect = double(width) / double(height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(FOV, aspect, Z_NEAR, Z_FAR);
        glMatrixMode(GL_MODELVIEW);
    }

    virtual void OnKeyboard(unsigned char key, int x, int y)
    {
        m_motion.OnKeyDown(key);
    }

    virtual void OnKeyboardUp(unsigned char key, int x, int y)
    {
        m_motion.OnKeyUp(key);
    }

    virtual void OnIdle()
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
    static constexpr float Z_FAR = 50.0f;

    static constexpr float START_X = 1.5f;
    static constexpr float START_Z = 1.5f;

    Maze m_maze;
    MotionController m_motion;
};


void RunLW4Task3()
{
    MazeApplication app{ "Maze of my brain", 1000, 700 };
    app.MainLoop();
}
