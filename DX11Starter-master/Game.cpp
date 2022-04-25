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
	delete player;
	delete ball;
	delete net;
	delete sky;
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

	worldCam = std::make_shared<Camera>((float)this->width / this->height, XMFLOAT3(0, 15, -18));
	worldCam->GetTransform()->SetPitchYawRoll(0.6f, 0.0f, 0.0f); //0.6f
	worldCam->UpdateViewMatrix();

	dirLight = {};
	dirLight.type = LIGHT_TYPE_DIRECTIONAL;
	dirLight.direction = XMFLOAT3(0.0f, -0.2f, -1.0f);
	dirLight.color = XMFLOAT3(1.0f, 0.5f, 0.5f);
	dirLight.intensity = 1;

	backLeftLight = {};
	backLeftLight.type = LIGHT_TYPE_DIRECTIONAL;
	backLeftLight.direction = XMFLOAT3(1.0f, -0.2f, 1.0f);
	backLeftLight.color = XMFLOAT3(0.2f, 0.0f, 0.4f);
	backLeftLight.intensity = 1.0f;

	backRightLight = {};
	backRightLight.type = LIGHT_TYPE_DIRECTIONAL;
	backRightLight.direction = XMFLOAT3(-1.0f, -0.2f, 1.0f);
	backRightLight.color = XMFLOAT3(0.2f, 0.0f, 0.4f);
	backRightLight.intensity = 1.0f;

	/*bluePoint = {};
	bluePoint.type = LIGHT_TYPE_POINT;
	bluePoint.position = XMFLOAT3(1.0f, 3.0, 0.0);
	bluePoint.color = XMFLOAT3(0.0f, 0.0, 1.0);
	bluePoint.intensity = 1;
	bluePoint.range = 20;*/
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
	//racket = std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/Racket tennis.obj").c_str(), device, context);

	// load textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronze;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeMetal;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestone;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneNormal;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneRoughness;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobblestoneMetal;

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
		GetFullPathTo_Wide(L"../../Assets/Textures/pixel.png").c_str(), nullptr, whiteTexture.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/normal.png").c_str(), nullptr, standardNormal.GetAddressOf());
	CreateWICTextureFromFile(device.Get(), context.Get(),
		GetFullPathTo_Wide(L"../../Assets/Textures/blackTexture.png").c_str(), nullptr, blackTexture.GetAddressOf());

	/*skyBox = CreateCubemap(
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/Left_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/Right_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/Up_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/Down_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/Front_Tex.png").c_str(),
		GetFullPathTo_Wide(L"../../Assets/Textures/Skies/Back_Tex.png").c_str()
	);*/

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

	this->pureWhite = std::make_shared<Material>(white, vertexShader, pixelShader, 0.5f);
	this->lightGreen = std::make_shared<Material>(XMFLOAT4(0.55f, 1.0f, 0.0f, 1.0f), vertexShader, pixelShader, 0.5f);
	
	this->green.get()->AddSampler("DefaultSampler", samplerState.Get());
	this->blue.get()->AddSampler("DefaultSampler", samplerState.Get());
	this->red.get()->AddSampler("DefaultSampler", samplerState.Get());
	this->pureWhite.get()->AddSampler("DefaultSampler", samplerState.Get());
	this->lightGreen.get()->AddSampler("DefaultSampler", samplerState.Get());

	this->pureWhite.get()->AddTextureSRV("Albedo", whiteTexture.Get());
	this->pureWhite.get()->AddTextureSRV("NormalMap", bronzeNormal.Get());
	this->pureWhite.get()->AddTextureSRV("RoughnessMap", whiteTexture.Get());
	this->pureWhite.get()->AddTextureSRV("MetalMap", blackTexture.Get());

	this->lightGreen.get()->AddTextureSRV("Albedo", whiteTexture.Get());
	this->lightGreen.get()->AddTextureSRV("NormalMap", bronzeNormal.Get());
	this->lightGreen.get()->AddTextureSRV("RoughnessMap", whiteTexture.Get());
	this->lightGreen.get()->AddTextureSRV("MetalMap", blackTexture.Get());

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

	float cubeScaler = 0.502f; // value to make cube have radius 1
	float lineWidth = 0.2f;
	float alleyWidth = 3.0f;
	float lineHeight = -0.49; // lines are slightly above ground;

	court = std::vector<Entity*>();
	Entity* courtBase = new Entity(cube, this->blue);
	courtBase->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	courtBase->GetTransform()->Scale(AREA_HALF_WIDTH * 2, 1, 2 * AREA_HALF_HEIGHT);
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

	player = new Player(cube, this->pureWhite);
	player->GetTransform()->Scale(cubeScaler, 2 * cubeScaler, cubeScaler);

	ball = new Ball(sphere, this->lightGreen);

	net = new Entity(cube, this->pureWhite);
	net->GetTransform()->Scale(cubeScaler, cubeScaler, cubeScaler);
	net->GetTransform()->Scale(28, 3.0f, 0.2f);
	net->GetTransform()->SetPosition(0, 1.5f, 0);

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

	if(ball->IsActive()) {
		ball->Update(deltaTime);
	} 
	else if(Input::GetInstance().KeyPress('W')) {
		ball->Serve(player->GetTransform()->GetPosition());
	}

	player->Update(deltaTime, ball);
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
		"directionalLight1",   // The name of the (eventual) variable in the shader 
		&dirLight,   // The address of the data to set 
		sizeof(Light));  // The size of the data (the whole struct!) to set

	pixelShader->SetData(
		"directionalLight2", 
		&backLeftLight,  
		sizeof(Light));

	pixelShader->SetData(
		"directionalLight3",
		&backRightLight,
		sizeof(Light));

	
	for(Entity* entity : court) {
		entity->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
		entity->Draw(context, worldCam);
	}
	player->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
	player->Draw(context, worldCam);
	net->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
	net->Draw(context, worldCam);
	if(ball->IsActive()) {
		ball->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor);
		ball->Draw(context, worldCam);
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