#pragma once

#include "box2d/box2d.h"
#include <SFML/Graphics.hpp>

#include <cmath>
#include <iostream>
#include <cstdint>
#include <functional>
#include <vector>
#include <memory>


constexpr float PIXELS_PER_METER = 30.0f;
constexpr float PI = 3.14159265f;
constexpr uint64_t CAT_BALL = 0x0001;
constexpr uint64_t CAT_WORLD = 0x0002;


inline b2Vec2 ToB2Vec2(sf::Vector2f v)
{
	return { v.x / PIXELS_PER_METER, -v.y / PIXELS_PER_METER };
}

inline sf::Vector2f ToSFMLVec2(b2Vec2 v)
{
	return { v.x * PIXELS_PER_METER, -v.y * PIXELS_PER_METER };
}

inline float ToB2AngleDeg(float deg)
{
	return -deg * PI / 180.0f;
}

inline float ToSFMLAngleDeg(float rad)
{
	return -rad * 180.0f / PI;
}


// класс отвечающий за физику мира
class PhysicsWorld
{
public:
	PhysicsWorld()
	{
		b2WorldDef def = b2DefaultWorldDef();
		def.gravity = { 0.0f, -9.8f };
		def.hitEventThreshold = 0.1f;
		m_worldId = b2CreateWorld(&def);
	}

	~PhysicsWorld()
	{
		b2DestroyWorld(m_worldId);
	}

	b2WorldId GetId() const
	{
		return m_worldId;
	}

	void Step()
	{
		b2World_Step(m_worldId, 1.0f / 60.0f, 4);
	}

	void ProcessCollisions()
	{
		b2ContactEvents events = b2World_GetContactEvents(m_worldId);
		for (int i = 0; i < events.hitCount; ++i)
		{
			const b2ContactHitEvent& hit = events.hitEvents[i];
			b2BodyId bodyA = b2Shape_GetBody(hit.shapeIdA);
			b2BodyId bodyB = b2Shape_GetBody(hit.shapeIdB);

			b2BodyId ball = b2_nullBodyId;
			if (IsBall(bodyA))
			{
				ball = bodyA;
			}
			else if (IsBall(bodyB))
			{
				ball = bodyB;
			}

			if (B2_IS_NULL(ball))
			{
				continue;
			}

			b2Vec2 v = b2Body_GetLinearVelocity(ball);
			v.x *= 0.8f;
			v.y *= 0.8f;
			b2Body_SetLinearVelocity(ball, v);
		}
	}

	
private:
	static bool IsBall(b2BodyId body)
	{
		return reinterpret_cast<uintptr_t>(b2Body_GetUserData(body)) == 1;
	}

	b2WorldId m_worldId{};
};


// арена с физическими стенами, в рамках которых происходят действия
// задаёт границы 
class Arena
{
public:
	Arena(
		b2WorldId world,
		sf::FloatRect rect,
		float wallThicknessPx = 20.0f
	)	
		: m_world(world)
		, m_rect(rect)
		, m_wallThickness(wallThicknessPx) 
	{
		CreateWalls();
	}

	~Arena()
	{
		DestroyWalls();
	}

	void Resize(const sf::FloatRect& newRect)
	{
		DestroyWalls();
		m_rect = newRect;
		CreateWalls();
	}

	const sf::FloatRect& GetRect() const 
	{ 
		return m_rect; 
	}

	bool Contains(sf::Vector2f p) const 
	{ 
		return m_rect.contains(p); 
	}

	void Draw(sf::RenderTarget& rt) const
	{
		auto drawWall = [&](sf::Vector2f pos, sf::Vector2f size)
			{
				sf::RectangleShape r{ size };
				r.setPosition(pos);
				r.setFillColor(sf::Color{ 70, 130, 200 });
				rt.draw(r);
			};

		float thickness = m_wallThickness;
		drawWall({ m_rect.left, m_rect.top }, { m_rect.width, thickness });
		//drawWall({ m_rect.left, m_rect.top + m_rect.height - thickness }, { m_rect.width, thickness });
		drawWall({ m_rect.left, m_rect.top }, { thickness, m_rect.height });
		drawWall({ m_rect.left + m_rect.width - thickness, m_rect.top }, { thickness, m_rect.height });
	}


private:
	b2WorldId m_world{};
	sf::FloatRect m_rect{};
	float m_wallThickness{};
	std::vector<b2BodyId> m_walls{};

