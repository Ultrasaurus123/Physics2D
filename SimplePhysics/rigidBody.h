#pragma once
#include <SimpleMath.h>;

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
	RigidBody() : 
		shape(nullptr), 
		invMass(0.f), 
		restitution(0.f) 
	{
	}

	virtual ~RigidBody() 
	{
		delete shape;
	}

	Shape *shape;
	Vector2 position;
	Vector2 velocity;
	Vector2 acceleration;
	float invMass;
	float restitution;
};