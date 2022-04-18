#pragma once

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include "Mesh.h"
#include <memory>
#include <vector>
#include "Entity.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"
#include "Lights.h"
#include "Sky.h"
#include "Player.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	static const int COURT_HALF_WIDTH = 8;
	static const int COURT_HALF_HEIGHT = 14;
	static const int AREA_HALF_WIDTH = 20;
	static const int AREA_HALF_HEIGHT = 17;

private:
	std::vector<Entity*> court;
	Player* player;
	Entity* net;

	std::shared_ptr<Mesh> cube;
	std::shared_ptr<Mesh> sphere;
	std::shared_ptr<Mesh> spiral;
	std::shared_ptr<Camera> worldCam;
	std::shared_ptr<Material> blue;
	std::shared_ptr<Material> red;
	std::shared_ptr<Material> green;
	DirectX::XMFLOAT3 ambientColor;
	Light dirLight;
	Light redLight;
	Light greenLight;
	Light bluePoint;
	Light yellowPoint;

	// Should we use vsync to limit the frame rate?
	bool vsync;

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadShaders(); 
	void CreateBasicGeometry();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//    Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	
	// Shaders and shader-related constructs
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> customPixelShader;
	std::shared_ptr<SimplePixelShader> skyPixelShader;
	std::shared_ptr<SimpleVertexShader> skyVertexShader;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	Sky* sky;
};

