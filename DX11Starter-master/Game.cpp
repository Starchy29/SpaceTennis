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
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif

	ambientColor = DirectX::XMFLOAT3(0.2f, 0.2f, 0.2f);
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Note: Since we're using smart pointers (ComPtr),
	// we don't need to explicitly clean up those DirectX objects
	// - If we weren't using smart pointers, we'd need
	//   to call Release() on each DirectX object created in Game
	for (Entity* entity : entities) {
		delete entity;
	}
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateBasicGeometry();
	
	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	worldCam = std::make_shared<Camera>((float)this->width / this->height, XMFLOAT3(0, 0, -5));

	dirLight = {};
	dirLight.type = LIGHT_TYPE_DIRECTIONAL;
	dirLight.direction = XMFLOAT3(1.0f, 0.0, 0.0);
	dirLight.color = XMFLOAT3(1.0f, 1.0, 1.0);
	dirLight.intensity = 1;

	redLight = {};
	redLight.type = LIGHT_TYPE_DIRECTIONAL;
	redLight.direction = XMFLOAT3(-1.0f, 0.0, 0.0);
	redLight.color = XMFLOAT3(1.0f, 0.0, 0.0);
	redLight.intensity = 1;

	greenLight = {};
	greenLight.type = LIGHT_TYPE_DIRECTIONAL;
	greenLight.direction = XMFLOAT3(0.0f, 1.0, 0.0);
	greenLight.color = XMFLOAT3(0.0f, 1.0, 0.0);
	greenLight.intensity = 1;

	bluePoint = {};
	bluePoint.type = LIGHT_TYPE_POINT;
	bluePoint.position = XMFLOAT3(1.0f, 3.0, 0.0);
	bluePoint.color = XMFLOAT3(0.0f, 0.0, 1.0);
	bluePoint.intensity = 1;
	bluePoint.range = 20;

	yellowPoint = {};
	yellowPoint.type = LIGHT_TYPE_POINT;
	yellowPoint.position = XMFLOAT3(-2.0f, -3.0, 0.0);
	yellowPoint.color = XMFLOAT3(1.0f, 1.0, 0.0);
	yellowPoint.intensity = 1;
	yellowPoint.range = 10;
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
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	// Load Models
	sphere = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/sphere.obj").c_str(), device, context);
	cube = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device, context);
	spiral = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/helix.obj").c_str(), device, context);

	// load textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronze;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeMetal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestone;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneMetal;

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
	
	CreateDDSTextureFromFile(device.Get(), context.Get(), 
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/SunnyCubeMap.dds").c_str(), nullptr, skyBox.GetAddressOf());

	// create sampler
	D3D11_SAMPLER_DESC samplerDescription = {};
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDescription.MaxAnisotropy = 8;
	samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

	device.Get()->CreateSamplerState(&samplerDescription, samplerState.GetAddressOf());

	this->blue = std::make_shared<Material>(white, vertexShader, pixelShader, 0.5f);
	this->red = std::make_shared<Material>(white, vertexShader, pixelShader, 0.5f);
	this->green = std::make_shared<Material>(white, vertexShader, pixelShader, 0.5f);
	
	this->green.get()->AddSampler("DefaultSampler", samplerState.Get());
	this->blue.get()->AddSampler("DefaultSampler", samplerState.Get());
	this->red.get()->AddSampler("DefaultSampler", samplerState.Get());

	this->red.get()->AddTextureSRV("Albedo", bronze.Get());
	this->red.get()->AddTextureSRV("NormalMap", bronzeNormal.Get());
	this->red.get()->AddTextureSRV("RoughnessMap", bronzeRoughness.Get());
	this->red.get()->AddTextureSRV("MetalMap", bronzeMetal.Get());

	this->green.get()->AddTextureSRV("Albedo", bronze.Get());
	this->green.get()->AddTextureSRV("NormalMap", bronzeNormal.Get());
	this->green.get()->AddTextureSRV("RoughnessMap", bronzeRoughness.Get());
	this->green.get()->AddTextureSRV("MetalMap", bronzeMetal.Get());

	this->blue.get()->AddTextureSRV("Albedo", cobblestone.Get());
	this->blue.get()->AddTextureSRV("NormalMap", cobblestoneNormal.Get());
	this->blue.get()->AddTextureSRV("RoughnessMap", cobblestoneRoughness.Get());
	this->blue.get()->AddTextureSRV("MetalMap", cobblestoneMetal.Get());

	this->red.get()->SetUVScale(4.0f);

	entities = std::vector<Entity*>();
	Entity* cubeEntity = new Entity(cube, this->red);
	cubeEntity->GetTransform()->MoveAbsolute(-5, 0, 0);
	entities.push_back(cubeEntity);

	Entity* sphereEntity = new Entity(sphere, this->blue);
	entities.push_back(sphereEntity);

	Entity* spiralEntity = new Entity(spiral, this->green);
	spiralEntity->GetTransform()->MoveAbsolute(5, 0, 0);
	entities.push_back(spiralEntity);

	sky = new Sky(cube, samplerState, device, skyVertexShader, skyPixelShader, skyBox);
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

	worldCam->Update(deltaTime);

	for(Entity* entity : entities) {
		entity->GetTransform()->Rotate(0.0f, 0.0f, 0.1f * deltaTime);
	}
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
		"directionalLight1",   // The name of the (eventual) variable in the shader 
		&dirLight,   // The address of the data to set 
		sizeof(Light));  // The size of the data (the whole struct!) to set

	pixelShader->SetData(
		"redLight",   // The name of the (eventual) variable in the shader 
		&redLight,   // The address of the data to set 
		sizeof(Light));  // The size of the data (the whole struct!) to set

	pixelShader->SetData(
		"greenLight",   // The name of the (eventual) variable in the shader 
		&greenLight,   // The address of the data to set 
		sizeof(Light));  // The size of the data (the whole struct!) to set

	pixelShader->SetData(
		"bluePoint",   // The name of the (eventual) variable in the shader 
		&bluePoint,   // The address of the data to set 
		sizeof(Light));  // The size of the data (the whole struct!) to set

	pixelShader->SetData(
		"yellowPoint",   // The name of the (eventual) variable in the shader 
		&yellowPoint,   // The address of the data to set 
		sizeof(Light));  // The size of the data (the whole struct!) to set

	for(Entity* entity : entities) {
		entity->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
		entity->Draw(context, worldCam);
	}

	sky->Draw(context, worldCam);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(vsync ? 1 : 0, 0);

	// Due to the usage of a more sophisticated swap chain,
	// the render target must be re-bound after every call to Present()
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthStencilView.Get());
}