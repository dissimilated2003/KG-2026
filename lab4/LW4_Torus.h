#pragma once

#include "Utils.h"


constexpr float PI = 3.1415926f;

//задание 2 - Разработайте OpenGL-приложение, визуализирующее трехмерную, 
// функционально заданную поверхность, соответствующую одному из заданных вариантов.
// вариант 8 = ТОР


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


class Torus
{
public:
    Torus(float R, float r, int alphaSegments, int betaSegments)
        : m_R(R)
        , m_r(r)
        , m_alphaSegments(alphaSegments)
        , m_betaSegments(betaSegments)
    {
        Generate();
    }

    void Draw()
    {
        for (const auto& q : m_faces)
        {
            glBegin(GL_QUADS);
            //обход против часовой
            glNormal3fv(glm::value_ptr(m_normals[q.a]));
            glColor4fv(glm::value_ptr(m_colors[q.a]));
            glVertex3fv(glm::value_ptr(m_vertices[q.a]));

            glNormal3fv(glm::value_ptr(m_normals[q.b]));
            glColor4fv(glm::value_ptr(m_colors[q.b]));
            glVertex3fv(glm::value_ptr(m_vertices[q.b]));

            glNormal3fv(glm::value_ptr(m_normals[q.c]));
            glColor4fv(glm::value_ptr(m_colors[q.c]));
            glVertex3fv(glm::value_ptr(m_vertices[q.c]));

            glNormal3fv(glm::value_ptr(m_normals[q.d]));
            glColor4fv(glm::value_ptr(m_colors[q.d]));
            glVertex3fv(glm::value_ptr(m_vertices[q.d]));
            glEnd();
        }
    }

    void DrawLines()
    {
        glLineWidth(1.0f);
        glColor3f(0.0f, 0.0f, 0.0f);

        for (const auto& q : m_faces)
        {
            glBegin(GL_LINE_LOOP);
            glVertex3fv(glm::value_ptr(m_vertices[q.a]));
            glVertex3fv(glm::value_ptr(m_vertices[q.b]));
            glVertex3fv(glm::value_ptr(m_vertices[q.c]));
            glVertex3fv(glm::value_ptr(m_vertices[q.d]));
            glEnd();
        }
    }


private:
    struct Quad
    {
        int a{};
        int b{};
        int c{};
        int d{};
    };

    float m_R{};
    float m_r{};
    unsigned m_alphaSegments{};
    unsigned m_betaSegments{};

    std::vector<glm::vec3> m_vertices{};
    std::vector<glm::vec3> m_normals{};
    std::vector<glm::vec4> m_colors{};
    std::vector<Quad> m_faces{};

    //		x(a,b) = (R + r*cos a)*cos b        R радиус направляющей окружности
    //		y(a,b) = (R + r*cos a)*sin b        r радиус образующей окружности
    //		z(a,b) = r*sin a
    //		a,b = [0, 2PI]
    glm::vec3 Point(float alpha, float beta) const
    {
        float cosa = std::cos(alpha);
        float sina = std::sin(alpha);
        float cosb = std::cos(beta);
        float sinb = std::sin(beta);

        return 
        {
            (m_R + m_r * cosa) * cosb,
            (m_R + m_r * cosa) * sinb,
            m_r * sina
        };
    }

    glm::vec3 Normal(float alpha, float beta) const
    {
        float cosa = std::cos(alpha);
        float sina = std::sin(alpha);
        float cosb = std::cos(beta);
        float sinb = std::sin(beta);

        return glm::normalize(glm::vec3(cosa * cosb, cosa * sinb, sina));
    }

    void Generate()
    {
        const int N = m_alphaSegments;
        const int M = m_betaSegments;

        const float da = 2.0f * PI / static_cast<float>(N);
        const float db = 2.0f * PI / static_cast<float>(M);

        m_vertices.reserve((N + 1) * (M + 1));
        m_normals.reserve((N + 1) * (M + 1));
        m_faces.reserve((N + 1) * (M + 1));

        for (int i = 0; i <= N; ++i)
        {
            float alpha = static_cast<float>(i) * da;
            for (int j = 0; j <= M; ++j)
            {
                float beta = static_cast<float>(j) * db;

                glm::vec3 point = Point(alpha, beta);
                glm::vec3 normal = Normal(alpha, beta);
                glm::vec4 color = { 0.2f, 0.4f, 0.85f, 1.0f };

                m_vertices.push_back(point);
                m_normals.push_back(normal);
                m_colors.push_back(color);
            }
        }

        m_faces.reserve(N * M);
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < M; ++j)
            {
                int i0 = i * (M + 1) + j;
                int i1 = (i + 1) * (M + 1) + j;
                int i2 = (i + 1) * (M + 1) + (j + 1);
                int i3 = i * (M + 1) + (j + 1);

                m_faces.push_back({ i0, i3, i2, i1 });
            }
        }
    }
};


class TorusApplication : public GLApplication
{
public:
    TorusApplication(const char* title, int width, int height)
        : GLApplication(title, width, height)
        , m_rotationController(width, height)
    {}

    ~TorusApplication() {}

protected:
    virtual void OnInit()
    {
        glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_NORMALIZE);

        GLfloat lightPos[] = { 1.0f, 1.5f, 1.0f, 0.0f };
        GLfloat lightAmbient[] = { 0.1f, 0.1f, 0.15f, 1.0f };
        GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

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

        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

        GLfloat matSpecular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 128.0f);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        m_torus.Draw();

        glDisable(GL_CULL_FACE);
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
                // R     r     seg seg
    Torus m_torus{ 3.0f, 1.2f, 48, 48 };  
    double m_fov = 45.0;
    double m_zNear = 0.1;
    double m_zFar = 30.0;
};


void RunLW4Task2()
{
    TorusApplication app{ "Thor", 800, 600 };
    app.MainLoop();
}