	void CreateWall(sf::Vector2f center, sf::Vector2f sizePx)
	{
		b2BodyDef def = b2DefaultBodyDef();
		def.type = b2_staticBody;
		def.position = ToB2Vec2(center);
		b2BodyId body = b2CreateBody(m_world, &def);

		b2Polygon box = b2MakeBox(sizePx.x / 2.0f / PIXELS_PER_METER,
			sizePx.y / 2.0f / PIXELS_PER_METER);

		b2ShapeDef shdef = b2DefaultShapeDef();
		shdef.material.restitution = 1.0f;
		shdef.material.friction = 0.1f;
		shdef.enableHitEvents = true;
		shdef.filter.categoryBits = CAT_WORLD;
		shdef.filter.maskBits = CAT_BALL;
		
		b2CreatePolygonShape(body, &shdef, &box);
		b2Body_SetUserData(body, reinterpret_cast<void*>(uintptr_t(3)));
		m_walls.push_back(body);
	}

	void CreateWalls()
	{
		auto t = m_wallThickness;
		// верх  - лево - право
		CreateWall({ m_rect.left + m_rect.width / 2.f, m_rect.top + t / 2.f },
			{ m_rect.width, t });
		/*CreateWall({ m_rect.left + m_rect.width / 2.f, m_rect.top + m_rect.height - t / 2.f },
			{ m_rect.width, t });*/
		CreateWall({ m_rect.left + t / 2.f, m_rect.top + m_rect.height / 2.f },
			{ t, m_rect.height });
		CreateWall({ m_rect.left + m_rect.width - t / 2.f, m_rect.top + m_rect.height / 2.f },
			{ t, m_rect.height });
	}
	
	void DestroyWalls()
	{
		for (b2BodyId b : m_walls)
		{
			if (b2Body_IsValid(b))
			{
				b2DestroyBody(b);
			}
		}
		m_walls.clear();
	}
};


//выстреливаемый шарик
class Ball
{
public:
	Ball(b2WorldId world,
		sf::Vector2f pos,
		sf::Vector2f velocity,
		float radius,
		sf::Color color
	)
		: m_radius(radius)
		, m_color(color)
	{
		b2BodyDef def = b2DefaultBodyDef();
		def.type = b2_dynamicBody;
		def.position = ToB2Vec2(pos);
		def.linearVelocity = ToB2Vec2(velocity);
		def.isBullet = true;
		def.motionLocks.angularZ = false;
		m_body = b2CreateBody(world, &def);

		b2Circle circle{ {0.0f, 0.0f}, radius / PIXELS_PER_METER };
		b2ShapeDef shdef = b2DefaultShapeDef();
		shdef.material.restitution = 1.0f;
		shdef.material.friction = 0.1f;
		shdef.density = 1.0f;
		shdef.enableHitEvents = true;
		shdef.filter.categoryBits = CAT_BALL;
		shdef.filter.maskBits = CAT_WORLD | CAT_BALL;

		b2CreateCircleShape(m_body, &shdef, &circle);
		b2Body_SetUserData(m_body, reinterpret_cast<void*>(uintptr_t(1)));
	}

	void Sync()
	{
		m_position = ToSFMLVec2(b2Body_GetPosition(m_body));
		m_angle = b2Rot_GetAngle(b2Body_GetRotation(m_body));
	}

	void Draw(sf::RenderTarget& rt) const
	{
		sf::CircleShape c(m_radius);
		c.setOrigin(m_radius, m_radius);
		c.setPosition(m_position);
		c.setFillColor(m_color);
		c.setOutlineColor(sf::Color::Black);
		c.setOutlineThickness(1.f);
		rt.draw(c);
	}

	b2BodyId GetBody() const 
	{ 
		return m_body; 
	}

	sf::Vector2f GetPosition() const 
	{ 
		return m_position; 
	}

	float GetRadius() const 
	{ 
		return m_radius; 
	}

private:
	sf::Vector2f m_position{};
	b2BodyId m_body{};
	sf::Color m_color{};

	float m_angle = 0.0f;
	float m_radius{};
};


