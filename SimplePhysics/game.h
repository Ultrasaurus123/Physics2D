#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <SimpleMath.h>
#include <vector>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include "rigidBody.h"

struct Contact
{
	Vector2 normal;
	float penDepth;
	std::shared_ptr<RigidBody> bodyA, bodyB;
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
	void CirclePlaneCollision(const std::shared_ptr<RigidBody> &circle, const Vector2 &planeNormal, float planeDepth);
	void CircleCircleCollision(const std::shared_ptr<RigidBody> &circleA, const std::shared_ptr<RigidBody> &circleB);
	void CircleBoxCollision(const std::shared_ptr<RigidBody> &circle, const std::shared_ptr<RigidBody> &box);
	void BoxBoxCollision(const std::shared_ptr<RigidBody> &boxA, const std::shared_ptr<RigidBody> &boxB);

private:
	Microsoft::WRL::ComPtr<ID3D11Device> _device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> _context;
	Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srvCircle;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srvGround;
	Microsoft::WRL::ComPtr<ID3D11BlendState> _blendState;
	std::shared_ptr<DirectX::SpriteBatch> _spriteBatch;

	std::vector<std::shared_ptr<RigidBody>> _rigidBodies;
	std::vector<Contact> _contacts;
};