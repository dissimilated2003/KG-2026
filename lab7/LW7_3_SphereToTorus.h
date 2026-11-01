#pragma once

#include "Utils.h"
#include "ShaderUtils.h"

extern constexpr float PI = 3.14159;


class RotationController
{
public:
    RotationController(int windowWidth, int windowHeight)
        : m_windowWidth(windowWidth)
        , m_windowHeight(windowHeight) 
    {}

    void ResizeWindow(int width, int height)
    {
        m_windowWidth = width;
        m_windowHeight = height;
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

            GLfloat rotateX = GLfloat(dy) * 180.0f / m_windowHeight;
            GLfloat rotateY = GLfloat(dx) * 180.0f / m_windowWidth;

            RotateCamera(rotateX, rotateY);
            m_mouseX = x;
            m_mouseY = y;
        }
    }

private:
    bool m_leftButtonPressed = false;
    int m_mouseX{}, m_mouseY{};
    int m_windowWidth{}, m_windowHeight{};

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

        modelView[0] = xAxis.x; modelView[4] = xAxis.y; modelView[8] = xAxis.z;
        modelView[1] = yAxis.x; modelView[5] = yAxis.y; modelView[9] = yAxis.z;
        modelView[2] = zAxis.x; modelView[6] = zAxis.y; modelView[10] = zAxis.z;

        glLoadMatrixf(modelView);
    }
};



class AnimationController
{
public:
    AnimationController() 
    { 
        Reset(); 
    }

    void Reset()
    {
        m_lastTick = GetTickCount64();
        m_delta = 0;
    }

    void Tick()
    {
        auto const currentTick = GetTickCount64();
        m_delta = currentTick - m_lastTick;
        m_lastTick = currentTick;
    }

    float GetTimeDelta() const
    {
        return m_delta * 0.001f;
    }
private:
    ULONGLONG m_lastTick;
    ULONGLONG m_delta;
};



class SphereToTorusApp : public GLApplication
{
public:
    SphereToTorusApp()
        : GLApplication("Sphere to Torus morphing", 800, 600, true, false),
        m_rotationController(800, 600),
        m_phase(0.0f),
        m_time(0.0f),
        m_useLighting(false)
    {}

    ~SphereToTorusApp()
    {
        glDeleteProgram(m_program);
    }

protected:
    void OnInit() override
    {
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

        InitShaders();
        InitGeometry();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(0.0, 3.0, 8.0,
            0.0, 0.0, 0.0,
            0.0, 1.0, 0.0);
    }

    void InitGeometry()
    {
        int numStepsU = 60;
        int numStepsV = 30;
        float stepU = 2.0f * PI / numStepsU;
        float stepV = 2.0f * PI / numStepsV;

        std::vector<glm::vec3> vertices;

        for (int i = 0; i < numStepsU; ++i)
        {
            for (int j = 0; j < numStepsV; ++j)
            {
                float u1 = i * stepU;
                float v1 = j * stepV;
                float u2 = (i + 1) * stepU;
                float v2 = (j + 1) * stepV;

                vertices.push_back(glm::vec3(u1, v1, 0.0f));
                vertices.push_back(glm::vec3(u2, v1, 0.0f));
                vertices.push_back(glm::vec3(u2, v2, 0.0f));
                vertices.push_back(glm::vec3(u1, v2, 0.0f));
            }
        }
        m_vertexCount = vertices.size();

        m_vertexBuffer.Create();
        m_vertexBuffer.BufferData(
            m_vertexCount * sizeof(glm::vec3),
            &vertices[0],
            GL_STATIC_DRAW
        );
    }

