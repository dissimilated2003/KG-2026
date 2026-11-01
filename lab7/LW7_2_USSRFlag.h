#pragma once

#include "ShaderUtils.h"


class USSRFlagApp : public GLApplication
{
public:
	USSRFlagApp() : GLApplication("USSR Flag", 800, 400, false, true) {}

	~USSRFlagApp()
	{
		glDeleteProgram(m_program);
	}

protected:
	void OnInit() override
	{
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		InitShaders();
	}

	void OnDisplay() override
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(m_program);

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
const vec3 RED = vec3(0.8, 0.0, 0.0);
const vec3 GOLD = vec3(1.0, 0.843, 0.0);


// Check if the point p is on the left side of the line p0p1
bool PointIsOnTheLeft(vec2 p0, vec2 p1, vec2 p)
{
	vec2 p0p1 = p1 - p0;
	// find the orthogonal vector to p0p1
    vec2 n = vec2(-p0p1.y, p0p1.x);

	// Find the dot product between n and (p - p0)
    return dot(p - p0, n) > 0.0;
}


bool PointIsInsideTriangle(vec2 p0, vec2 p1, vec2 p2, vec2 p)
{
    return
        PointIsOnTheLeft(p0, p1, p) &&
        PointIsOnTheLeft(p1, p2, p) &&
        PointIsOnTheLeft(p2, p0, p);
}


vec2 Rotate(vec2 p, vec2 center, float angle)
{
    vec2 d = p - center;
    float c = cos(angle);
    float s = sin(angle);
    return center + vec2(c * d.x - s * d.y, s * d.x + c * d.y);
}


bool PointIsInsideStar(vec2 starCenter, vec2 top, vec2 left, vec2 right, vec2 pos, int N)
{
	bool star = false;
	for (int i = 0; i < N; ++i)
	{
		float angle = float(i) * 2.0 * 3.14159265 / float(N);

		vec2 v0 = Rotate(top, starCenter, angle);
		vec2 v1 = Rotate(left, starCenter, angle);
		vec2 v2 = Rotate(right, starCenter, angle);

		if (PointIsInsideTriangle(v0, v1, v2, pos))
		{
			star = true;
			break;
		}
	}
	return star;
}


bool PointIsInCircle(vec2 center, float radius, vec2 pos)
{
	return length(pos - center) <= radius;
}


void main() 
{
    vec2 uv = gl_TexCoord[0].xy;
	vec2 tiledUV = fract(uv);
    vec2 pos = vec2(tiledUV.x * 2.0, tiledUV.y);

	const int starRays = 6;
	vec2 starCenter = vec2(0.33, 0.85);

	vec2 topOuter = vec2(0.33, 0.95);
	vec2 leftOuter = vec2(0.29, 0.82);
	vec2 rightOuter = vec2(0.37, 0.82);

	vec2 topInner = vec2(0.33, 0.92);
	vec2 leftInner = vec2(0.30, 0.83);
	vec2 rightInner = vec2(0.36, 0.83);
	bool starOuter = PointIsInsideStar(starCenter, topOuter, leftOuter, rightOuter, pos, starRays);
	bool starInner = PointIsInsideStar(starCenter, topInner, leftInner, rightInner, pos, starRays);

	bool hammerHandle = 
		PointIsInsideTriangle(vec2(0.23, 0.63), vec2(0.43, 0.43), vec2(0.455, 0.455), pos) ||
		PointIsInsideTriangle(vec2(0.455, 0.455), vec2(0.255, 0.655), vec2(0.23, 0.63), pos);
	bool hammerHead = 
		PointIsInsideTriangle(vec2(0.2, 0.60), vec2(0.27, 0.67), vec2(0.17, 0.63), pos) ||
		PointIsInsideTriangle(vec2(0.17, 0.63), vec2(0.27, 0.67), vec2(0.24, 0.70), pos) ||
		PointIsInsideTriangle(vec2(0.24, 0.70), vec2(0.27, 0.67), vec2(0.29, 0.69), pos);

	bool sickleHandle = 
		PointIsInsideTriangle(vec2(0.19, 0.43), vec2(0.26, 0.50), vec2(0.17, 0.45), pos) ||
		PointIsInsideTriangle(vec2(0.17, 0.45), vec2(0.26, 0.50), vec2(0.24, 0.52), pos);
	bool sickleBlade = 
		PointIsInCircle(vec2(0.30, 0.59), 0.14, pos) &&
		!PointIsInCircle(vec2(0.27, 0.62), 0.13, pos) &&
		!PointIsInCircle(vec2(0.20, 0.55), 0.07, pos);
		

	if (starOuter && !starInner || 
		hammerHandle || 
		hammerHead || 
		sickleHandle ||
		sickleBlade)
	{
		gl_FragColor = vec4(GOLD, 1.0);	
	}
	else
	{
		gl_FragColor = vec4(RED, 1.0);	
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

private:
	int m_width{};
	int m_height{};
	Program m_program;
};


void RunLW7Task2()
{
	USSRFlagApp app;
	
	
	try
	{
		app.MainLoop();
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << "\n";
	}
}
