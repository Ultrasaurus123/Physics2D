#pragma once
#include <memory>
#include <d3d11.h>
#include <wrl.h>
#include <SpriteBatch.h>
#include "rigidBody.h"


class GameObject
{
public:
	GameObject(const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> &srv, float invMass, float restitution, float radius);
	GameObject(const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> &srv, float invMass, float restitution, const Vector2 &halfWidths);
	virtual ~GameObject();

	RigidBody &GetBody() { return _body; }
	const RigidBody &GetBody() const { return _body; }
	void Draw(const std::shared_ptr<DirectX::SpriteBatch> &spriteBatch);
private:
	RigidBody _body;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv;
	Vector2 _origin;
	Vector2 _scale;
};