#pragma once

#include "Utils.h"
#include "ShaderUtils.h"


class ActionController
{
public:
    ActionController()
        : m_center(-0.5f, 0.0f)
        , m_zoom(1.2f)
    {}

    void OnSpecial(int key)
    {
        float moveStep = 0.1f / m_zoom;

        switch (key)
        {
        case GLUT_KEY_LEFT:
            m_center.x -= moveStep;
            break;
        case GLUT_KEY_RIGHT:
            m_center.x += moveStep;
            break;
        case GLUT_KEY_UP:
            m_center.y += moveStep;
            break;
        case GLUT_KEY_DOWN:
            m_center.y -= moveStep;
            break;
        case GLUT_KEY_F8: //PgDown, PgUp не работают
            m_zoom *= 1.15f;
            if (m_zoom > 50000.0f) m_zoom = 50000.0f;
            break;
        case GLUT_KEY_F7:
            m_zoom /= 1.15f;
            break;
        }
    }

    glm::vec2 GetCenter() const 
    { 
        return m_center; 
    }

    float GetZoom() const 
    { 
        return m_zoom; 
    }

private:
    glm::vec2 m_center;
    float m_zoom;
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

    float GetTimeDelta() const { return m_delta * 0.001f; }
private:
    ULONGLONG m_lastTick;
    ULONGLONG m_delta;
};



class MandelbrotApp : public GLApplication
{
public:
    MandelbrotApp() :
        GLApplication("Mandelbrot Zoom", 800, 600, false, false),
        m_time(0.0f)
    {}

    ~MandelbrotApp()
    {
        glDeleteProgram(m_program);
    }

protected:
    void OnInit() override
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        InitShaders();
    }

    void InitShaders()
    {
        ShaderLoader loader;
        Shader vertexShader = loader.LoadShaderFromString(GL_VERTEX_SHADER,
#pragma region vertex shader
            R"(
void main()
{
    gl_Position = gl_Vertex;
    gl_TexCoord[0] = gl_MultiTexCoord0;
}
            )"
#pragma endregion
        );


        Shader fragmentShader = loader.LoadShaderFromString(GL_FRAGMENT_SHADER,
#pragma region fragment shader
            R"(
uniform vec2 u_center;
uniform float u_zoom;
uniform float u_aspect;
uniform float u_time;
uniform float u_maxIter;


vec3 InigoQuilezPalette(in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d) 
{
    return a + b * cos(6.28318 * (c * t + d));
}


void main()
{
    vec2 uv = (gl_TexCoord[0].xy - 0.5) * vec2(u_aspect, 1.0);
    vec2 c = uv / u_zoom + u_center;


    vec2 z = vec2(0.0);
    int maxIter = int(u_maxIter);
    int iter = 0;
    
    for (int i = 0; i < 10000; i++) 
    {
        if (i >= maxIter) break;
        z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
        
        if (dot(z, z) > 1e3)
        {
            break;
        }
        iter++;
    }

    if (iter == maxIter) 
    {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } 
    else 
    {
        vec3 color = InigoQuilezPalette(
            float(iter) * 0.015 + u_time * 0.1, 
            vec3(0.5, 0.5, 0.5),
            vec3(0.5, 0.5, 0.5),
            vec3(1.0, 1.0, 1.0),
            vec3(0.0, 0.1, 0.2)
        );

        gl_FragColor = vec4(color, 1.0);
    }
}
            )"
#pragma endregion
        );

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
    }

    void OnDisplay() override
    {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_program);

        glUniform2f(m_program.GetUniformLocation("u_center"),
            m_actionController.GetCenter().x, m_actionController.GetCenter().y);
        glUniform1f(m_program.GetUniformLocation("u_zoom"),
            m_actionController.GetZoom());
        glUniform1f(m_program.GetUniformLocation("u_aspect"),
            (float)m_width / (float)m_height);
        glUniform1f(m_program.GetUniformLocation("u_time"),
            m_time);
        float maxIter = 150.0f + 100.0f * log(1.0f + m_actionController.GetZoom());
        glUniform1f(m_program.GetUniformLocation("u_maxIter"), maxIter);

        glBegin(GL_QUADS);
        {
            glTexCoord2f(0.0f, 0.0f); 
            glVertex2f(-1.0f, -1.0f);

            glTexCoord2f(1.0f, 0.0f); 
            glVertex2f(1.0f, -1.0f);

            glTexCoord2f(1.0f, 1.0f); 
            glVertex2f(1.0f, 1.0f);

            glTexCoord2f(0.0f, 1.0f); 
            glVertex2f(-1.0f, 1.0f);
        }
        glEnd();

        glUseProgram(0);
    }

    void OnSpecial(int key, int x, int y) override
    {
        m_actionController.OnSpecial(key);
        PostRedisplay();
    }

    void OnReshape(int width, int height) override
    {
        m_width = width;
        m_height = height;
        glViewport(0, 0, width, height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
    }

    void OnIdle() override
    {
        m_animationController.Tick();
        m_time += m_animationController.GetTimeDelta();

        PostRedisplay();
    }

private:
    Program m_program;
    ActionController m_actionController;
    AnimationController m_animationController;

    float m_time;
    int m_width = 800;
    int m_height = 600;
};


void RunLW7Task4()
{
    MandelbrotApp app;
    try
    {
        app.MainLoop();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << "\n";
    }
}
