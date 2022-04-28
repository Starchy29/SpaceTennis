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
#include "Ball.h"

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
	void ScorePoint(bool playerPoint);

	static const int COURT_HALF_WIDTH = 10;
	static const int COURT_HALF_HEIGHT = 14;
	static const int AREA_HALF_WIDTH = 20;
	static const int AREA_HALF_HEIGHT = 18;

private:
	void UpdateEnemy(float dt);

	int playerScore;
	int enemyScore;

	std::vector<Entity*> court;
	Player* player;
	Ball* ball;
	Entity* enemy;

	// enemy rackets
	Entity* leftHead;
	Entity* rightHead;
	Entity* leftHandle;
	Entity* rightHandle;

	std::shared_ptr<Mesh> cube;
	std::shared_ptr<Mesh> cylinder;
	std::shared_ptr<Mesh> sphere;
	std::shared_ptr<Camera> worldCam;

	std::shared_ptr<Material> pureWhite;
	std::shared_ptr<Material> lightGreen;
	std::shared_ptr<Material> asteroid;
	std::shared_ptr<Material> wood;
	std::shared_ptr<Material> paint;

	DirectX::XMFLOAT3 ambientColor;
	Light dirLight;
	Light ballLight;

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

	// --------------------------------------------------------
	// Author: Chris Cascioli
	// Purpose: Creates a cube map on the GPU from 6 individual textures
	// 
	// - You are allowed to directly copy/paste this into your code base
	//   for assignments, given that you clearly cite that this is not
	//   code of your own design.
	//
	// - Note: This code assumes you’re putting the function in Game.cpp, 
	//   you’ve included WICTextureLoader.h and you have an ID3D11Device 
	//   ComPtr called “device”.  Make any adjustments necessary for
	//   your own implementation.
	// --------------------------------------------------------
	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);
};