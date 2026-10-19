#include "Utils.h"


//1 задание - вариант 28 - Дельтоидальный гексеконтаэдр
// класс отвечающий за поворот камеры при зажатиях мыши
class RotationController
{
public:
    RotationController(int windowWidth, int windowHeight)
        : m_windowWidth(windowWidth)
        , m_windowHeight(windowHeight)
    {}

    void ResizeWindow(int windowWidth, int windowHeight)
    {
        m_windowWidth = windowWidth;
        m_windowHeight = windowHeight;
    }

    void OnMouse(int button, int state, int x, int y)
    {
        if (button == GLUT_LEFT_BUTTON)
        {
            m_leftButtonPressed = (state == GLUT_DOWN);
            m_mouseX = x;
            m_mouseY = y;
        }
    }

    bool LeftButtonIsPressed() const
    {
        return m_leftButtonPressed;
    }

    void OnMotion(int x, int y)
    {
        if (m_leftButtonPressed)
        {
            int dx = x - m_mouseX;
            int dy = y - m_mouseY;

            GLfloat rotateX = GLfloat(dy) * 180 / m_windowHeight;
            GLfloat rotateY = GLfloat(dx) * 180 / m_windowWidth;

            RotateCamera(rotateX, rotateY);
            m_mouseX = x;
            m_mouseY = y;
        }
    }

private:
    bool m_leftButtonPressed = false;
    int m_mouseX{};
    int m_mouseY{};
    int m_windowWidth{};
    int m_windowHeight{};

    void RotateCamera(GLfloat rotateX, GLfloat rotateY)
    {
        GLfloat modelView[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, &modelView[0]);
        glm::vec3 xAxis{ modelView[0], modelView[4], modelView[8] };
        glm::vec3 yAxis{ modelView[1], modelView[5], modelView[9] };

        glRotatef(rotateX, xAxis.x, xAxis.y, xAxis.z);
        glRotatef(rotateY, yAxis.x, yAxis.y, yAxis.z);

        NormalizeModelViewMatrix();
    }

    void NormalizeModelViewMatrix()
    {
        GLfloat modelView[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, &modelView[0]);

        glm::vec3 xAxis(modelView[0], modelView[4], modelView[8]);
        xAxis = glm::normalize(xAxis);
        glm::vec3 yAxis(modelView[1], modelView[5], modelView[9]);
        yAxis = glm::normalize(yAxis);

        glm::vec3 zAxis = glm::cross(xAxis, yAxis);
        zAxis = glm::normalize(zAxis);

        xAxis = glm::cross(yAxis, zAxis);
        xAxis = glm::normalize(xAxis);
        yAxis = glm::cross(zAxis, xAxis);
        yAxis = glm::normalize(yAxis);

        modelView[0] = xAxis.x; modelView[4] = xAxis.y, modelView[8] = xAxis.z;
        modelView[1] = yAxis.x; modelView[5] = yAxis.y, modelView[9] = yAxis.z;
        modelView[2] = zAxis.x; modelView[6] = zAxis.y, modelView[10] = zAxis.z;

        glLoadMatrixf(modelView);
    }
};


struct Face
{
    std::vector<int> indices;
    glm::vec3 normal;
    glm::vec4 color;
};


//полуправильный выпуклый многогранник, 62 вершины, 60 граней, 120 ребер
class DeltoidalHexecontahedron
{
public:
    DeltoidalHexecontahedron(float radius)
    {
        Generate(radius);
    }
    
    void Draw()
    {
        for (const auto& face : m_faces)
        {
            glBegin(GL_QUADS);
            glNormal3fv(glm::value_ptr(face.normal));
            glColor4fv(glm::value_ptr(face.color));
            for (int index : face.indices)
            {
                glVertex3fv(glm::value_ptr(m_vertices[index]));
            }
            glEnd();
        }
    }

    void DrawLines()
    {
        glLineWidth(2.0f);
        glColor3f(0.0f, 0.0f, 0.0f);
        for (const auto face : m_faces)
        {
            glBegin(GL_LINE_LOOP);
            for (int index : face.indices)
            {
                glVertex3fv(glm::value_ptr(m_vertices[index]));
            }
            glEnd();
        }
    }

private:
    std::vector<Face> m_faces;
    std::vector<glm::vec3> m_vertices;
    

    int AddVertex(glm::vec3 v)
    {
        for (int i = 0; i < m_vertices.size(); ++i)
        {
            if (glm::distance(m_vertices[i], v) < 0.0001f)
            {
                return i;
            }
        }
        m_vertices.push_back(v);
        return m_vertices.size() - 1;
    }

    glm::vec4 HslToRgb(float h, float s, float l)
    {
        // h в [0, 360), s, l в [0, 1]
        float c = (1.0f - std::fabs(2.0f * l - 1.0f)) * s;
        float hh = h / 60.0f;
        float x = c * (1.0f - std::fabs(std::fmod(hh, 2.0f) - 1.0f));
        float m = l - c / 2.0f;

        float r = 0, g = 0, b = 0;
        if (hh < 1.0f) { r = c; g = x; }
        else if (hh < 2.0f) { r = x; g = c; }
        else if (hh < 3.0f) { g = c; b = x; }
        else if (hh < 4.0f) { g = x; b = c; }
        else if (hh < 5.0f) { r = x; b = c; }
        else { r = c; b = x; }

        return { r + m, g + m, b + m, 1.0f };
    }

