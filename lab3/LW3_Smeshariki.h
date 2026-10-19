#pragma once

#include "Utils.h"

extern constexpr float PI = 3.1415926f;


class Smesharik final
{
public:
	Smesharik() {}

	void Draw() const
	{
		glPushMatrix();

			glTranslatef(m_position.x, m_position.y, 0.0f);
			glScalef(m_scale, m_scale, 1.0f);

			DrawBackParts();
			DrawBody();
			DrawFace();

		glPopMatrix();
	}

	void SetViewport(int width, int height, float left, float right, float bottom, float top)
	{
		m_width = std::max(width, 1);
		m_height = std::max(height, 1);
		m_left = left;
		m_right = right;
		m_bottom = bottom;
		m_top = top;
		//ClampToArea();
	}

	bool HitTest(const glm::vec2& point) const
	{
		const float RADIUS = 0.5f * m_scale;
		return glm::length(point - m_position) <= RADIUS;
	}

	bool StartDragging(int x, int y)
	{
		glm::vec2 point = NormalizeCoords(x, y);
		if (!HitTest(point))
		{
			return false;
		}

		m_isDragging = true;
		m_dragOffset = m_position - point;

		return true;
	}

	void DragTo(int x, int y)
	{
		if (!m_isDragging)
		{
			return;
		}

		glm::vec2 point = NormalizeCoords(x, y);
		m_position = point + m_dragOffset;

		//ClampToArea();
	}

	void StopDragging()
	{
		m_isDragging = false;
	}

	void SetPosition(const glm::vec2& position)
	{
		m_position = position;
	}

	glm::vec2 GetPosition() const
	{
		return m_position;
	}

	void SetScale(float scale)
	{
		m_scale = scale;
	}

	float GetScale() const
	{
		return m_scale;
	}

private:
	glm::vec2 m_position{ glm::vec2{0.0f, 0.0f} };
	glm::vec2 m_dragOffset{ glm::vec2{0.0f, 0.0f} };

	bool m_isDragging{ false };

	float m_scale{ 1.0f };
	int m_width{ 1 };
	int m_height{ 1 };
	float m_left{ -1.0f };
	float m_right{ 1.0f };
	float m_bottom{ -1.0f };
	float m_top{ 1.0f };

	static constexpr float HALF_EXTENT = 0.68f;
	void ClampToArea()
	{
		// Размер Смешарика с учётом масштаба
		const float HALF = HALF_EXTENT * m_scale;

		const float AREA_LEFT = m_left;
		const float AREA_RIGHT = m_right;
		const float AREA_BOTTOM = m_bottom;
		const float AREA_TOP = m_top;

		if (HALF * 2.0f >= AREA_RIGHT - AREA_LEFT)
		{
			m_position.x = 0.5f * (AREA_LEFT + AREA_RIGHT);
		}
		else
		{
			m_position.x = std::clamp(m_position.x, AREA_LEFT + HALF, AREA_RIGHT - HALF);
		}

		if (HALF * 2.0f >= AREA_TOP - AREA_BOTTOM)
		{
			m_position.y = 0.5f * (AREA_BOTTOM + AREA_TOP);
		}
		else
		{
			m_position.y = std::clamp(m_position.y, AREA_BOTTOM + HALF, AREA_TOP - HALF);
		}
	}


	glm::vec2 NormalizeCoords(int x, int y) const
	{
		float nx = m_left + static_cast<float>(x) / static_cast<float>(m_width) * (m_right - m_left);
		float ny = m_top - static_cast<float>(y) / static_cast<float>(m_height) * (m_top - m_bottom);

		return glm::vec2(nx, ny);
	}

	void SetColor(float r, float g, float b) const
	{
		glColor3f(r, g, b);
	}

	void DrawCircle(float radius, int segments = 64) const
	{
		DrawEllipse(radius, radius, segments);
	}

	void DrawEllipse(float rx, float ry, int segments = 64) const
	{
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(0.0f, 0.0f);
		for (int i = 0; i <= segments; ++i)
		{
			const float ANGLE = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
			glVertex2f(std::cos(ANGLE) * rx, std::sin(ANGLE) * ry);
		}

		glEnd();
	}