    void InitShaders()
    {
        ShaderLoader loader;

#pragma region vertex shader
        const std::string vShader =
            R"(
uniform float u_phase;


varying vec3 vNormal;
varying vec3 vViewPos;


void main()
{
    float u = gl_Vertex.x;
    float v = gl_Vertex.y;

    float Rt = mix(0.0, 2.0, u_phase);
    float rt = mix(2.0, 0.8, u_phase);


    vec3 pos = vec3(
        (Rt + rt * cos(v)) * cos(u),
        (Rt + rt * cos(v)) * sin(u),
        rt * sin(v)
    );


    vec3 normal = normalize(vec3(
        cos(v) * cos(u),
        cos(v) * sin(u),
        sin(v)
    ));

    vNormal = normalize(gl_NormalMatrix * normal);
    vViewPos = (gl_ModelViewMatrix * vec4(pos, 1.0)).xyz;

    gl_Position = gl_ModelViewProjectionMatrix * vec4(pos, 1.0);
}
            )";
#pragma endregion
        Shader vertexShader = loader.LoadShaderFromString(GL_VERTEX_SHADER, vShader);

#pragma region fragment shader
        const std::string fShader = R"(
varying vec3 vNormal;
varying vec3 vViewPos;

void main()
{
    vec3 objectColor = vec3(0.2, 0.4, 0.9);

    vec3 lightPos = vec3(5.0, 5.0, 5.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;


    vec3 norm = normalize(vNormal); 
    vec3 lightDir = normalize(lightPos - vViewPos); 
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 result = (ambient + diffuse) * objectColor;
    gl_FragColor = vec4(result, 1.0);
}
            )";
#pragma endregion
        Shader fragmentShader = loader.LoadShaderFromString(GL_FRAGMENT_SHADER, fShader);

        m_program.Create();
        m_program.AttachShader(vertexShader);
        m_program.AttachShader(fragmentShader);

        ShaderCompiler compiler;
        compiler.CompileShader(vertexShader);
        compiler.CompileShader(fragmentShader);
        compiler.CheckStatus();

        ProgramLinker linker;
        linker.LinkProgram(m_program);
        linker.CheckStatus();

        m_phaseLocation = m_program.GetUniformLocation("u_phase");
    }

    void OnDisplay() override
    {
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(m_program);
        glUniform1f(m_phaseLocation, m_phase);

        if (m_useLighting)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_LIGHTING);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glDisable(GL_LIGHTING);
            glColor3f(0.4f, 0.8f, 1.0f);
        }

        m_vertexBuffer.Bind();
        glVertexPointer(3, GL_FLOAT, sizeof(glm::vec3), m_vertexBuffer.GetBufferPointer());
        glEnableClientState(GL_VERTEX_ARRAY);

        glDrawArrays(GL_QUADS, 0, m_vertexCount);

        glDisableClientState(GL_VERTEX_ARRAY);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glUseProgram(0);
    }

    void OnMouse(int button, int state, int x, int y) override
    {
        m_rotationController.OnMouse(button, state, x, y);
    }

    void OnMotion(int x, int y) override
    {
        if (m_rotationController.LeftButtonIsPressed())
        {
            m_rotationController.OnMotion(x, y);
            PostRedisplay();
        }
    }

    void OnKeyboard(unsigned char key, int x, int y) override
    {
        if (key == 'l' || key == 'L')
        {
            m_useLighting = !m_useLighting;
            PostRedisplay();
        }

        if (key == 'p' || key == 'P')
        {
            m_paused = !m_paused;
            PostRedisplay();
        }
    }

    void OnReshape(int width, int height) override
    {
        glViewport(0, 0, width, height);
        m_rotationController.ResizeWindow(width, height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0, double(width) / double(height), 0.1, 100.0);
        glMatrixMode(GL_MODELVIEW);
    }

    void OnIdle() override
    {
        if (!m_paused)
        {
            m_animationController.Tick();
            m_time += m_animationController.GetTimeDelta();

            m_phase = (sinf(m_time * 1.5f) + 1.0f) * 0.5f;

            PostRedisplay();
        }
    }

private:
    Program m_program;
    VertexBuffer m_vertexBuffer;
    size_t m_vertexCount{};

    RotationController m_rotationController;
    AnimationController m_animationController;

    GLint m_phaseLocation{};
    float m_phase{};
    float m_time{};
    bool m_useLighting{};
    bool m_paused{};
};


void RunLW7Task3()
{
    SphereToTorusApp app;
    try
    {
        app.MainLoop();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << "\n";
    }
}
