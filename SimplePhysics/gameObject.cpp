#include "gameObject.h"

using namespace DirectX;
using namespace Microsoft::WRL;

GameObject::GameObject(const ComPtr<ID3D11ShaderResourceView> &srv, float invMass, float restitution, float radius)
{
	_srv = srv;
	ComPtr<ID3D11Resource> resource;
	ComPtr<ID3D11Texture2D> tex;
	D3D11_TEXTURE2D_DESC texDesc = {};

	srv->GetResource(&resource);
	resource.As(&tex);
	tex->GetDesc(&texDesc);

	_body.shape = new CircleShape(radius);
	_body.invMass = invMass;
	_body.restitution = restitution;

	_scale = Vector2(radius / (texDesc.Width * 0.5f), radius / (texDesc.Height * 0.5f));
	_origin = Vector2((texDesc.Width * 0.5f), (texDesc.Height * 0.5f));
}

GameObject::GameObject(const ComPtr<ID3D11ShaderResourceView> &srv, float invMass, float restitution, const Vector2 &halfWidths)
{
	_srv = srv;
	ComPtr<ID3D11Resource> resource;
	ComPtr<ID3D11Texture2D> tex;
	D3D11_TEXTURE2D_DESC texDesc = {};

	srv->GetResource(&resource);
	resource.As(&tex);
	tex->GetDesc(&texDesc);

	_body.shape = new BoxShape(halfWidths);
	_body.invMass = invMass;
	_body.restitution = restitution;

	_scale = Vector2(halfWidths.x / (texDesc.Width * 0.5f), halfWidths.y / (texDesc.Height * 0.5f));
	_origin = Vector2((texDesc.Width * 0.5f), (texDesc.Height * 0.5f));
}

GameObject::~GameObject()
{

}

void GameObject::Draw(const std::shared_ptr<DirectX::SpriteBatch> &spriteBatch)
{
	Vector2 position = _body.position;
	position.y = 720 - position.y;
	spriteBatch->Draw(_srv.Get(), position, nullptr, Colors::White, _body.rotation, _origin, _scale);
}