    void AddFace(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, const glm::vec4& color)
    {
        Face f{};
        glm::vec3 normal = glm::normalize(glm::cross(b - a, c - a));
        glm::vec3 center = (a + b + c + d) / 4.0f;
        
        if (glm::dot(normal, center) < 0.0f)
        {
            normal = -normal;
            f.indices = { AddVertex(a), AddVertex(d), AddVertex(c), AddVertex(b) };
        }
        else
        {
            f.indices = { AddVertex(a), AddVertex(b), AddVertex(c), AddVertex(d) };
        }

        f.normal = normal;

        f.color = color;

        m_faces.push_back(f);
    }

    static constexpr unsigned ICOSAHEDRON_VERTICES = 12;
    void Generate(float radius)
    {
        float phi = (1 + sqrt(5)) / 2; //золотое сечение, за основой икосаэдр
        glm::vec3 icosahedron[ICOSAHEDRON_VERTICES] = {
            {0, 1, phi},    {0, -1, phi},   {0, 1, -phi},   {0, -1, -phi},
            {1, phi, 0},    {-1, phi, 0},   {1, -phi, 0},   {-1, -phi, 0},
            {phi, 0, 1},    {phi, 0, -1},   {-phi, 0, 1},   {-phi, 0, -1}
        };

        int groupIndex{};
                    //v3 - центр грани икосаэдра
                    //p5.. - 3 вершины икосаэдра, поднятые на радиус
                    //p4.. - 3 вершины середины граней икосаэдра, поднятые на радиус 
        glm::vec3 v3{}, p5i{}, p5j{}, p5k{}, p4ij{}, p4jk{}, p4ki{};
        for (int i = 0; i < ICOSAHEDRON_VERTICES; ++i)
        {
            for (int j = i + 1; j < ICOSAHEDRON_VERTICES; ++j)
            {
                for (int k = j + 1; k < ICOSAHEDRON_VERTICES; ++k)
                {
                    if (abs(glm::distance(icosahedron[i], icosahedron[j]) - 2.0f) < 0.1f &&
                        abs(glm::distance(icosahedron[j], icosahedron[k]) - 2.0f) < 0.1f &&
                        abs(glm::distance(icosahedron[k], icosahedron[i]) - 2.0f) < 0.1f)
                    {
                        v3 = glm::normalize(icosahedron[i] + icosahedron[j] + icosahedron[k]) * radius;
                        p5i = glm::normalize(icosahedron[i]) * radius;
                        p5j = glm::normalize(icosahedron[j]) * radius;
                        p5k = glm::normalize(icosahedron[k]) * radius;
                        p4ij = glm::normalize(icosahedron[i] + icosahedron[j]) * radius;
                        p4jk = glm::normalize(icosahedron[j] + icosahedron[k]) * radius;
                        p4ki = glm::normalize(icosahedron[k] + icosahedron[i]) * radius;

                        float hue = static_cast<float>(groupIndex) * (360.0f / 20.0f);
                        glm::vec4 groupColor = HslToRgb(hue, 0.65f, 0.5f);
                        AddFace(v3, p4ij, p5j, p4jk, groupColor);
                        AddFace(v3, p4jk, p5k, p4ki, groupColor);
                        AddFace(v3, p4ki, p5i, p4ij, groupColor);
                        ++groupIndex;
                    }
                }
            }
        }

    }
};


class HexecontahedronApplication : public GLApplication
{
public:
    HexecontahedronApplication(const char* title, int width, int height)
        : GLApplication(title, width, height)
        , m_rotationController(width, height)
    {}

    ~HexecontahedronApplication() {}

protected:

    virtual void OnInit()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_DEPTH_TEST);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(
            0, 0, 12,
            0, 0, 0,
            0, 1, 0
        );
    }

    virtual void OnDisplay()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);

        //ребра непрозрачные
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
        m_deltoidalHexecontahedron.DrawLines();

        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDepthMask(GL_FALSE);

        glEnable(GL_CULL_FACE);

        //рисуются задние грани и отсекаются передние
        glCullFace(GL_FRONT);
        m_deltoidalHexecontahedron.Draw();

        //рисуются передние и отсекаются задние
        //
        glCullFace(GL_BACK);
        m_deltoidalHexecontahedron.Draw();

        glDisable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_LIGHTING);
    }

    virtual void OnReshape(int width, int height)
    {
        glViewport(0, 0, width, height);
        m_rotationController.ResizeWindow(width, height);
        double aspectRatio = double(width) / double(height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(m_fov, aspectRatio, m_zNear, m_zFar);
        glMatrixMode(GL_MODELVIEW);
    }

    virtual void OnMouse(int button, int state, int x, int y)
    {
        m_rotationController.OnMouse(button, state, x, y);
    }

    virtual void OnMotion(int x, int y)
    {
        if (m_rotationController.LeftButtonIsPressed())
        {
            m_rotationController.OnMotion(x, y);
            PostRedisplay();
        }
    }

private:
    RotationController m_rotationController;
    DeltoidalHexecontahedron m_deltoidalHexecontahedron{ 2.5f };
    double m_fov = 45.0;
    double m_zNear = 0.1;
    double m_zFar = 15.0;
};


void RunLW4Task1()
{
    HexecontahedronApplication app{"My deltoidal hexecontahedron", 800, 600};
    app.MainLoop();
} 
