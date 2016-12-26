#include <Windows.h>
#include <SpriteBatch.h>
#include <assert.h>
#include <time.h>
#include <WICTextureLoader.h>
#include "Game.h"

using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;
using namespace DirectX;

#define CHECKHR(x) if (FAILED(x)) { assert(false); return false; }

Game::Game()
{
}

Game::~Game()
{
	CoUninitialize();
}

bool Game::Init(HWND window)
{
	UINT flags = 0;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapDesc{};
	HRESULT hr = S_OK;

#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // DEBUG
	hr = CoInitialize(nullptr);
	CHECKHR(hr);

	RECT clientArea = {};
	GetClientRect(window, &clientArea);

	swapDesc.BufferCount = 2;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.Width = clientArea.right - clientArea.left;
	swapDesc.BufferDesc.Height = clientArea.bottom - clientArea.top;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = window;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.Windowed = TRUE;

	hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
		&featureLevel, 1, D3D11_SDK_VERSION, &swapDesc, &_swapChain, &_device, nullptr, &_context);
	CHECKHR(hr);

	ComPtr<ID3D11Texture2D> backbuffer;
	hr = _swapChain->GetBuffer(0, IID_PPV_ARGS(&backbuffer));
	CHECKHR(hr);

	hr = _device->CreateRenderTargetView(backbuffer.Get(), nullptr, &_backBuffer);
	CHECKHR(hr);
	_context->OMSetRenderTargets(1, _backBuffer.GetAddressOf(), nullptr);

	D3D11_VIEWPORT viewport{};
	viewport.Width = float(swapDesc.BufferDesc.Width);
	viewport.Height = float(swapDesc.BufferDesc.Height);
	viewport.MaxDepth = 1.f;
	_context->RSSetViewports(1, &viewport);

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = _device->CreateBlendState(&blendDesc, &_blendState);
	CHECKHR(hr);

	ComPtr<ID3D11Resource> image;
	hr = DirectX::CreateWICTextureFromFile(_device.Get(), L"round.png", image.ReleaseAndGetAddressOf(), &_srvCircle);
	CHECKHR(hr);
	hr = DirectX::CreateWICTextureFromFile(_device.Get(), L"ground.jpg", image.ReleaseAndGetAddressOf(), &_srvGround);
	CHECKHR(hr);

	//Sprite Batch / Sprite Font
	_spriteBatch.reset(new DirectX::SpriteBatch(_context.Get()));
	_spriteBatch->SetViewport(viewport);

	srand(time(nullptr));

	//add default rigid bodies
	for (int i = 0; i < 30; i++)
	{
		std::shared_ptr<RigidBody> rBody = std::make_shared<RigidBody>();
		if (rand() % 2 == 0)
		{
			rBody->shape = new CircleShape(rand() % 16 + 16.f);
		}
		else
		{
			rBody->shape = new BoxShape(Vector2(rand() % 32 + 16.f, rand() % 32 + 16.f));
		}
		rBody->position = Vector2(rand() % 1150 + 50, 700);
		rBody->invMass = 1 / (rand() % 100 + 20.f);
		rBody->restitution = 1.f;
		rBody->velocity = Vector2(rand() % 150, rand() % 100);
		_rigidBodies.push_back(rBody);
	}
	std::shared_ptr<RigidBody> rBody = std::make_shared<RigidBody>();
	rBody->shape = new BoxShape(Vector2(640.f, 50.f));
	rBody->position = Vector2(640.f, 50.f);
	_rigidBodies.push_back(rBody);

	rBody = std::make_shared<RigidBody>();
	rBody->shape = new BoxShape(Vector2(640.f, 50.f));
	rBody->position = Vector2(640.f, 1230.f);
	_rigidBodies.push_back(rBody);

	rBody = std::make_shared<RigidBody>();
	rBody->shape = new BoxShape(Vector2(50.f, 640.f));
	rBody->position = Vector2(-50.f, 640.f);
	_rigidBodies.push_back(rBody);

	rBody = std::make_shared<RigidBody>();
	rBody->shape = new BoxShape(Vector2(50.f, 640.f));
	rBody->position = Vector2(1330.f, 640.f);
	_rigidBodies.push_back(rBody);

	return true;
}
bool Game::Update(float dt)
{
	if (_rigidBodies.size() > 0)
	{
		auto &body = _rigidBodies[0];
		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			body->velocity += Vector2(-50.f, 0);
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			body->velocity += Vector2(50.f, 0);
		}
		if (GetAsyncKeyState(VK_UP) & 0x8000)
		{
			body->velocity += Vector2(0, 50.f);
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			body->velocity += Vector2(0, -50.f);
		}
	}

	for (int i = 0; i < _rigidBodies.size(); i++)
	{
		std::shared_ptr<RigidBody> &body = _rigidBodies[i];

		if (body->invMass > 0.f)
		{
			Vector2 totalAcc = body->acceleration + Vector2(0, -98.f);
			body->velocity += totalAcc * dt;
			body->position += body->velocity * dt;
		}
	}

	GetContacts();
	SolveContacts();

	return true;
}

