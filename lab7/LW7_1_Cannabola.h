#pragma once


#include "ShaderUtils.h"


extern constexpr float PI = 3.1415f;


class CannabolaApp : public GLApplication
{
public:
	CannabolaApp() : GLApplication("My cannabola", 800, 600, false, true) 
	{
	}

	~CannabolaApp()
	{
		glDeleteProgram(m_program);
	}

protected:
	void OnInit() override
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glColor3f(0.0f, 0.8f, 0.2f);
		InitShaders();

		int steps = 2000;
		float step = 2 * PI / steps;
		std::vector<glm::fvec3> vertices;
		vertices.reserve(steps + 1);
		for (int i = 0; i <= steps; ++i)
		{
			vertices.push_back(glm::fvec3(i * step, 0.0f, 0.0f));
		}

		m_vertexCount = vertices.size();
		m_vertexBuffer.Create();
		m_vertexBuffer.BufferData(
			vertices.size() * sizeof(glm::fvec3), 
			&vertices[0], 
			GL_STATIC_DRAW
		);

	}

	void OnDisplay() override
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(m_program);
		
		m_vertexBuffer.Bind();
		glVertexPointer(
			3, 
			GL_FLOAT, 
			sizeof(glm::fvec3), 
			m_vertexBuffer.GetBufferPointer()
		);

		glEnableClientState(GL_VERTEX_ARRAY);
		glDrawArrays(GL_LINE_STRIP, 0, m_vertexCount);
		glDisableClientState(GL_VERTEX_ARRAY);

		glUseProgram(0);
	}

	void OnReshape(int width, int height) override
	{
		glViewport(0, 0, width, height);
		double aspect = double(width) / double(height);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		const float BOUNDS_SIZE = 1.5f;
		const float Y_OFFSET = 1.0f;

		if (aspect >= 1.0)
		{
			glOrtho(
				-BOUNDS_SIZE * aspect, 
				BOUNDS_SIZE * aspect, 
				-BOUNDS_SIZE + Y_OFFSET, 
				BOUNDS_SIZE + Y_OFFSET,
				-1.0, 
				1.0
			);
		}
		else
		{
			glOrtho(
				-BOUNDS_SIZE, 
				BOUNDS_SIZE, 
				-BOUNDS_SIZE / aspect + Y_OFFSET,
				BOUNDS_SIZE / aspect + Y_OFFSET,
				-1.0, 
				1.0
			);
		}

		glMatrixMode(GL_MODELVIEW);
	}

	void OnMouse(int button, int state, int x, int y) override
	{
	}

	void OnMotion(int x, int y) override
	{
	}

	void OnIdle() override
	{
	}

	void InitShaders()
	{
		ShaderLoader loader;


		Shader vertexShader = loader.LoadShaderFromString(GL_VERTEX_SHADER,
#pragma region vertex shader
			R"(
void main()
{
	float x = gl_Vertex.x;
	float r = (1.0 + sin(x)) * (1.0 + 0.9 * cos(8.0 * x)) * 
			(1.0 + 0.1 * cos(24.0 * x)) * (0.5 + 0.05 * cos(140.0 * x));

	vec4 pos = vec4(r * cos(x), r * sin(x), 0.0, 1.0);

	gl_Position = gl_ModelViewProjectionMatrix * pos;
	gl_FrontColor = gl_Color;
}					
			)"
#pragma endregion
		);


		m_program.Create();
		m_program.AttachShader(vertexShader);

		ShaderCompiler compiler;
		compiler.CompileShader(vertexShader);
		compiler.CheckStatus();

		ProgramLinker linker;
		linker.LinkProgram(m_program);
		linker.CheckStatus();
	}

private:

	Program m_program;
	size_t m_vertexCount{};
	VertexBuffer m_vertexBuffer;
};


void RunLW7Task1()
{
	CannabolaApp app;
	try
	{
		app.MainLoop();
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << "\n";
	}
}