// класс препятствия на карте
class Obstacle
{
public:
	Obstacle(b2WorldId world,
		sf::Vector2f center,
		sf::Vector2f size,
		float angle
	)
		: m_size(size)
	{
		b2BodyDef def = b2DefaultBodyDef();
		def.type = b2_staticBody;
		def.position = ToB2Vec2(center);
		def.rotation = b2MakeRot(ToB2AngleDeg(angle));
		m_body = b2CreateBody(world, &def);

		b2Polygon box = b2MakeBox(size.x / 2.0f / PIXELS_PER_METER, size.y / 2.0f / PIXELS_PER_METER);
		b2ShapeDef shdef = b2DefaultShapeDef();
		shdef.material.restitution = 1.0f;
		shdef.material.friction = 0.1f;
		shdef.enableHitEvents = true;
		shdef.filter.categoryBits = CAT_WORLD;
		shdef.filter.maskBits = CAT_BALL;
		b2CreatePolygonShape(m_body, &shdef, &box);
		b2Body_SetUserData(m_body, reinterpret_cast<void*>(uintptr_t(2)));
	}

	void Sync()
	{
		m_position = ToSFMLVec2(b2Body_GetPosition(m_body));
		m_angle = ToSFMLAngleDeg(b2Rot_GetAngle(b2Body_GetRotation(m_body)));
	}

	void Draw(sf::RenderTarget& rt) const
	{
		sf::RectangleShape r(m_size);
		r.setOrigin(m_size.x / 2.f, m_size.y / 2.f);
		r.setPosition(m_position);
		r.setRotation(m_angle);
		r.setFillColor(sf::Color(70, 130, 200));
		r.setOutlineColor(sf::Color(30, 80, 150));
		r.setOutlineThickness(2.f);
		rt.draw(r);
	}

	b2BodyId GetBody() const
	{
		return m_body;
	}

private:
	b2BodyId m_body{};
	sf::Vector2f m_position{};
	sf::Vector2f m_size{};

	float m_angle = 0.0f;
};


//пушка как декор
// текущая папка C:/Users/Андрей/Desktop/BACKUP/КГ/lab2/laba2/
class Cannon
{
public:
	Cannon(sf::Vector2f pos, 
		float barrelLength = 40.0f	
	)
		: m_position(pos)
		, m_barrelLength(barrelLength)
	{}

	void SetPosition(sf::Vector2f pos) 
	{ 
		m_position = pos; 
	}

	sf::Vector2f GetDirection() const
	{
		float rad = m_angleDeg * 3.14159265f / 180.f;
		return { std::cos(rad), std::sin(rad) };
	}

	sf::Vector2f GetMuzzlePosition() const
	{
		return m_position + GetDirection() * m_barrelLength;
	}

	void AimAt(sf::Vector2f target)
	{
		sf::Vector2f d = target - m_position;
		m_angleDeg = std::atan2(d.y, d.x) * 180.f / 3.14159265f;
	}

	void Draw(sf::RenderTarget& rt) const
	{
		sf::CircleShape base(12.f);
		base.setOrigin(12.f, 12.f);
		base.setPosition(m_position);
		base.setFillColor(sf::Color(200, 130, 130));
		base.setOutlineColor(sf::Color(120, 70, 70));
		base.setOutlineThickness(2.f);
		rt.draw(base);

		sf::RectangleShape barrel({ m_barrelLength, 8.f });
		barrel.setOrigin(0.f, 4.f);
		barrel.setPosition(m_position);
		barrel.setRotation(m_angleDeg);
		barrel.setFillColor(sf::Color(180, 100, 100));
		rt.draw(barrel);
	}

	sf::Vector2f GetPosition() const 
	{ 
		return m_position; 
	}

private:
	sf::Vector2f m_position;
	float m_barrelLength;
	float m_angleDeg = 0.f;

	
};


//класс для управления жизнью шариков
class BallManager
{
public:
	BallManager(b2WorldId world) : m_world(world) {}

	bool CanShoot() const
	{
		return GetCount() < MAX_BALLS;
	}

	void Shoot(sf::Vector2f pos, 
		sf::Vector2f velocity, 
		float radius = 10.f, 
		sf::Color color = sf::Color(220, 80, 80))
	{
		if (!CanShoot())
		{
			return;
		}
		m_balls.push_back(std::make_unique<Ball>(m_world, pos, velocity, radius, color));
	}

