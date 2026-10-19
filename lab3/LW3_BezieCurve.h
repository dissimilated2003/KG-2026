#pragma once

#include "Utils.h"


class BezieCurve final
{
public:
	BezieCurve()
	{
		m_points = {
			glm::vec2(-0.8f, -0.5f),
			glm::vec2(-0.6f, 0.7f),
			glm::vec2(0.6f, 0.7f),
			glm::vec2(0.8f, -0.5f)
		};
	}

	void Draw() const
	{
		DrawControlPolygon();
		DrawControlPoints();
		DrawCurve();
	}

	void SetViewportSize(int width, int height, float left, float right, float bottom, float top)
	{
		m_width = width;
		m_height = height;
		m_left = left;
		m_right = right;
		m_bottom = bottom;
		m_top = top;
	}

	bool StartDragging(int mouseX, int mouseY)
	{
		glm::vec2 cursorPos = NormalizeCoords(mouseX, mouseY);
		const float RADIUS = 0.06f;
		for (size_t i = 0; i < m_points.size(); ++i)
		{
			if (glm::length(m_points[i] - cursorPos) < RADIUS)
			{
				m_draggedPoint = i;
				return true;
			}
		}

		return false;
	}

	void DragTo(int mouseX, int mouseY)
	{
		if (m_draggedPoint >= 0)
		{
			m_points[m_draggedPoint] = NormalizeCoords(mouseX, mouseY);
		}
	}

	void StopDragging()
	{
		m_draggedPoint = -1;
	}

private:
	std::vector<glm::vec2> m_points{};
	
	int m_draggedPoint = -1;
	int m_width = 1;
	int m_height = 1;

	float m_left = -1.0f;
	float m_right = 1.0f;
	float m_bottom = -1.0f;
	float m_top = 1.0f;

	glm::vec2 GetPoint(float t) const // метод линейной интерполяции (де Кастельжо) x*(1-t) + y*(t)
	{
		const glm::vec2& p0 = m_points[0];
		const glm::vec2& p1 = m_points[1];
		const glm::vec2& p2 = m_points[2];
		const glm::vec2& p3 = m_points[3];

		glm::vec2 p01 = glm::mix(p0, p1, t);
		glm::vec2 p12 = glm::mix(p1, p2, t);
		glm::vec2 p23 = glm::mix(p2, p3, t);

		glm::vec2 p012 = glm::mix(p01, p12, t);
		glm::vec2 p123 = glm::mix(p12, p23, t);

		return glm::mix(p012, p123, t);
	}

	glm::vec2 NormalizeCoords(int x, int y) const
	{
		float nx = m_left + static_cast<float>(x) / static_cast<float>(m_width) * (m_right - m_left);
		float ny = m_top - static_cast<float>(y) / static_cast<float>(m_height) * (m_top - m_bottom);

		return glm::vec2(nx, ny);
	}

	void DrawCurve() const
	{
		glColor3f(0.15f, 0.15f, 0.15f);
		glLineWidth(3.0f);

		glBegin(GL_LINE_STRIP);

		const float SEGMENTS = 100;
		for (int i = 0; i <= SEGMENTS; ++i)
		{
			float step = i / SEGMENTS;
			glm::vec2 p = GetPoint(step);
			glVertex2f(p.x, p.y);
		}

		glEnd();
		glLineWidth(1.0f);
	}

	void DrawControlPolygon() const
	{
		glColor3f(0.55f, 0.55f, 0.55f);
		glLineWidth(1.0f);

		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xff00);

		glBegin(GL_LINE_STRIP);
		for (const glm::vec2& p : m_points)
		{
			glVertex2f(p.x, p.y);
		}

		glEnd();
		glDisable(GL_LINE_STIPPLE);
	}

	void DrawControlPoints() const
	{
		glPointSize(8.0f);
		glColor3f(0.5f, 0.7f, 1.0f);

		glBegin(GL_POINTS);
		for (const glm::vec2& p : m_points)
		{
			glVertex2f(p.x, p.y);
		}

		glEnd();
		glPointSize(1.0f);
	}
};


class BezieApplication : public GLApplication
{
public:
	BezieApplication() : GLApplication("Bezie", 800, 600, true, false) {}

	~BezieApplication() {}

protected:
	void OnInit() override
	{
		glClearColor(1, 1, 1, 1);
		glDisable(GL_DEPTH_TEST);
	}

	void OnDisplay() override
	{
		glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		m_curve.Draw();
	}

	void OnReshape(int width, int height) override
	{
		glViewport(0, 0, width, height);
		float aspect = static_cast<float>(width) / static_cast<float>(height);
		float left = -1.0f;
		float right = 1.0f;
		float bottom = -1.0f;
		float top = 1.0f;

		if (aspect > 1.0f)
		{
			left = -aspect;
			right = aspect;
		}
		else
		{
			bottom = -1.0f / aspect;
			top = 1.0f / aspect;
		}

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(left, right, bottom, top);
		glMatrixMode(GL_MODELVIEW);

		m_curve.SetViewportSize(width, height, left, right, bottom, top);
	}

	void OnMouse(int button, int state, int x, int y) override
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			if (state == GLUT_DOWN)
			{
				m_curve.StartDragging(x, y);
			}
			else if (state == GLUT_UP)
			{
				m_curve.StopDragging();
			}

			PostRedisplay();
		}
	}

	void OnMotion(int x, int y) override
	{
		m_curve.DragTo(x, y);
		PostRedisplay();
	}

private:
	BezieCurve m_curve;
};


int RunTask1()
{
	BezieApplication app{};
	glewInit();
	
	try
	{
		app.MainLoop();
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << "\n";
	}

	return EXIT_SUCCESS;
}
