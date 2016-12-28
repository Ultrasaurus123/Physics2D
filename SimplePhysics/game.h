#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <SimpleMath.h>
#include <vector>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "rigidBody.h"
#include "gameObject.h"

struct Contact
{
	Contact(RigidBody &a, RigidBody &b) : bodyA(a), bodyB(b) {}
	Vector2 normal;
	float penDepth;
	RigidBody &bodyA, &bodyB;
};

class Game
{
public:
	Game();
	virtual ~Game();

	bool Init(HWND window);
	bool Update(float dt);
	void Draw();
	void GetContacts();
	void SolveContacts();
	void CircleCircleCollision(RigidBody &circleA, RigidBody &circleB);
	void CircleBoxCollision(RigidBody &circle, RigidBody &box);
	void BoxBoxCollision(RigidBody &boxA, RigidBody &boxB);
	void ProjectBoxOnAxis(const Vector2 *corners, const Vector2 &axis, float *minVal, float *maxVal);

private:
	Microsoft::WRL::ComPtr<ID3D11Device> _device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srvCircle;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srvGround;
	Microsoft::WRL::ComPtr<ID3D11BlendState> _blendState;
	std::shared_ptr<DirectX::SpriteBatch> _spriteBatch;

	std::vector<std::shared_ptr<GameObject>> _gameObjects;
	std::vector<Contact> _contacts;
};