	void Update(const sf::FloatRect& arena)
	{
		for (auto& b : m_balls)
		{
			b->Sync();
		}
		for (auto it = m_balls.begin(); it != m_balls.end();)
		{
			sf::Vector2f pos = (*it)->GetPosition();
			if (pos.y > arena.top + arena.height + 50.0f)
			{
				b2BodyId body = (*it)->GetBody();
				if (b2Body_IsValid(body))
				{
					b2DestroyBody(body);
				}
				it = m_balls.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void Draw(sf::RenderTarget& rt) const
	{
		for (const auto& ball : m_balls)
		{
			ball->Draw(rt);
		}
	}

	int GetCount() const 
	{ 
		return static_cast<int>(m_balls.size()); 
	}

private:
	sf::Clock m_goneSoundCooldown;
	std::vector<std::unique_ptr<Ball>> m_balls{};
	static constexpr unsigned MAX_BALLS = 50;

	b2WorldId m_world{};
};


//композиция, класс игры
class ExtremeBallGame
{
public:
	ExtremeBallGame()
		: m_window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Extreme balls", sf::Style::Default)
		, m_arena(
			m_world.GetId(), 
			sf::FloatRect{0.0f, 0.0f, static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)}
		)
		, m_cannon({ 500.f, 350.f })
		, m_ballManager(m_world.GetId())
	{
		m_window.setVerticalSyncEnabled(false);
		InitializeObstacles();
	}

	void RunGame()
	{
		sf::Clock clock{};
		while (m_window.isOpen())
		{
			sf::Event ev;
			while (m_window.pollEvent(ev))
			{
				HandleEvent(ev);
			}

			float dt = clock.restart().asSeconds();
			m_accumulator += dt;
			while (m_accumulator >= 1.0f / 60.0f)
			{
				m_world.Step();
				m_world.ProcessCollisions();
				m_accumulator -= 1.0f / 60.0f;
			}

			for (auto& o : m_obstacles)
			{
				o->Sync();
			}
			m_ballManager.Update(m_arena.GetRect());
			Render();
		}
	}

private:
	float m_accumulator = 0.f;
	const unsigned WINDOW_WIDTH = 1000;
	const unsigned WINDOW_HEIGHT = 700;

	sf::RenderWindow m_window;
	PhysicsWorld m_world;
	Arena m_arena;
	Cannon m_cannon;
	BallManager m_ballManager;
	std::vector<std::unique_ptr<Obstacle>> m_obstacles;


	void HandleEvent(const sf::Event& ev)
	{
		if (ev.type == sf::Event::Closed)
		{
			m_window.close();
		}

		if (ev.type == sf::Event::Resized)
		{
			m_window.setView(sf::View(sf::FloatRect{ 0.0f, 0.0f,
				static_cast<float>(ev.size.width), static_cast<float>(ev.size.height) }));
		}

		if (ev.type == sf::Event::MouseMoved)
		{
			m_cannon.AimAt(sf::Vector2f{
				static_cast<float>(ev.mouseMove.x),
				static_cast<float>(ev.mouseMove.y)
			});
		}

		if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left)
		{
			sf::Vector2f mousePos{
				static_cast<float>(ev.mouseButton.x),
				static_cast<float>(ev.mouseButton.y)
			};
			if (/*m_arena.Contains(mousePos) && */m_ballManager.CanShoot())
			{
				const float BALL_SPEED = 600.0f;
				m_ballManager.Shoot(
					m_cannon.GetMuzzlePosition(),
					m_cannon.GetDirection() * BALL_SPEED,
					10.0f
				);
			}
		}
	}

	void InitializeObstacles()
	{
		m_obstacles.push_back(std::make_unique<Obstacle>(
			m_world.GetId(),
			sf::Vector2f{ 300.0f, 200.0f },
			sf::Vector2f{ 100.0f, 100.0f },
			45.0f)); 
		m_obstacles.push_back(std::make_unique<Obstacle>(
			m_world.GetId(),
			sf::Vector2f{ 500.0f, 430.0f },
			sf::Vector2f{ 130.0f, 60.0f },
			-30.f));
		m_obstacles.push_back(std::make_unique<Obstacle>(
			m_world.GetId(),
			sf::Vector2f{ 780.0f, 180.0f },
			sf::Vector2f{ 80.0f, 80.0f },
			0.0f));
		//2 палки снизу
		m_obstacles.push_back(std::make_unique<Obstacle>(
			m_world.GetId(),
			sf::Vector2f{ 150.f, 620.f },
			sf::Vector2f{ 600.f, 20.f },
			15.f));
		m_obstacles.push_back(std::make_unique<Obstacle>(
			m_world.GetId(),
			sf::Vector2f{ 800.f, 480.f },
			sf::Vector2f{ 700.f, 20.f },
			-15.f));
	}

	void Render()
	{
		m_window.clear(sf::Color::White);
		m_arena.Draw(m_window);
		for (auto& o : m_obstacles) o->Draw(m_window);
		m_ballManager.Draw(m_window);
		m_cannon.Draw(m_window);
		m_window.display();
	}
};


void RunExtraTask()
{
	ExtremeBallGame game{};
	game.RunGame();
}