void Game::Draw()
{
	const float background[4] = { 0, 0, 0.5, 1 };
	_context->ClearRenderTargetView(_backBuffer.Get(), background);

	_spriteBatch->Begin(DirectX::SpriteSortMode_Deferred, _blendState.Get());
	for (int i = 0; i < _rigidBodies.size(); i++)
	{
		std::shared_ptr<RigidBody> &body = _rigidBodies[i];
		if (body->shape->type == ShapeType::Circle)
		{
			Vector2 position = body->position;
			position.y = 720 - position.y;
			float radius = ((CircleShape*)body->shape)->radius;
			_spriteBatch->Draw(_srvCircle.Get(), position - Vector2(radius, radius), nullptr, Colors::White, 0, Vector2::Zero, Vector2(radius/32.f, radius/32.f));
		}
		else if (body->shape->type == ShapeType::Box)
		{
			Vector2 position = body->position;
			position.y = 720 - position.y;
			Vector2 halfWidths = ((BoxShape*)body->shape)->halfWidths;
			RECT destRect = { position.x - halfWidths.x, position.y - halfWidths.y, position.x + halfWidths.x, position.y + halfWidths.y };
			_spriteBatch->Draw(_srvGround.Get(), destRect);
		}
	}
	_spriteBatch->End();

	_swapChain->Present(1, 0);
}

void Game::GetContacts()
{
	_contacts.clear();
	Vector2 groundNormal = Vector2(0, 1.0);
	float groundDepth = 70.f;

	for (int i = 0; i < _rigidBodies.size(); i++)
	{
		std::shared_ptr<RigidBody> &body = _rigidBodies[i];
		if (body->shape->type == ShapeType::Circle)
		{
			for (int j = i + 1; j < _rigidBodies.size(); j++)
			{
				if (_rigidBodies[j]->shape->type == ShapeType::Circle)
				{
					CircleCircleCollision(body, _rigidBodies[j]);
				}
				else if (_rigidBodies[j]->shape->type == ShapeType::Box)
				{
					CircleBoxCollision(body, _rigidBodies[j]);
				}
			}
		}
		else if (body->shape->type == ShapeType::Box)
		{
			for (int j = i + 1; j < _rigidBodies.size(); j++)
			{
				if (_rigidBodies[j]->shape->type == ShapeType::Circle)
				{
					CircleBoxCollision(_rigidBodies[j], body);
				}
				else if (_rigidBodies[j]->shape->type == ShapeType::Box)
				{
					BoxBoxCollision(body, _rigidBodies[j]);
				}
			}
		}

	}
}

void Game::SolveContacts()
{
	for (int i = 0; i < _contacts.size(); i++)
	{
		Contact &contact = _contacts[i];

		float totalInvMass = contact.bodyB->invMass;
		if (contact.bodyA != nullptr)
		{
			totalInvMass += contact.bodyA->invMass;
		}

		if (totalInvMass < 0.0001f)
		{
			continue;
		}

		Vector2 relVelocity = contact.bodyB->velocity;
		if (contact.bodyA != nullptr)
		{
			relVelocity -= contact.bodyA->velocity;
		}
		float incoming = relVelocity.Dot(contact.normal);
		if (incoming < 0)
		{
			float restitution = 0.5f * contact.bodyB->restitution;
			if (contact.bodyA != nullptr)
			{
				restitution += 0.5f * contact.bodyA->restitution;
			}
			float outgoing = -incoming * restitution;
			float newSpeed = -incoming + outgoing;
			contact.bodyB->velocity += (contact.bodyB->invMass / totalInvMass) * contact.normal * newSpeed;
			if (contact.bodyA != nullptr)
			{
				contact.bodyA->velocity += (contact.bodyA->invMass / totalInvMass) * -contact.normal * newSpeed;
			}

		}
		contact.bodyB->position += (contact.bodyB->invMass / totalInvMass) * contact.normal * contact.penDepth;
		if (contact.bodyA != nullptr)
		{
			contact.bodyA->position += (contact.bodyA->invMass / totalInvMass) * -contact.normal * contact.penDepth;
		}
	}

}

