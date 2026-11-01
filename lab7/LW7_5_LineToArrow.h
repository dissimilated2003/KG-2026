#pragma once


#include "Utils.h"
#include "ShaderUtils.h"


class LineToArrowApp : public GLApplication
{
public:
    LineToArrowApp() : GLApplication("Line to arrow", 800, 600, false, false)
    {
        m_vertices[0] = glm::vec3(-0.8f, -0.5f, 0.0f);
        m_vertices[1] = glm::vec3(0.8f, 0.5f, 0.0f);
    }

    ~LineToArrowApp()
    {
        glDeleteProgram(m_program);
    }

protected:
    void OnInit() override
    {
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(-m_aspect, m_aspect, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);

        InitShaders();
        InitGeometry();
    }

    void InitGeometry()
    {
        m_vertexBuffer.Create();
        m_vertexBuffer.BufferData(sizeof(glm::vec3) * 2, m_vertices, GL_DYNAMIC_DRAW);
    }

    void InitShaders()
    {
        ShaderLoader loader;
        Shader vertexShader = loader.LoadShaderFromString(GL_VERTEX_SHADER,
#pragma region vertex shader
            R"(
#version 150
in vec3 aPos;
out vec3 vPos;


void main()
{
    vPos = aPos;
    gl_Position = vec4(aPos, 1.0);
}
            )"
#pragma endregion
        );


        Shader geometryShader = loader.LoadShaderFromString(GL_GEOMETRY_SHADER,
#pragma region geometry shader
            R"(
#version 150
layout(lines) in;
layout(line_strip, max_vertices = 6) out;


in vec3 vPos[];
out vec3 gColor;


uniform mat4 u_mvp;


void main()
{
    vec2 p0 = vPos[0].xy;
    vec2 p1 = vPos[1].xy;
    
    gColor = vec3(0.0, 0.0, 0.0);


    gl_Position = u_mvp * vec4(p0, 0.0, 1.0);
    EmitVertex();
    gl_Position = u_mvp * vec4(p1, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();


    vec2 dir = p1 - p0;
    float len = length(dir);
    
    if (len < 0.001) return;

    vec2 nDir = normalize(dir);
    vec2 normal = vec2(-nDir.y, nDir.x);


    float arrowLen = len * 0.25;
    float halfAngle = radians(15.0);
    float wingWidth = arrowLen * tan(halfAngle);


    vec2 base = p1 - nDir * arrowLen;
    vec2 wing1 = base + normal * wingWidth;
    vec2 wing2 = base - normal * wingWidth;


    gl_Position = u_mvp * vec4(p1, 0.0, 1.0);
    EmitVertex();
    gl_Position = u_mvp * vec4(wing1, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();


    gl_Position = u_mvp * vec4(p1, 0.0, 1.0);
    EmitVertex();
    gl_Position = u_mvp * vec4(wing2, 0.0, 1.0);
    EmitVertex();
    EndPrimitive();
}
            )"
#pragma endregion
        );


        Shader fragmentShader = loader.LoadShaderFromString(GL_FRAGMENT_SHADER,
#pragma region fragment shader
            R"(
#version 150
in vec3 gColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(gColor, 1.0);
}
            )"
#pragma endregion
        );

        m_program.Create();
        m_program.AttachShader(vertexShader);
        m_program.AttachShader(geometryShader);
        m_program.AttachShader(fragmentShader);

        glBindAttribLocation(m_program, 0, "aPos");

        ShaderCompiler compiler;
        compiler.CompileShader(vertexShader);
        compiler.CompileShader(geometryShader);
        compiler.CompileShader(fragmentShader);
        compiler.CheckStatus();

        ProgramLinker linker;
        linker.LinkProgram(m_program);
        linker.CheckStatus();

        m_mvpLocation = glGetUniformLocation(m_program, "u_mvp");
    }

    void OnDisplay() override
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(m_program);

        GLfloat mvp[16];
        glGetFloatv(GL_PROJECTION_MATRIX, mvp);
        glUniformMatrix4fv(m_mvpLocation, 1, GL_FALSE, mvp);

        m_vertexBuffer.Bind();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);

        glDrawArrays(GL_LINES, 0, 2);

        glDisableVertexAttribArray(0);
        m_vertexBuffer.Unbind();
        glUseProgram(0);

        DrawVertexMarkers();
    }

    void OnReshape(int width, int height) override
    {
        m_width = width;
        m_height = height;
        glViewport(0, 0, width, height);
        m_aspect = double(width) / double(height);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(-m_aspect, m_aspect, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
    }

    void OnMouse(int button, int state, int x, int y) override
    {
        if (button == GLUT_LEFT_BUTTON)
        {
            if (state == GLUT_DOWN)
            {
                glm::vec2 worldPos = ScreenToWorld(x, y);
                m_draggedVertex = FindClosestVertex(worldPos);
            }
            else if (state == GLUT_UP)
            {
                m_draggedVertex = -1;
            }
        }
    }

    void OnMotion(int x, int y) override
    {
        if (m_draggedVertex >= 0)
        {
            glm::vec2 worldPos = ScreenToWorld(x, y);
            m_vertices[m_draggedVertex].x = worldPos.x;
            m_vertices[m_draggedVertex].y = worldPos.y;

            m_vertexBuffer.Bind();
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * 2, m_vertices);
            m_vertexBuffer.Unbind();

            PostRedisplay();
        }
    }

private:
    Program m_program;
    VertexBuffer m_vertexBuffer;
    GLint m_mvpLocation{};

    glm::vec3 m_vertices[2];
    int m_draggedVertex = -1;

    int m_width = 800;
    int m_height = 600;
    double m_aspect = 800.0 / 600.0;

#pragma region drag and drop
    glm::vec2 ScreenToWorld(int x, int y) const
    {
        float wx = (float(x) / m_width) * 2.0f * m_aspect - m_aspect;
        float wy = 1.0f - (float(y) / m_height) * 2.0f;
        return glm::vec2(wx, wy);
    }

    int FindClosestVertex(glm::vec2 worldPos) const
    {
        const float captureRadius = 0.1f;
        int bestIdx = -1;
        float bestDist = captureRadius;

        for (int i = 0; i < 2; ++i)
        {
            glm::vec2 vPos(m_vertices[i].x, m_vertices[i].y);
            float dist = glm::distance(vPos, worldPos);
            if (dist < bestDist)
            {
                bestDist = dist;
                bestIdx = i;
            }
        }
        return bestIdx;
    }

    void DrawVertexMarkers() const
    {
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.8f, 0.2f);
        float mSize = 0.02f;

        for (int i = 0; i < 2; ++i)
        {
            float cx = m_vertices[i].x;
            float cy = m_vertices[i].y;

            glBegin(GL_QUADS);
            glVertex2f(cx - mSize, cy - mSize);
            glVertex2f(cx + mSize, cy - mSize);
            glVertex2f(cx + mSize, cy + mSize);
            glVertex2f(cx - mSize, cy + mSize);
            glEnd();
        }
    }
#pragma endregion

};

void RunLW7Task5()
{
    LineToArrowApp app;
    try
    {
        app.MainLoop();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << "\n";
    }
}
