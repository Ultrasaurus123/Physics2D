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
	std::shared_ptr<GameObject> rBody;
	for (int i = 0; i < 30; i++)
	{
		if (rand() % 2 == 0)
		{
			rBody = std::make_shared<GameObject>(_srvCircle, 1 / (rand() % 100 + 20.f), .75f, rand() % 16 + 16.f);
		}
		else
		{
			rBody = std::make_shared<GameObject>(_srvGround, 1 / (rand() % 100 + 20.f), .75f, Vector2(rand() % 32 + 16.f, rand() % 32 + 16.f));
		}
		rBody->GetBody().rotation = (rand() % 316) / 100.f;
		rBody->GetBody().position = Vector2(rand() % 1150 + 50, 700);
		rBody->GetBody().velocity = Vector2(rand() % 150, rand() % 100);
		_gameObjects.push_back(rBody);
	}

	rBody = std::make_shared<GameObject>(_srvGround, 0.f, 0.f, Vector2(640.f, 50.f));
	rBody->GetBody().position = Vector2(640.f, 50.f);
	_gameObjects.push_back(rBody);

	rBody = std::make_shared<GameObject>(_srvGround, 0.f, 0.f, Vector2(640.f, 50.f));
	rBody->GetBody().position = Vector2(640.f, 1230.f);
	_gameObjects.push_back(rBody);
	
	rBody = std::make_shared<GameObject>(_srvGround, 0.f, 0.f, Vector2(50.f, 640.f));
	rBody->GetBody().position = Vector2(-50.f, 640.f);
	_gameObjects.push_back(rBody);
	
	rBody = std::make_shared<GameObject>(_srvGround, 0.f, 0.f, Vector2(50.f, 640.f));
	rBody->GetBody().position = Vector2(1330.f, 640.f);
	_gameObjects.push_back(rBody);

	rBody = std::make_shared<GameObject>(_srvGround, 0.f, 0.f, Vector2(200.f, 25.f));
	rBody->GetBody().position = Vector2(500.f, 400.f);
	rBody->GetBody().rotation = XMConvertToRadians(-30.f);
	_gameObjects.push_back(rBody);

	return true;
}
bool Game::Update(float dt)
{
	if (_gameObjects.size() > 0)
	{
		auto &body = _gameObjects[0]->GetBody();
		if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		{
			body.velocity += Vector2(-50.f, 0);
		}
		if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		{
			body.velocity += Vector2(50.f, 0);
		}
		if (GetAsyncKeyState(VK_UP) & 0x8000)
		{
			body.velocity += Vector2(0, 50.f);
		}
		if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		{
			body.velocity += Vector2(0, -50.f);
		}
	}

	for (int i = 0; i < _gameObjects.size(); i++)
	{
		RigidBody &body = _gameObjects[i]->GetBody();

		if (body.invMass > 0.f)
		{
			Vector2 totalAcc = body.acceleration + Vector2(0, -98.f);
			body.velocity += totalAcc * dt;
			body.position += body.velocity * dt;
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
	for (int i = 0; i < _gameObjects.size(); i++)
	{
		_gameObjects[i]->Draw(_spriteBatch);
	}
	_spriteBatch->End();

	_swapChain->Present(1, 0);
}

void Game::GetContacts()
{
	_contacts.clear();
	Vector2 groundNormal = Vector2(0, 1.0);
	float groundDepth = 70.f;

	for (int i = 0; i < _gameObjects.size(); i++)
	{
		RigidBody &body = _gameObjects[i]->GetBody();
		if (body.shape->type == ShapeType::Circle)
		{
			for (int j = i + 1; j < _gameObjects.size(); j++)
			{
				if (_gameObjects[j]->GetBody().shape->type == ShapeType::Circle)
				{
					CircleCircleCollision(body, _gameObjects[j]->GetBody());
				}
				else if (_gameObjects[j]->GetBody().shape->type == ShapeType::Box)
				{
					CircleBoxCollision(body, _gameObjects[j]->GetBody());
				}
			}
		}
		else if (body.shape->type == ShapeType::Box)
		{
			for (int j = i + 1; j < _gameObjects.size(); j++)
			{
				if (_gameObjects[j]->GetBody().shape->type == ShapeType::Circle)
				{
					CircleBoxCollision(_gameObjects[j]->GetBody(), body);
				}
				else if (_gameObjects[j]->GetBody().shape->type == ShapeType::Box)
				{
					BoxBoxCollision(body, _gameObjects[j]->GetBody());
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

		float totalInvMass = contact.bodyB.invMass;
		totalInvMass += contact.bodyA.invMass;
		if (totalInvMass < 0.0001f)
		{
			continue;
		}

		Vector2 relVelocity = contact.bodyB.velocity;
		relVelocity -= contact.bodyA.velocity;
		float incoming = relVelocity.Dot(contact.normal);
		if (incoming < 0)
		{
			float restitution = 0.5f * contact.bodyB.restitution;
			restitution += 0.5f * contact.bodyA.restitution;
			float outgoing = -incoming * restitution;
			float newSpeed = -incoming + outgoing;
			contact.bodyB.velocity += (contact.bodyB.invMass / totalInvMass) * contact.normal * newSpeed;
			contact.bodyA.velocity += (contact.bodyA.invMass / totalInvMass) * -contact.normal * newSpeed;

		}
		contact.bodyB.position += (contact.bodyB.invMass / totalInvMass) * contact.normal * contact.penDepth;
		contact.bodyA.position += (contact.bodyA.invMass / totalInvMass) * -contact.normal * contact.penDepth;
	}

}

void Game::CircleBoxCollision(RigidBody &circle, RigidBody &box)
{
	const Vector2 &halfWidths = ((BoxShape*)box.shape)->halfWidths;
	float radius = ((CircleShape*)circle.shape)->radius;

	Vector2 posDiff = circle.position - box.position;
	Matrix rotMatrix = Matrix::CreateRotationZ(-box.rotation);
	Vector2 up = Vector2::TransformNormal(Vector2(0, 1), rotMatrix);
	Vector2 right = Vector2::TransformNormal(Vector2(1, 0), rotMatrix);
	float distUp = up.Dot(posDiff);
	float distRight = right.Dot(posDiff);
	if (fabsf(distUp) > halfWidths.y)
	{
		distUp = std::copysignf(halfWidths.y, distUp);
	}

	if (fabsf(distRight) > halfWidths.x)
	{
		distRight = std::copysignf(halfWidths.x, distRight);
	}

	Vector2 closestPoint = box.position + up * distUp + right * distRight;

	Vector2 diff = circle.position - closestPoint;
	float dist = diff.Length();

	if (dist <= radius)
	{
		Contact c(box, circle);
		diff.Normalize(c.normal);
		c.penDepth = radius - dist;
		_contacts.push_back(c);

	}
}

void Game::CircleCircleCollision(RigidBody &circleA, RigidBody &circleB)
{
	Vector2 diff = circleB.position - circleA.position;
	float dist = diff.Length();
	float totalRadius = (((CircleShape*)circleA.shape)->radius + ((CircleShape*)circleB.shape)->radius);

	if (dist <= totalRadius)
	{
		Contact c(circleA, circleB);
		diff.Normalize(c.normal);
		c.penDepth = totalRadius - dist;
		_contacts.push_back(c);

	}
}

void Game::BoxBoxCollision(RigidBody &boxA, RigidBody &boxB)
{
	if (boxA.invMass + boxB.invMass < 0.00001f)
	{
		return;
	}
	const Vector2 &halfWidthsA = ((BoxShape*)boxA.shape)->halfWidths;
	const Vector2 &halfWidthsB = ((BoxShape*)boxB.shape)->halfWidths;
	Matrix rotMatrixA = Matrix::CreateRotationZ(-boxA.rotation);
	Vector2 upA = Vector2::TransformNormal(Vector2(0, 1), rotMatrixA);
	Vector2 rightA = Vector2::TransformNormal(Vector2(1, 0), rotMatrixA);
	Matrix rotMatrixB = Matrix::CreateRotationZ(-boxB.rotation);
	Vector2 upB = Vector2::TransformNormal(Vector2(0, 1), rotMatrixB);
	Vector2 rightB = Vector2::TransformNormal(Vector2(1, 0), rotMatrixB);

	Vector2 cornersA[] =
	{
		boxA.position + halfWidthsA.x * rightA + halfWidthsA.y * upA,
		boxA.position - halfWidthsA.x * rightA + halfWidthsA.y * upA,
		boxA.position + halfWidthsA.x * rightA - halfWidthsA.y * upA,
		boxA.position - halfWidthsA.x * rightA - halfWidthsA.y * upA,
	};
	Vector2 cornersB[] =
	{
		boxB.position + halfWidthsB.x * rightB + halfWidthsB.y * upB,
		boxB.position - halfWidthsB.x * rightB + halfWidthsB.y * upB,
		boxB.position + halfWidthsB.x * rightB - halfWidthsB.y * upB,
		boxB.position - halfWidthsB.x * rightB - halfWidthsB.y * upB,
	};

	Vector2 axes[] =
	{
		upA, -upA, rightA, -rightA,
		upB, -upB, rightB, -rightB
	};

	Vector2 minAxis;
	float minDepth = FLT_MAX;
	float minA = 0, maxA = 0;
	float minB = 0, maxB = 0;

	for (int i = 0; i < _countof(axes); i++)
	{
		ProjectBoxOnAxis(cornersA, axes[i], &minA, &maxA);
		ProjectBoxOnAxis(cornersB, axes[i], &minB, &maxB);
		float overlap1 = maxA - minB;
		float overlap2 = maxB - minA;
		float penDepth = min(overlap1, overlap2);
		if (penDepth < 0)
		{
			return;
		}
		if (penDepth < minDepth)
		{
			minDepth = penDepth;
			minAxis = axes[i];
			if (overlap2 < overlap1)
			{
				minAxis = -minAxis;
			}
		}
	}

	Contact c(boxA, boxB);
	c.normal = minAxis;
	c.penDepth = minDepth;
	_contacts.push_back(c);
}

void Game::ProjectBoxOnAxis(const Vector2 *corners, const Vector2 &axis, float *minVal, float *maxVal)
{
	float currMin = FLT_MAX;
	float currMax = -FLT_MAX;
	for (int i = 0; i < 4; i++)
	{
		float proj = corners[i].Dot(axis);
		if (proj < currMin)
		{
			currMin = proj;
		}
		if (proj > currMax)
		{
			currMax = proj;
		}
	}
	*minVal = currMin;
	*maxVal = currMax;
}