void Game::CirclePlaneCollision(const std::shared_ptr<RigidBody> &circle, const Vector2 &planeNormal, float planeDepth)
{
	float dist = circle->position.Dot(planeNormal) - planeDepth;
	float radius = ((CircleShape*)circle->shape)->radius;
	if (dist <= radius)
	{
		Contact c = {};
		c.normal = planeNormal;
		c.penDepth = radius - dist;
		c.bodyA = nullptr;
		c.bodyB = circle;
		_contacts.push_back(c);
	}
}

void Game::CircleBoxCollision(const std::shared_ptr<RigidBody> &circle, const std::shared_ptr<RigidBody> &box)
{
	const Vector2 &halfWidths = ((BoxShape*)box->shape)->halfWidths;
	float radius = ((CircleShape*)circle->shape)->radius;

	Vector2 closestPoint = Vector2::Zero;
	if (circle->position.x <= box->position.x - halfWidths.x)
	{ 
		closestPoint.x = box->position.x - halfWidths.x;
	}
	else if (circle->position.x >= box->position.x + halfWidths.x)
	{
		closestPoint.x = box->position.x + halfWidths.x;
	}
	else
	{
		closestPoint.x = circle->position.x;
	}
	if (circle->position.y <= box->position.y - halfWidths.y)
	{
		closestPoint.y = box->position.y - halfWidths.y;
	}
	else if (circle->position.y >= box->position.y + halfWidths.y)
	{
		closestPoint.y = box->position.y + halfWidths.y;
	}
	else
	{
		closestPoint.y = circle->position.y;
	}

	Vector2 diff = circle->position - closestPoint;
	float dist = diff.Length();

	if (dist <= radius)
	{
		Contact c = {};
		diff.Normalize(c.normal);
		c.penDepth = radius - dist;
		c.bodyA = box;
		c.bodyB = circle;
		_contacts.push_back(c);

	}
}

void Game::CircleCircleCollision(const std::shared_ptr<RigidBody> &circleA, const std::shared_ptr<RigidBody> &circleB)
{
	Vector2 diff = circleB->position - circleA->position;
	float dist = diff.Length();
	float totalRadius = (((CircleShape*)circleA->shape)->radius + ((CircleShape*)circleB->shape)->radius);

	if (dist <= totalRadius)
	{
		Contact c = {};
		diff.Normalize(c.normal);
		c.penDepth = totalRadius - dist;
		c.bodyA = circleA;
		c.bodyB = circleB;
		_contacts.push_back(c);

	}
}

void Game::BoxBoxCollision(const std::shared_ptr<RigidBody> &boxA, const std::shared_ptr<RigidBody> &boxB)
{
	if (boxA->invMass + boxB->invMass < 0.00001f)
	{
		return;
	}
	const Vector2 &halfWidthsA = ((BoxShape*)boxA->shape)->halfWidths;
	const Vector2 &halfWidthsB = ((BoxShape*)boxB->shape)->halfWidths;
	Vector2 minNormal = Vector2(0, 1.f);
	float minDepth = (boxA->position.y + halfWidthsA.y) - (boxB->position.y - halfWidthsB.y);
	float penDepth = (boxA->position.x + halfWidthsA.x) - (boxB->position.x - halfWidthsB.x);
	if (penDepth < minDepth)
	{
		minDepth = penDepth;
		minNormal = Vector2(1.f, 0);
	}
	penDepth = (boxB->position.y + halfWidthsB.y) - (boxA->position.y - halfWidthsA.y);
	if (penDepth < minDepth)
	{
		minDepth = penDepth;
		minNormal = Vector2(0, -1.f);
	}
	penDepth = (boxB->position.x + halfWidthsB.x) - (boxA->position.x - halfWidthsA.x);
	if (penDepth < minDepth)
	{
		minDepth = penDepth;
		minNormal = Vector2(-1.f, 0);
	}
	if (minDepth < 0)
	{
		return;
	}

	Contact c = {};
	c.normal = minNormal;
	c.penDepth = minDepth;
	c.bodyA = boxA;
	c.bodyB = boxB;
	_contacts.push_back(c);
}