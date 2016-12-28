#pragma once
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

enum class ShapeType
{
	Circle,
	Box
};

struct Shape 
{
	const ShapeType type;

protected:
	Shape(ShapeType t) : type(t) {};
};

struct CircleShape : public Shape
{
	CircleShape(float r) : Shape(ShapeType::Circle), radius(r) {}
	float radius;
};

struct BoxShape : public Shape
{
	BoxShape(const Vector2 &hW) : Shape(ShapeType::Box), halfWidths(hW) {}
	Vector2 halfWidths;
};

struct RigidBody 
{
	virtual ~RigidBody() 
	{
		delete shape;
	}

	Shape *shape = nullptr;
	Vector2 position;
	Vector2 velocity;
	Vector2 acceleration;
	float invMass = 0.f;
	float restitution = 0.f;
	float rotation = 0.f;
	float angVelocity = 0.f;
	float angAcceleration = 0.f;
};