#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include <memory>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

// Needed for a helper function to read compiled shader files from the hard drive
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true),			   // Show extra stats (fps) in title bar?
	vsync(false)
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	//CreateConsoleWindow(500, 120, 32, 120);
	//printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

	ambientColor = DirectX::XMFLOAT3(0.2f, 0.2f, 0.2f);
}

Game::~Game()
{
	for(Entity* entity : court) {
		delete entity;
	}
	delete player->racketHead;
	delete player->racketHandle;
	delete player;
	delete ball;
	delete sky;
	delete enemy;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	SetWindowText(hWnd, "Space Tennis: 0 - 0");

	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateBasicGeometry();
	
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	worldCam = std::make_shared<Camera>((float)this->width / this->height, XMFLOAT3(0, 15, -18));
	worldCam->GetTransform()->SetPitchYawRoll(0.6f, 0.0f, 0.0f); //0.6f
	worldCam->UpdateViewMatrix();

	dirLight = {};
	dirLight.type = LIGHT_TYPE_DIRECTIONAL;
	dirLight.direction = XMFLOAT3(0.0f, -0.2f, -1.0f);
	dirLight.color = XMFLOAT3(1.0f, 0.5f, 0.5f);
	dirLight.intensity = 0.3f;

	ballLight = {};
	ballLight.type = LIGHT_TYPE_POINT;
	ballLight.position = XMFLOAT3(0.0f, 0.0, 0.0);
	ballLight.color = XMFLOAT3(0.55f, 1.0f, 0.0f);
	ballLight.intensity = 1;
	ballLight.range = 10;
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(device, context, GetFullPathTo_Wide(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(device, context, GetFullPathTo_Wide(L"PixelShader.cso").c_str());
	skyVertexShader = std::make_shared<SimpleVertexShader>(device, context, GetFullPathTo_Wide(L"SkyVertexShader.cso").c_str());
	skyPixelShader = std::make_shared<SimplePixelShader>(device, context, GetFullPathTo_Wide(L"SkyPixelShader.cso").c_str());
	customPixelShader = std::make_shared<SimplePixelShader>(device, context, GetFullPathTo_Wide(L"CustomPS.cso").c_str());
}


void Game::CreateBasicGeometry()
{
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Load Models
	sphere = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/sphere.obj").c_str(), device, context);
	cube = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device, context);
	cylinder = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cylinder.obj").c_str(), device, context);

	// load textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronze;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeMetal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestone;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneMetal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> wood;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodMetal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paint;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintMetal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> asteroid;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> asteroidNormal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> whiteTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blackTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> standardNormal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skyBox;

	CreateWICTextureFromFile(device.Get(), context.Get(), 
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/bronze_albedo.png").c_str(), nullptr, bronze.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(), 
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/bronze_normals.png").c_str(), nullptr, bronzeNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/bronze_roughness.png").c_str(), nullptr, bronzeRoughness.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/bronze_metal.png").c_str(), nullptr, bronzeMetal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/cobblestone_albedo.png").c_str(), nullptr, cobblestone.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/cobblestone_normals.png").c_str(), nullptr, cobblestoneNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/cobblestone_roughness.png").c_str(), nullptr, cobblestoneRoughness.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/cobblestone_metal.png").c_str(), nullptr, cobblestoneMetal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/wood_albedo.png").c_str(), nullptr, wood.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/wood_normals.png").c_str(), nullptr, woodNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/wood_roughness.png").c_str(), nullptr, woodRoughness.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/wood_metal.png").c_str(), nullptr, woodMetal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/paint_albedo.png").c_str(), nullptr, paint.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/paint_normals.png").c_str(), nullptr, paintNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/paint_roughness.png").c_str(), nullptr, paintRoughness.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/paint_metal.png").c_str(), nullptr, paintMetal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/asteroid.png").c_str(), nullptr, asteroid.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/PBR/asteroid_normals.png").c_str(), nullptr, asteroidNormal.GetAddressOf());

	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/pixel.png").c_str(), nullptr, whiteTexture.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/normal.png").c_str(), nullptr, standardNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/blackTexture.png").c_str(), nullptr, blackTexture.GetAddressOf());

	skyBox = CreateCubemap(
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/Left_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/Right_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/Up_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/Down_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/Front_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/Back_Tex.png").c_str()
	);

	// create sampler
	D3D11_SAMPLER_DESC samplerDescription = {};
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDescription.MaxAnisotropy = 8;
	samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

	device.Get()->CreateSamplerState(&samplerDescription, samplerState.GetAddressOf());

	this->pureWhite = std::make_shared<Material>(white, vertexShader, pixelShader, 0.5f);
	this->lightGreen = std::make_shared<Material>(XMFLOAT4(0.55f, 1.0f, 0.0f, 1.0f), vertexShader, pixelShader, 0.5f);
	this->asteroid = std::make_shared<Material>(white, vertexShader, pixelShader, 0.5f);
	this->wood = std::make_shared<Material>(white, vertexShader, pixelShader, 0.5f);
	this->paint = std::make_shared<Material>(white, vertexShader, pixelShader, 0.5f);

	this->pureWhite.get()->AddSampler("DefaultSampler", samplerState.Get());
	this->lightGreen.get()->AddSampler("DefaultSampler", samplerState.Get());
	this->asteroid.get()->AddSampler("DefaultSampler", samplerState.Get());
	this->wood.get()->AddSampler("DefaultSampler", samplerState.Get());
	this->paint.get()->AddSampler("DefaultSampler", samplerState.Get());

	this->pureWhite.get()->AddTextureSRV("Albedo", whiteTexture.Get());
	this->pureWhite.get()->AddTextureSRV("NormalMap", bronzeNormal.Get());
	this->pureWhite.get()->AddTextureSRV("RoughnessMap", whiteTexture.Get());
	this->pureWhite.get()->AddTextureSRV("MetalMap", blackTexture.Get());

	this->lightGreen.get()->AddTextureSRV("Albedo", whiteTexture.Get());
	this->lightGreen.get()->AddTextureSRV("NormalMap", bronzeNormal.Get());
	this->lightGreen.get()->AddTextureSRV("RoughnessMap", whiteTexture.Get());
	this->lightGreen.get()->AddTextureSRV("MetalMap", blackTexture.Get());

	this->asteroid.get()->AddTextureSRV("Albedo", asteroid.Get());
	this->asteroid.get()->AddTextureSRV("NormalMap", asteroidNormal.Get());
	this->asteroid.get()->AddTextureSRV("RoughnessMap", whiteTexture.Get());
	this->asteroid.get()->AddTextureSRV("MetalMap", blackTexture.Get());

	this->wood.get()->AddTextureSRV("Albedo", wood.Get());
	this->wood.get()->AddTextureSRV("NormalMap", woodNormal.Get());
	this->wood.get()->AddTextureSRV("RoughnessMap", woodRoughness.Get());
	this->wood.get()->AddTextureSRV("MetalMap", woodMetal.Get());

	this->paint.get()->AddTextureSRV("Albedo", paint.Get());
	this->paint.get()->AddTextureSRV("NormalMap", paintNormal.Get());
	this->paint.get()->AddTextureSRV("RoughnessMap", paintRoughness.Get());
	this->paint.get()->AddTextureSRV("MetalMap", paintMetal.Get());

	this->asteroid->SetUVScale(4.0f);

	float cubeScaler = 0.502f; // value to make cube have radius 1
	float lineWidth = 0.2f;
	float alleyWidth = 3.0f;
	float lineHeight = -0.49; // lines are slightly above ground;

	court = std::vector<Entity*>();
	Entity* courtBase = new Entity(cube, this->asteroid);
	courtBase->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	courtBase->GetTransform()->Scale(AREA_HALF_WIDTH * 4, 1, 2 * AREA_HALF_HEIGHT);
	courtBase->GetTransform()->SetPosition(0, -0.5f, 0);
	court.push_back(courtBase);

	Entity* leftSingles = new Entity(cube, this->pureWhite);
	leftSingles->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	leftSingles->GetTransform()->Scale(lineWidth, 1, COURT_HALF_HEIGHT * 2);
	leftSingles->GetTransform()->SetPosition(-COURT_HALF_WIDTH, lineHeight, 0);
	court.push_back(leftSingles);

	Entity* rightSingles = new Entity(cube, this->pureWhite);
	rightSingles->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	rightSingles->GetTransform()->Scale(lineWidth, 1, COURT_HALF_HEIGHT * 2);
	rightSingles->GetTransform()->SetPosition(COURT_HALF_WIDTH, lineHeight, 0);
	court.push_back(rightSingles);

	Entity* leftDoubles = new Entity(cube, this->pureWhite);
	leftDoubles->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	leftDoubles->GetTransform()->Scale(lineWidth, 1, COURT_HALF_HEIGHT * 2);
	leftDoubles->GetTransform()->SetPosition(-COURT_HALF_WIDTH - alleyWidth, lineHeight, 0);
	court.push_back(leftDoubles);

	Entity* rightDoubles = new Entity(cube, this->pureWhite);
	rightDoubles->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	rightDoubles->GetTransform()->Scale(lineWidth, 1, COURT_HALF_HEIGHT * 2);
	rightDoubles->GetTransform()->SetPosition(COURT_HALF_WIDTH + alleyWidth, lineHeight, 0);
	court.push_back(rightDoubles);

	Entity* tLine = new Entity(cube, this->pureWhite);
	tLine->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	tLine->GetTransform()->Scale(lineWidth, 1, COURT_HALF_HEIGHT);
	tLine->GetTransform()->SetPosition(0, lineHeight, 0);
	court.push_back(tLine);

	Entity* backBase = new Entity(cube, this->pureWhite);
	backBase->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	backBase->GetTransform()->Scale(COURT_HALF_WIDTH * 2 + 2 * alleyWidth, 1, lineWidth);
	backBase->GetTransform()->SetPosition(0, lineHeight, -COURT_HALF_HEIGHT);
	court.push_back(backBase);

	Entity* farBase = new Entity(cube, this->pureWhite);
	farBase->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	farBase->GetTransform()->Scale(COURT_HALF_WIDTH * 2 + 2 * alleyWidth, 1, lineWidth);
	farBase->GetTransform()->SetPosition(0, lineHeight, COURT_HALF_HEIGHT);
	court.push_back(farBase);

	Entity* backServe = new Entity(cube, this->pureWhite);
	backServe->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	backServe->GetTransform()->Scale(COURT_HALF_WIDTH * 2, 1, lineWidth);
	backServe->GetTransform()->SetPosition(0, lineHeight, -COURT_HALF_HEIGHT / 2);
	court.push_back(backServe);

	Entity* farServe = new Entity(cube, this->pureWhite);
	farServe->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	farServe->GetTransform()->Scale(COURT_HALF_WIDTH * 2, 1, lineWidth);
	farServe->GetTransform()->SetPosition(0, lineHeight, COURT_HALF_HEIGHT / 2);
	court.push_back(farServe);

	Entity* net = new Entity(cube, this->pureWhite);
	net->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	net->GetTransform()->Scale(28, 3.0f, 0.2f);
	net->GetTransform()->SetPosition(0, 1.5f, 0);
	court.push_back(net);

	player = new Player(cube, this->paint);
	player->GetTransform()->Scale(cubeScaler, 2 * cubeScaler, cubeScaler);

	player->racketHandle = new Entity(cube, this->wood);
	player->racketHandle->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	player->racketHandle->GetTransform()->Scale(1.0f, 0.2f, 0.2f);

	leftHandle = new Entity(cube, this->wood);
	leftHandle->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	leftHandle->GetTransform()->Scale(1.0f, 0.2f, 0.2f);
	court.push_back(leftHandle); // I guess this counts as the court too now? idk man

	rightHandle = new Entity(cube, this->wood);
	rightHandle->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	rightHandle->GetTransform()->Scale(1.0f, 0.2f, 0.2f);
	court.push_back(rightHandle);

	player->racketHead = new Entity(cylinder, this->wood);
	player->racketHead->GetTransform()->Rotate(DirectX::XM_PIDIV2, 0.0f, 0.0f);
	player->racketHead->GetTransform()->Scale(0.8f, 0.1f, 0.5f);

	leftHead = new Entity(cylinder, this->wood);
	leftHead->GetTransform()->Rotate(DirectX::XM_PIDIV2, 0.0f, 0.0f);
	leftHead->GetTransform()->Scale(0.8f, 0.1f, 0.5f);
	court.push_back(leftHead);

	rightHead = new Entity(cylinder, this->wood);
	rightHead->GetTransform()->Rotate(DirectX::XM_PIDIV2, 0.0f, 0.0f);
	rightHead->GetTransform()->Scale(0.8f, 0.1f, 0.5f);
	court.push_back(rightHead);

	ball = new Ball(sphere, this->lightGreen);
	ball->GetTransform()->Scale(0.8f, 0.8f, 0.8f);

	enemy = new Entity(cube, this->paint);
	enemy->GetTransform()->SetPosition(0, 1.5, COURT_HALF_HEIGHT);
	enemy->GetTransform()->Scale(cubeScaler, 2 * cubeScaler, cubeScaler);

	sky = new Sky(cube, samplerState, device, skyVertexShader, skyPixelShader, skyBox);

	// throw some rocks in there just cause
	for(int i = 0; i < 20; i++) {
		Entity* left = new Entity(sphere, this->asteroid);
		left->GetTransform()->SetPosition(rand() % 1000 / 1000.0f * -20 - COURT_HALF_WIDTH - 5, 0, rand() % 1000 / 1000.0f * 2 * AREA_HALF_HEIGHT - AREA_HALF_HEIGHT);
		left->GetTransform()->Scale(rand() % 1000 / 1000.0f * 3, rand() % 1000 / 1000.0f * 3, rand() % 1000 / 1000.0f * 3);
		court.push_back(left);

		Entity* right = new Entity(sphere, this->asteroid);
		right->GetTransform()->SetPosition(rand() % 1000 / 1000.0f * 20 + COURT_HALF_WIDTH + 5, 0, rand() % 1000 / 1000.0f * 2 * AREA_HALF_HEIGHT - AREA_HALF_HEIGHT);
		right->GetTransform()->Scale(rand() % 1000 / 1000.0f * 3, rand() % 1000 / 1000.0f * 3, rand() % 1000 / 1000.0f * 3);
		court.push_back(right);
	}
}


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
	if(worldCam != nullptr) {
		worldCam->UpdateProjectionMatrix((float)this->width / this->height);
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();

	player->Update(deltaTime, ball);

	if(ball->IsActive()) {
		int scorer = ball->Update(deltaTime);
		if(scorer > 0) {
			ScorePoint(true);
		}
		else if(scorer < 0) {
			ScorePoint(false);
		}
		ballLight.intensity = 1;
		ballLight.position = ball->GetTransform()->GetPosition();

		UpdateEnemy(deltaTime);
	} 
	else {
		ballLight.intensity = 0; // don't show
		if(Input::GetInstance().KeyPress('W')) {
			ball->Serve(player->GetTransform()->GetPosition());
		}

		// lock player to baseline when serving
		XMFLOAT3 playPos = player->GetTransform()->GetPosition();
		player->GetTransform()->SetPosition(playPos.x, playPos.y, -COURT_HALF_HEIGHT - 0.5f);
	}
	
	worldCam->Update(player->GetTransform()->GetPosition());
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Cornflower Blue in this case) for clearing
	const float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV.Get(), color);
	context->ClearDepthStencilView(
		depthStencilView.Get(),
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	pixelShader->SetData(
		"directionalLight",   // The name of the (eventual) variable in the shader 
		&dirLight,   // The address of the data to set 
		sizeof(Light));  // The size of the data (the whole struct!) to set

	pixelShader->SetData(
		"ballLight",
		&ballLight,
		sizeof(Light));

	
	for(Entity* entity : court) {
		entity->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
		entity->Draw(context, worldCam);
	}
	player->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
	player->Draw(context, worldCam);
	player->racketHead->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
	player->racketHead->Draw(context, worldCam);
	player->racketHandle->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
	player->racketHandle->Draw(context, worldCam);
	if(ball->IsActive()) {
		ball->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
		ball->Draw(context, worldCam);
	}

	enemy->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
	enemy->Draw(context, worldCam);

	sky->Draw(context, worldCam);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(vsync ? 1 : 0, 0);

	// Due to the usage of a more sophisticated swap chain,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}

void Game::ScorePoint(bool playerPoint)
{
	if (playerPoint) {
		playerScore += 15;
		if(playerScore == 45) {
			playerScore = 40; // because Tennis is silly
		}
	} else {
		enemyScore += 15;
		if(enemyScore == 45) {
			enemyScore = 40; // because Tennis is silly
		}
	}

	if (playerScore >= 50 || enemyScore >= 50) {
		// end game
		playerScore = 0;
		enemyScore = 0;
	}

	std::string title = "Space Tennis: ";
	title.append(std::to_string(playerScore));
	title.append(" - ");
	title.append(std::to_string(enemyScore));
	SetWindowText(hWnd, title.c_str());
}

void Game::UpdateEnemy(float dt) {
	// move towards ball
	XMFLOAT3 ballPos = ball->GetTransform()->GetPosition();
	XMFLOAT3 position = enemy->GetTransform()->GetPosition();
	float enemSpeed = 8.0f;
	if(position.x > ballPos.x + 1.0f) {
		enemy->GetTransform()->MoveAbsolute(-enemSpeed * dt, 0, 0);
	}
	else if(position.x < ballPos.x - 1.0f) {
		enemy->GetTransform()->MoveAbsolute(enemSpeed * dt, 0, 0);
	}

	if(ballPos.z > COURT_HALF_HEIGHT - 4.0f && ballPos.y > 3.0f && ballPos.y > position.y) { // jump to ball when it gets towards the back
		enemy->GetTransform()->MoveAbsolute(0, 2 * enemSpeed * dt, 0);
	}
	else if (position.y > 1.5f) { // come back down when ball is gone
		enemy->GetTransform()->MoveAbsolute(0, -2 * enemSpeed * dt, 0);
	}

	// move rackets too
	rightHandle->GetTransform()->SetPosition(0.9f + position.x, position.y, position.z);
	leftHandle->GetTransform()->SetPosition(-0.9f + position.x, position.y, position.z);
	rightHead->GetTransform()->SetPosition(1.5f + position.x, position.y, position.z);
	leftHead->GetTransform()->SetPosition(-1.5f + position.x, position.y, position.z);

	// hit ball when close
	if(ballPos.z > COURT_HALF_HEIGHT - 2.0f && ballPos.z < COURT_HALF_HEIGHT + 2.0f
		&& abs(ballPos.x - position.x) < 1.0f
		&& abs(ballPos.y - position.y) < 2.0f
	) {
		float chanceChanger = 1.0f; // more likely to hit cross court
		if(position.x > 0) {
			chanceChanger *= -1;
		}

		ball->Hit(Vector3(rand() % 1000 / 1000.0f * 8.0f - 4.0f + chanceChanger, 8, -13), false);
	}
}

// --------------------------------------------------------
// Loads six individual textures (the six faces of a cube map), then
// creates a blank cube map and copies each of the six textures to
// another face.  Afterwards, creates a shader resource view for
// the cube map and cleans up all of the temporary resources.
// --------------------------------------------------------
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Game::CreateCubemap(
	const wchar_t* right,
	const wchar_t* left,
	const wchar_t* up,
	const wchar_t* down,
	const wchar_t* front,
	const wchar_t* back)
{
	// Load the 6 textures into an array.
	// - We need references to the TEXTURES, not the SHADER RESOURCE VIEWS!
	// - Specifically NOT generating mipmaps, as we usually don't need them for the sky!
	// - Order matters here!  +X, -X, +Y, -Y, +Z, -Z
	ID3D11Texture2D* textures[6] = {};
	CreateWICTextureFromFile(device.Get(), right, (ID3D11Resource**)&textures[0], 0);
	CreateWICTextureFromFile(device.Get(), left, (ID3D11Resource**)&textures[1], 0);
	CreateWICTextureFromFile(device.Get(), up, (ID3D11Resource**)&textures[2], 0);
	CreateWICTextureFromFile(device.Get(), down, (ID3D11Resource**)&textures[3], 0);
	CreateWICTextureFromFile(device.Get(), front, (ID3D11Resource**)&textures[4], 0);
	CreateWICTextureFromFile(device.Get(), back, (ID3D11Resource**)&textures[5], 0);

	// We'll assume all of the textures are the same color format and resolution,
	// so get the description of the first shader resource view
	D3D11_TEXTURE2D_DESC faceDesc = {};
	textures[0]->GetDesc(&faceDesc);

	// Describe the resource for the cube map, which is simply 
	// a "texture 2d array".  This is a special GPU resource format, 
	// NOT just a C++ array of textures!!!
	D3D11_TEXTURE2D_DESC cubeDesc = {};
	cubeDesc.ArraySize = 6; // Cube map!
	cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // We'll be using as a texture in a shader
	cubeDesc.CPUAccessFlags = 0; // No read back
	cubeDesc.Format = faceDesc.Format; // Match the loaded texture's color format
	cubeDesc.Width = faceDesc.Width;  // Match the size
	cubeDesc.Height = faceDesc.Height; // Match the size
	cubeDesc.MipLevels = 1; // Only need 1
	cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // A CUBE MAP, not 6 separate textures
	cubeDesc.Usage = D3D11_USAGE_DEFAULT; // Standard usage
	cubeDesc.SampleDesc.Count = 1;
	cubeDesc.SampleDesc.Quality = 0;

	// Create the actual texture resource
	ID3D11Texture2D* cubeMapTexture = 0;
	device->CreateTexture2D(&cubeDesc, 0, &cubeMapTexture);

	// Loop through the individual face textures and copy them,
	// one at a time, to the cube map texure
	for (int i = 0; i < 6; i++)
	{
		// Calculate the subresource position to copy into
		unsigned int subresource = D3D11CalcSubresource(
			0,	// Which mip (zero, since there's only one)
			i,	// Which array element?
			1); 	// How many mip levels are in the texture?

		// Copy from one resource (texture) to another
		context->CopySubresourceRegion(
			cubeMapTexture, // Destination resource
			subresource,	// Dest subresource index (one of the array elements)
			0, 0, 0,		// XYZ location of copy
			textures[i],	// Source resource
			0,	// Source subresource index (we're assuming there's only one)
			0);	// Source subresource "box" of data to copy (zero means the whole thing)
	}

	// At this point, all of the faces have been copied into the 
	// cube map texture, so we can describe a shader resource view for it
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = cubeDesc.Format; 	// Same format as texture
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; // Treat this as a cube!
	srvDesc.TextureCube.MipLevels = 1;	// Only need access to 1 mip
	srvDesc.TextureCube.MostDetailedMip = 0; // Index of the first mip we want to see

	// Make the SRV
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
	device->CreateShaderResourceView(cubeMapTexture, &srvDesc, cubeSRV.GetAddressOf());

	// Now that we're done, clean up the stuff we don't need anymore
	cubeMapTexture->Release();  // Done with this particular reference (the SRV has another)
	for (int i = 0; i < 6; i++)
		textures[i]->Release();

	// Send back the SRV, which is what we need for our shaders
	return cubeSRV;
}