	void DrawEllipseOutline(float rx, float ry, int segments = 64) const
	{
		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < segments; ++i)
		{
			const float ANGLE = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
			glVertex2f(std::cos(ANGLE) * rx, std::sin(ANGLE) * ry);
		}
		glEnd();
	}

	void DrawFilledQuad(
		const glm::vec2& p1,
		const glm::vec2& p2,
		const glm::vec2& p3,
		const glm::vec2& p4) const
	{
		glBegin(GL_QUADS);

		glVertex2f(p1.x, p1.y);
		glVertex2f(p2.x, p2.y);
		glVertex2f(p3.x, p3.y);
		glVertex2f(p4.x, p4.y);

		glEnd();
	}

	glm::vec2 RotatePoint(const glm::vec2& p, float degrees) const
	{
		const float rad = degrees * PI / 180.0f;
		const float c = std::cos(rad);
		const float s = std::sin(rad);
		return glm::vec2(p.x * c - p.y * s, p.x * s + p.y * c);
	}

	// эллипс с поворотом вокруг своего центра
	void DrawRotatedEllipseAt(const glm::vec2& center, float rx, float ry, 
		float angleDeg, int segments = 64) const
	{
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(center.x, center.y);
		for (int i = 0; i <= segments; ++i)
		{
			const float a = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
			glm::vec2 p(std::cos(a) * rx, std::sin(a) * ry);
			p = RotatePoint(p, angleDeg) + center;
			glVertex2f(p.x, p.y);
		}
		glEnd();
	}

	void DrawRotatedEllipseOutlineAt(const glm::vec2& center, float rx, float ry, 
		float angleDeg, int segments = 64) const
	{
		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < segments; ++i)
		{
			const float a = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
			glm::vec2 p(std::cos(a) * rx, std::sin(a) * ry);
			p = RotatePoint(p, angleDeg) + center;
			glVertex2f(p.x, p.y);
		}
		glEnd();
	}

	void DrawBackParts() const
	{
		///////////////////////////////////////// уши
		SetColor(0.35f, 0.80f, 1.0f);
		DrawRotatedEllipseAt(glm::vec2(-0.12f, 0.32f), 0.07f, 0.25f, 18.0f);
		DrawRotatedEllipseAt(glm::vec2(0.12f, 0.32f), 0.07f, 0.25f, -18.0f);

		SetColor(0.65f, 0.93f, 1.0f);
		DrawRotatedEllipseAt(glm::vec2(-0.12f, 0.33f), 0.03f, 0.16f, 18.0f);
		DrawRotatedEllipseAt(glm::vec2(0.12f, 0.33f), 0.03f, 0.16f, -18.0f);

		///////////////////////////////////////// ноги
		SetColor(0.20f, 0.62f, 0.95f);
		DrawRotatedEllipseAt(glm::vec2(-0.12f, -0.26f), 0.10f, 0.05f, -8.0f);
		DrawRotatedEllipseAt(glm::vec2(0.12f, -0.26f), 0.10f, 0.05f, 8.0f);

		///////////////////////////////////////// руки
		SetColor(0.25f, 0.70f, 0.98f);
		DrawRotatedEllipseAt(glm::vec2(-0.28f, -0.03f), 0.05f, 0.14f, -30.0f);
		DrawRotatedEllipseAt(glm::vec2(0.28f, -0.03f), 0.05f, 0.14f, 30.0f);
	}

	void DrawBody() const
	{
		SetColor(0.35f, 0.80f, 1.0f);
		DrawCircle(0.28f);

		SetColor(0.05f, 0.25f, 0.35f);
		glLineWidth(2.0f);
		DrawEllipseOutline(0.28f, 0.28f);
		glLineWidth(1.0f);
	}

	void DrawEllipseAt(const glm::vec2& center, float rx, float ry, int segments = 64) const
	{
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(center.x, center.y);
		for (int i = 0; i <= segments; ++i)
		{
			const float a = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
			glVertex2f(center.x + std::cos(a) * rx, center.y + std::sin(a) * ry);
		}
		glEnd();
	}

	
	void DrawEllipseOutlineAt(const glm::vec2& center, float rx, float ry, int segments = 64) const
	{
		glBegin(GL_LINE_LOOP);
		for (int i = 0; i < segments; ++i)
		{
			const float a = 2.0f * PI * static_cast<float>(i) / static_cast<float>(segments);
			glVertex2f(center.x + std::cos(a) * rx, center.y + std::sin(a) * ry);
		}
		glEnd();
	}


	void DrawCircleAt(const glm::vec2& center, float radius, int segments = 64) const
	{
		DrawEllipseAt(center, radius, radius, segments);
	}

	void DrawFace() const
	{
		///////////////////////////////////////// глаза
		SetColor(1.0f, 1.0f, 1.0f);
		DrawEllipseAt(glm::vec2(-0.08f, 0.09f), 0.08f, 0.10f);
		DrawEllipseAt(glm::vec2(0.08f, 0.09f), 0.08f, 0.10f);

		///////////////////////////////////////// контуры глаз
		SetColor(0.05f, 0.05f, 0.05f);
		glLineWidth(1.5f);
		DrawEllipseOutlineAt(glm::vec2(-0.08f, 0.09f), 0.08f, 0.10f);
		DrawEllipseOutlineAt(glm::vec2(0.08f, 0.09f), 0.08f, 0.10f);
		glLineWidth(1.0f);

		///////////////////////////////////////// зрачки
		SetColor(0.0f, 0.0f, 0.0f);
		DrawCircleAt(glm::vec2(-0.05f, 0.07f), 0.02f);
		DrawCircleAt(glm::vec2(0.05f, 0.07f), 0.02f);

		///////////////////////////////////////// нос
		SetColor(0.95f, 0.20f, 0.20f);
		DrawEllipseAt(glm::vec2(0.0f, 0.004f), 0.04f, 0.03f);

		SetColor(0.35f, 0.05f, 0.05f);
		glLineWidth(1.2f);
		DrawEllipseOutlineAt(glm::vec2(0.0f, 0.004f), 0.04f, 0.03f);
		glLineWidth(1.0f);

		///////////////////////////////////////// рот
		SetColor(0.35f, 0.05f, 0.08f);
		glBegin(GL_POLYGON);
		glVertex2f(-0.09f, -0.07f);
		glVertex2f(0.09f, -0.07f);
		glVertex2f(0.06f, -0.15f);
		glVertex2f(0.0f, -0.18f);
		glVertex2f(-0.06f, -0.15f);
		glEnd();

		///////////////////////////////////////// зубы
		SetColor(1.0f, 1.0f, 1.0f);
		DrawFilledQuad(
			glm::vec2(-0.042f, -0.07f),
			glm::vec2(0.0f, -0.07f),
			glm::vec2(0.0f, -0.12f),
			glm::vec2(-0.042f, -0.12f)
		);
		DrawFilledQuad(
			glm::vec2(0.0f, -0.07f),
			glm::vec2(0.042f, -0.07f),
			glm::vec2(0.042f, -0.12f),
			glm::vec2(0.0f, -0.12f)
		);

		///////////////////////////////////////// разделитель зубов
		SetColor(0.0f, 0.0f, 0.0f);
		glBegin(GL_LINES);
		glVertex2f(0.0f, -0.07f);
		glVertex2f(0.0f, -0.12f);
		glEnd();
	}
};


class SmesharikApplication final : public GLApplication
{
public:
	SmesharikApplication() : GLApplication("Smesharik", 800, 600, false, false) {}
	~SmesharikApplication() {}

protected:
	void OnInit() override
	{
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void OnDisplay() override
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		m_smesharik.Draw();
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

		m_smesharik.SetViewport(width, height, left, right, bottom, top);
	}

	void OnMouse(int button, int state, int x, int y) override
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			if (state == GLUT_DOWN)
			{
				m_smesharik.StartDragging(x, y);
			}
			else if (state == GLUT_UP)
			{
				m_smesharik.StopDragging();
			}

			PostRedisplay();
		}
	}

	void OnMotion(int x, int y) override
	{
		m_smesharik.DragTo(x, y);
		PostRedisplay();
	}

private:
	Smesharik m_smesharik{};

};


int RunTask2()
{
	SmesharikApplication app{};
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
