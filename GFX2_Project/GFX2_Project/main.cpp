#include <ctime>
#include <d3d11.h>
#include <DirectXMath.h>
#include <iostream>

#include "Camera.h"
#include "DDSTextureLoader.h"
#include "Light.h"
#include "MoveComponent.h"
#include "Object.h"
#include "ObjLoader.h"
#include "Vertex_Inputs.h"
#include "XTime.h"

// === Include Compiled Shaders
#include "Model_PS.h"
#include "Model_VS.h"
#include "Skybox_PS.h"
#include "Skybox_VS.h"
#include "VertexColor_PS.h"
#include "VertexColor_VS.h"

using namespace DirectX;
using namespace std;

#pragma pack_matrix(row_major)

#define BACKBUFFER_WIDTH	1024
#define BACKBUFFER_HEIGHT	780

// === Macros
#define SAFE_RELEASE(p) { if(p) { p->Release(); p = nullptr; } }
#define Float4x4ToXMMAtrix(float4x4) { XMMATRIX(float4x4._11, float4x4._12, float4x4._13, float4x4._14, float4x4._21, float4x4._22, float4x4._23, float4x4._24, float4x4._31, float4x4._32, float4x4._33, float4x4._34, float4x4._41, float4x4._42, float4x4._43, float4x4._44) }

// === Window Class
class ApplicationWindow
{	
	// === Constant Buffer Structures
	struct SEND_TO_VRAM_OBJECT
	{
		XMFLOAT4X4 worldMatrix;
	};
	struct SEND_TO_VRAM_SCENE
	{
		XMFLOAT4X4 viewMatrix;
		XMFLOAT4X4 projectionMatrix;
	};

	// === Window Variables
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;
	int								width		= 1024;
	int								height		= 780;
	// === DirectX Variables
	IDXGISwapChain*					pSwapChain;
	ID3D11Device*					pDevice;
	ID3D11DeviceContext*			pDeviceContext;
	ID3D11RenderTargetView*			pRenderTargetView;
	D3D11_VIEWPORT					viewPort;
	ID3D11Texture2D*				pDepthStencil;
	ID3D11DepthStencilView*			pDepthView;
	ID3D11BlendState*				pBlendState;
	ID3D11RasterizerState*			pRS_CullBack;
	ID3D11RasterizerState*			pRS_CullFront;
	// === Constant Buffers
	ID3D11Buffer*					pObjectConstantBuffer;
	ID3D11Buffer*					pSceneConstantBuffer;
	ID3D11Buffer*					pLightConstantBuffer;
	// === Shaders
	ID3D11VertexShader*				pModel_VS;
	ID3D11PixelShader*				pModel_PS;
	ID3D11VertexShader*				pSkybox_VS;
	ID3D11PixelShader*				pSkybox_PS;
	ID3D11VertexShader*				pVertexColor_VS;
	ID3D11PixelShader*				pVertexColor_PS;
	SEND_TO_VRAM_SCENE				toShaderScene;
	SEND_TO_VRAM_OBJECT				toShaderObject;
	// === Render Texture
	ID3D11RenderTargetView*			pRenderTextureTargetView;
	ID3D11Texture2D*				pRenderTexture;
	// === Scene Objects
	Object							Star;
	Object							Ground;
	Object							Bamboo;
	Object							Skybox;
	Object							Barrel;
	Object							RTObject;
	Object							PatrolPointLight;
	// === Lights
	Lights							mLights;
	DirectionalLight				mDirectionalLight;
	PointLight						mPointLight;
	SpotLight						mSpotLight;
	AmbientLight					mAmbientLight;
	// === Variables
	Camera							m_Camera;
	Camera							m_SecondaryCamera;
	XMFLOAT4X4						ProjectionMatrix;
	XMFLOAT4X4						SecondaryProjectionMatrix;
	XTime							Time;
	bool							KeyBuffer;
	// === Colors
	float RED[4];
	float GREEN[4];
	float BLUE[4];
	float YELLOW[4];
	float DARKRED[4];
	float WHITE[4];
	float BLACK[4];
	float DARKGREEN[4];

public:
	// ===== Initialization
	ApplicationWindow(HINSTANCE hinst, WNDPROC proc);
	// ===== CleanUp
	bool ShutDown();
	// ===== Public Interface
	bool Run();
	void ResizeWindow(int _width, int _height);
private:
	// ===== D3D11 Initialization Functions
	void InitializeDeviceAndSwapChain();
	void InitializeRenderTarget();
	void SetupViewport();
	void InitializeDepthView();
	void InitializeBlendState();
	void InitializeRasterizerStates();
	void InitializeShaders();
	void InitializeConstantBuffers();
	void InitializeSamplerState();
	void InitializeRenderTexture();
	// ===== Priavte Interface
	void CreateLights();
	XMFLOAT4X4 CreateProjectionMatrix(float _fov, float _width, float _height);
	void CreateSkybox();
	void DrawSkybox(Camera _camera);
	void DrawRTObject();
	void DrawObject(Object* _object);
	void DrawScene();
	void LoadObjectModel(const char* _path, Object& _object);
	void LoadObjects();
	void UpdateSceneBuffer(Camera _camera, XMFLOAT4X4 _projMatrix);
	void UpdateLighting();
	void UpdateObjects();
};

// === Global Tracker of the Application
ApplicationWindow* pApplication;

// ===== Constructor ===== //
ApplicationWindow::ApplicationWindow(HINSTANCE hinst, WNDPROC proc)
{ 
	// === Colors === //
	RED[0] = 1; RED[1] = 0; RED[2] = 0; RED[3] = 1;
	GREEN[0] = 0; GREEN[1] = 1; GREEN[2] = 0; GREEN[3] = 1;
	BLUE[0] = 0; BLUE[1] = 0; BLUE[2] = 1; BLUE[3] = 1;
	YELLOW[0] = 1; YELLOW[1] = 1; YELLOW[2] = 0; YELLOW[3] = 1;
	DARKRED[0] = 0.5f; DARKRED[1] = 0; DARKRED[2] = 0; DARKRED[3] = 1;
	WHITE[0] = 1; WHITE[1] = 1; WHITE[2] = 1; WHITE[3] = 1;
	BLACK[0] = 0; BLACK[1] = 0; BLACK[2] = 0; BLACK[3] = 1;
	DARKGREEN[0] = 0; DARKGREEN[1] = 0.5f; DARKGREEN[2] = 0; DARKGREEN[3] = 0;
	// ============== //

	// === Create the Window
	application = hinst; 
	appWndProc = proc; 

	WNDCLASSEX  wndClass;
    ZeroMemory( &wndClass, sizeof( wndClass ) );
    wndClass.cbSize         = sizeof( WNDCLASSEX );             
    wndClass.lpfnWndProc    = appWndProc;						
    wndClass.lpszClassName  = L"DirectXApplication";            
	wndClass.hInstance      = application;		               
    wndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );    
    wndClass.hbrBackground  = ( HBRUSH )( COLOR_WINDOWFRAME ); 
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
    RegisterClassEx( &wndClass );

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(	L"DirectXApplication", L"GFX II Project",	WS_OVERLAPPEDWINDOW  /*& ~(WS_THICKFRAME|WS_MAXIMIZEBOX)*/, 
							CW_USEDEFAULT, CW_USEDEFAULT, window_size.right-window_size.left, window_size.bottom-window_size.top,					
							NULL, NULL,	application, this );												

    ShowWindow( window, SW_SHOW );
	// ===

	// === DirectX Initialization
	InitializeDeviceAndSwapChain();
	InitializeRenderTarget();
	SetupViewport();
	InitializeDepthView();
	InitializeBlendState();
	InitializeRasterizerStates();
	InitializeShaders();
	InitializeConstantBuffers();
	InitializeRenderTexture();
	// ===

	// === Other Initializations
	CreateLights();
	CreateSkybox();
	LoadObjects();
	// ===

	// === Create the Projection Matrix
	ProjectionMatrix = CreateProjectionMatrix(65.0f, (float)width, (float)height);
	toShaderScene.projectionMatrix = ProjectionMatrix;
	SecondaryProjectionMatrix = CreateProjectionMatrix(65.0f, 512.0f, 1024.0f);
	// ===

	// === Setup the Secondary Camera
	XMFLOAT4X4 secondaryView = RTObject.WorldMatrix;
	XMStoreFloat4x4(&secondaryView, XMMatrixMultiply(XMMatrixRotationY(XMConvertToRadians(180)), Float4x4ToXMMAtrix(secondaryView)));
	m_SecondaryCamera.SetViewMatrix(secondaryView);
	// ===
}
// ======================= //

// ===== CleanUp ===== //
bool ApplicationWindow::ShutDown()
{
	// === Release all DirectX Pointer Objects
	SAFE_RELEASE(pSwapChain);
	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pDeviceContext);
	SAFE_RELEASE(pRenderTargetView);
	SAFE_RELEASE(pDepthStencil);
	SAFE_RELEASE(pDepthView);
	SAFE_RELEASE(pBlendState);
	SAFE_RELEASE(pRS_CullBack);
	SAFE_RELEASE(pRS_CullFront);
	SAFE_RELEASE(pObjectConstantBuffer);
	SAFE_RELEASE(pSceneConstantBuffer);
	SAFE_RELEASE(pLightConstantBuffer);
	SAFE_RELEASE(pModel_PS);
	SAFE_RELEASE(pModel_VS);
	SAFE_RELEASE(pSkybox_PS);
	SAFE_RELEASE(pSkybox_VS);
	SAFE_RELEASE(pVertexColor_PS);
	SAFE_RELEASE(pVertexColor_VS);
	SAFE_RELEASE(pRenderTextureTargetView);
	SAFE_RELEASE(pRenderTexture);

	UnregisterClass(L"DirectXApplication", application);
	return true;
}
// =================== //

// ===== Public Interface ===== //
bool ApplicationWindow::Run()
{
	// === Update Time
	Time.Signal();

	// === Update Camera
	m_Camera.HandleInput(Time.Delta());

	// === Update the Lighting
	UpdateLighting();

	// === Render to Texture
	pDeviceContext->OMSetRenderTargets(1, &pRenderTextureTargetView, pDepthView);
	pDeviceContext->OMSetBlendState(pBlendState, NULL, 0xffffffff);
	pDeviceContext->ClearRenderTargetView(pRenderTextureTargetView, RED);
	pDeviceContext->ClearDepthStencilView(pDepthView, D3D11_CLEAR_DEPTH, 1, NULL);

	UpdateSceneBuffer(m_SecondaryCamera, SecondaryProjectionMatrix);

	DrawSkybox(m_SecondaryCamera);

	DrawScene();

	// === Normal Render
	pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthView);
	pDeviceContext->OMSetBlendState(pBlendState, NULL, 0xffffffff);
	pDeviceContext->ClearRenderTargetView(pRenderTargetView, BLUE);
	pDeviceContext->ClearDepthStencilView(pDepthView, D3D11_CLEAR_DEPTH, 1, NULL);

	UpdateSceneBuffer(m_Camera, ProjectionMatrix);

	DrawSkybox(m_Camera);

	DrawScene();

	DrawObject(&RTObject);

	// === Update all the Objects
	UpdateObjects();

	pSwapChain->Present(0, 0);
	return true; 
}

void ApplicationWindow::ResizeWindow(int _width, int _height)
{
	if (pSwapChain) {

		width = _width;
		height = _height;


		pDeviceContext->OMSetRenderTargets(0, 0, 0);

		// === Release the Render Target view
		pRenderTargetView->Release();

		HRESULT result;
		// Preserve the existing buffer count and format.
		// Automatically choose the width and height to match the client rect for HWNDs.
		result = pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);

		// === Create a render-target-view.
		ID3D11Texture2D* pBuffer;
		result = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);

		result = pDevice->CreateRenderTargetView(pBuffer, NULL, &pRenderTargetView);

		pBuffer->Release();

		DXGI_SWAP_CHAIN_DESC desc;
		pSwapChain->GetDesc(&desc);

		//width = desc.BufferDesc.Width;
		//height = desc.BufferDesc.Height;

		pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);

		// === Set up the viewport.
		SetupViewport();

		// === Create the Projection Matrix
		ProjectionMatrix = CreateProjectionMatrix(65.0f, (float)_width, (float)_height);
		toShaderScene.projectionMatrix = ProjectionMatrix;

		// === Recreate the Depth Buffer
		SAFE_RELEASE(pDepthStencil);
		SAFE_RELEASE(pDepthView);
		InitializeDepthView();
	}
}
// ============================ //

// ===== D3D11 Initialization Functions ===== //
void ApplicationWindow::InitializeDeviceAndSwapChain()
{
	UINT flag = NULL;
#if _DEBUG
	flag = D3D11_CREATE_DEVICE_DEBUG;
#endif
	// === Array of Feature Levels
	const D3D_FEATURE_LEVEL featureLevels[4] = { D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL::D3D_FEATURE_LEVEL_11_1 };
	// === SwapChain Description
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(swapDesc));
	swapDesc.BufferCount = 1;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapDesc.BufferDesc.Height = BACKBUFFER_HEIGHT;
	swapDesc.BufferDesc.Width = BACKBUFFER_WIDTH;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = window;
	swapDesc.Windowed = true;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.Flags = NULL;
	// === Feature Level 
	D3D_FEATURE_LEVEL featureLevel;

	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE, NULL, flag, featureLevels, 4, D3D11_SDK_VERSION, &swapDesc, &pSwapChain, &pDevice, &featureLevel, &pDeviceContext);
}

void ApplicationWindow::InitializeRenderTarget()
{
	ID3D11Resource*	pBackBuffer;
	pSwapChain->GetBuffer(0, __uuidof(pBackBuffer), reinterpret_cast<void**>(&pBackBuffer));
	pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
	SAFE_RELEASE(pBackBuffer);
}

void ApplicationWindow::SetupViewport()
{
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(swapDesc));
	pSwapChain->GetDesc(&swapDesc);
	viewPort.Height = height;
	viewPort.Width = width;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;

	pDeviceContext->RSSetViewports(1, &viewPort);
}

void ApplicationWindow::InitializeDepthView()
{
	// === Create the Depth-Stencil
	D3D11_TEXTURE2D_DESC depthDesc;
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	pDevice->CreateTexture2D(&depthDesc, NULL, &pDepthStencil);

	// === Create the Depth-Stencil View
	D3D11_DEPTH_STENCIL_VIEW_DESC dViewDesc;
	ZeroMemory(&dViewDesc, sizeof(dViewDesc));
	dViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	dViewDesc.Texture2D.MipSlice = 0;

	pDevice->CreateDepthStencilView(pDepthStencil, &dViewDesc, &pDepthView);
}

void ApplicationWindow::InitializeBlendState()
{
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	pDevice->CreateBlendState(&blendDesc, &pBlendState);
}

void ApplicationWindow::InitializeRasterizerStates()
{
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(rasterDesc));
	rasterDesc.AntialiasedLineEnable = false; // true;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false; // true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// === Create RS with Cull Back Mode
	pDevice->CreateRasterizerState(&rasterDesc, &pRS_CullBack);

	rasterDesc.CullMode = D3D11_CULL_FRONT;

	// === Create RS with Cull Front Mode
	pDevice->CreateRasterizerState(&rasterDesc, &pRS_CullFront);
}

void ApplicationWindow::InitializeShaders()
{
	// === Model Shaders
	pDevice->CreateVertexShader(&Model_VS, sizeof(Model_VS), NULL, &pModel_VS);
	pDevice->CreatePixelShader(&Model_PS, sizeof(Model_PS), NULL, &pModel_PS);
	// === Skybox Shaders
	pDevice->CreateVertexShader(&Skybox_VS, sizeof(Skybox_VS), NULL, &pSkybox_VS);
	pDevice->CreatePixelShader(&Skybox_PS, sizeof(Skybox_PS), NULL, &pSkybox_PS);
	// === VertexColor Shaders
	pDevice->CreateVertexShader(&VertexColor_VS, sizeof(VertexColor_VS), NULL, &pVertexColor_VS);
	pDevice->CreatePixelShader(&VertexColor_PS, sizeof(VertexColor_PS), NULL, &pVertexColor_PS);
}

void ApplicationWindow::InitializeConstantBuffers()
{
	D3D11_BUFFER_DESC bufferDesc;
	// == Object Buffer
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = sizeof(SEND_TO_VRAM_OBJECT);
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

	pDevice->CreateBuffer(&bufferDesc, NULL, &pObjectConstantBuffer);

	// == Scene Buffer
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = sizeof(SEND_TO_VRAM_SCENE);
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

	pDevice->CreateBuffer(&bufferDesc, NULL, &pSceneConstantBuffer);

	// == Light Buffer
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.ByteWidth = sizeof(Lights);
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
	bufferDesc.MiscFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;

	pDevice->CreateBuffer(&bufferDesc, NULL, &pLightConstantBuffer);
}

void ApplicationWindow::InitializeRenderTexture()
{
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	pDevice->CreateTexture2D(&desc, NULL, &pRenderTexture);

	pDevice->CreateRenderTargetView(pRenderTexture, NULL, &pRenderTextureTargetView);
}
// ========================================== //

// ===== Private Interface ===== //
void ApplicationWindow::CreateLights()
{
	// === Ambient Light
	mAmbientLight.LightColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	// === Directional Light
	mDirectionalLight.LightColor = XMFLOAT4(0.96f, 0.95f, 0.35f, 1.0f);
	mDirectionalLight.LightDirection = XMFLOAT4(1, -1, 0, 0);
	// === Point Light
	mPointLight.LightColor = XMFLOAT4(0.96f, 0.95f, 0.35f, 1.0f);
	mPointLight.Position = XMFLOAT4(3, 1, 3, 1);
	mPointLight.Radius = 2.0f;
	// === SpotLight
	mSpotLight.ConeDirection = XMFLOAT4(0, -1, 0, 0);
	mSpotLight.ConeRatio = 0.9f;
	mSpotLight.LightColor = XMFLOAT4(0.96f, 0.95f, 0.35f, 1.0f);
	mSpotLight.Position = XMFLOAT4(0, 3, 0, 1);
	mSpotLight.Radius = 1.0f;

	// === Lights Container
	mLights.mAmbientLight = mAmbientLight;
	mLights.mDirectionalLight = mDirectionalLight;
	mLights.mPointLight = mPointLight;
	mLights.mSpotLight = mSpotLight;
}

XMFLOAT4X4 ApplicationWindow::CreateProjectionMatrix(float _fov, float _width, float _height)
{
	float nearPlane = 0.1, farPlane = 100;
	float VertFOV = XMConvertToRadians(_fov), AspectRatio = _width / _height;
	float yScale = (1 / tan(VertFOV * 0.5)), xScale = yScale / AspectRatio;
	return XMFLOAT4X4(xScale, 0, 0, 0,
		0, yScale, 0, 0,
		0, 0, farPlane / (farPlane - nearPlane), 1,
		0, 0, (-(farPlane * nearPlane)) / (farPlane - nearPlane), 0);
}

void ApplicationWindow::CreateSkybox()
{
	// === Setup the World Matrix
	XMStoreFloat4x4(&Skybox.WorldMatrix, XMMatrixIdentity());

	// === Set up the Vertex Buffer and Index Buffer
	Vertex vertices[24];
	// == Front Face
	vertices[0] = Vertex(-5, 5, 5, 1, 0, 0.001f);
	vertices[1] = Vertex(5, 5, 5, 1, 0.5f, 0.001f);
	vertices[2] = Vertex(-5, -5, 5, 1, 0, 0.25f);
	vertices[3] = Vertex(5, -5, 5, 1, 0.5f, 0.25f);
	// == Back Face
	vertices[4] = Vertex(5, 5, -5, 1, 0.5f, 0.001f);
	vertices[5] = Vertex(-5, 5, -5, 1, 1, 0.001f);
	vertices[6] = Vertex(5, -5, -5, 1, 0.5f, 0.25f);
	vertices[7] = Vertex(-5, -5, -5, 1, 1, 0.25f);
	// == Left Face
	vertices[8] = Vertex(-5, 5, -5, 1, 0, 0.25f);
	vertices[9] = Vertex(-5, 5, 5, 1, 0.5f, 0.25f);
	vertices[10] = Vertex(-5, -5, -5, 1, 0, 0.5f);
	vertices[11] = Vertex(-5, -5, 5, 1, 0.5, 0.5f);
	// == Right Face
	vertices[12] = Vertex(5, 5, 5, 1, 0.5f, 0.25f);
	vertices[13] = Vertex(5, 5, -5, 1, 1, 0.25f);
	vertices[14] = Vertex(5, -5, 5, 1, 0.5f, 0.5f);
	vertices[15] = Vertex(5, -5, -5, 1, 1, 0.5f);
	// == Top Face
	vertices[16] = Vertex(-5, 5, -5, 1, 0, 0.5f);
	vertices[17] = Vertex(5, 5, -5, 1, 0.5f, 0.5f);
	vertices[18] = Vertex(-5, 5, 5, 1, 0, 0.74f);
	vertices[19] = Vertex(5, 5, 5, 1, 0.5f, 0.74f);
	// == Bottom Face
	vertices[20] = Vertex(-5, -5, 5, 1, 0.5f, 0.5f);
	vertices[21] = Vertex(5, -5, 5, 1, 1, 0.5f);
	vertices[22] = Vertex(-5, -5, -5, 1, 0.5f, 0.74f);
	vertices[23] = Vertex(5, -5, -5, 1, 1, 0.74f);
	// == Triangles (Indexes)
	unsigned int indexes[] = { 0, 1, 3, 0, 3, 2, 4, 5, 7, 4, 7, 6, 8, 9, 11, 8, 11, 10, 12, 13, 15, 12, 15, 14, 16, 17, 19, 16, 19, 18, 20, 21, 23, 20, 23, 22 };
	// == Setup Vertex Buffer
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.MiscFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	initData.pSysMem = vertices;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	pDevice->CreateBuffer(&bufferDesc, &initData, &Skybox.pVertexBuffer);
	// == Index Buffer
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(indexes);
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.MiscFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;

	initData.pSysMem = indexes;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	pDevice->CreateBuffer(&bufferDesc, &initData, &Skybox.pIndexBuffer);
	// == Vertex Size
	Skybox.VertexSize = sizeof(Vertex);
	// == Number of Indexes
	Skybox.NumIndexes = sizeof(indexes) / sizeof(unsigned int);

	// === Create the InputLayout
	pDevice->CreateInputLayout(Layout_Vertex, sizeof(Layout_Vertex) / sizeof(D3D11_INPUT_ELEMENT_DESC), Skybox_VS, sizeof(Skybox_VS), &Skybox.pInputLayout);

	// === Setup the Shaders
	Skybox.pPixelShader = pSkybox_PS;
	Skybox.pVertexShader = pSkybox_VS;

	// == Create the ShaderResourceView
	CreateDDSTextureFromFile(pDevice, L"NebulaSkybox.dds", NULL, &Skybox.pShaderResourceView);

	// === Setup the Sampler State
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	pDevice->CreateSamplerState(&samplerDesc, &Skybox.pSamplerState);
}

void ApplicationWindow::DrawSkybox(Camera _camera)
{
	// === Move the Skybox to the Camera's position
	XMFLOAT3 cameraPos = _camera.GetPosition();
	Skybox.WorldMatrix = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, cameraPos.x, cameraPos.y, cameraPos.z, 1);

	// === Draw the Skybox
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DrawObject(&Skybox);

	// === Clear the Depth Buffer
	pDeviceContext->ClearDepthStencilView(pDepthView, D3D11_CLEAR_DEPTH, 1, NULL);
}

// - LoadObjectModel
// --- Loads in the data from an obj file
// --- Sets up the Vertex Buffer, Index Buffer, Vertex Size, and Number of Indexes
void ApplicationWindow::LoadObjectModel(const char* _path, Object& _object)
{
	vector<Vector3> positions, uvs, normals;
	vector<Vertex> vertices;
	vector<unsigned int> indexes;
	// === Load the Data from the filepath
	LoadObjFile(_path, positions, uvs, normals);
	// === Cycle through the data, setting up the actaul object
	Vertex vert;
	Vertex* objectVertices = new Vertex[positions.size()];
	unsigned int* objectIndexes = new unsigned int[positions.size()];
	for (unsigned int i = 0; i < positions.size(); i++) {
		objectVertices[i] = Vertex(positions[i].x, positions[i].y, positions[i].z, 1, uvs[i].x, uvs[i].y, uvs[i].z, normals[i].x, normals[i].y, normals[i].z);
		objectIndexes[i] = i;
	}
	// === Setup the Object
	// == Vertex Buffer
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(Vertex) * positions.size();
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.MiscFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

	initData.pSysMem = objectVertices;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	pDevice->CreateBuffer(&bufferDesc, &initData, &_object.pVertexBuffer);
	// == Index Buffer
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.ByteWidth = sizeof(unsigned int) * positions.size();
	bufferDesc.CPUAccessFlags = NULL;
	bufferDesc.MiscFlags = 0;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;

//	memcpy(&initData.pSysMem, &objectIndexes, sizeof(unsigned int) * positions.size());
	initData.pSysMem = objectIndexes;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;

	pDevice->CreateBuffer(&bufferDesc, &initData, &_object.pIndexBuffer);
	// == Set the VertexSize
	_object.VertexSize = sizeof(Vertex);
	// == Set the Number of Vertices
	_object.NumIndexes = positions.size();
}

void ApplicationWindow::LoadObjects()
{
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA initData;
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	// === Load the Star Object
	{
		// == Set the WorldMatrix
		Star.WorldMatrix = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 2, 1);
		// == Set the Vertex Buffer
		Vertex_PositionColor vertices[17];
		vertices[0] = Vertex_PositionColor(0.0f, 0.0f, -0.15f, 1, WHITE);
		vertices[16] = Vertex_PositionColor(0.0f, 0.0f, 0.15f, 1, WHITE);
		float outerAngle = 90.0f, innerAngle = 270.0f;
		for (int i = 1; i < 15; i += 3) {
			vertices[i] = Vertex_PositionColor(0.5f * cos(outerAngle * 0.0174532925f), 0.5f * sin(outerAngle * 0.0174532925f), 0.0f, 1);
			vertices[i + 1] = Vertex_PositionColor(0.2f * cos(innerAngle * 0.0174532925f), 0.2f * sin(innerAngle * 0.0174532925f), -0.07f, 1, DARKRED);
			vertices[i + 2] = Vertex_PositionColor(0.2f * cos(innerAngle * 0.0174532925f), 0.2f * sin(innerAngle * 0.0174532925f), 0.07f, 1, DARKRED);
			outerAngle += 72;
			innerAngle += 72;
		}
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(vertices);
		bufferDesc.CPUAccessFlags = NULL;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.StructureByteStride = 8;

		initData.pSysMem = vertices;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer(&bufferDesc, &initData, &Star.pVertexBuffer);
		// == Set the Index Buffer
		unsigned int indexes[] = { 1, 0, 11, 1, 8, 0, 13, 0, 8, 13, 5, 0, 10, 0, 5, 10, 2, 0, 7, 0, 2, 7, 14, 0, 4, 0, 14, 4, 11, 0, 1, 12, 16, 1, 16, 9, 13, 9, 16, 13, 16, 6, 10, 6, 16, 10, 16, 3, 7, 3, 16, 7, 16, 15, 4, 15, 16, 4, 16, 12, 1, 9, 8, 13, 8, 9, 13, 6, 5, 10, 5, 6, 10, 3, 2, 7, 2, 3, 7, 15, 14, 4, 14, 15, 4, 12, 11, 1, 11, 12 };
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(indexes);
		bufferDesc.CPUAccessFlags = NULL;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;

		initData.pSysMem = indexes;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer(&bufferDesc, &initData, &Star.pIndexBuffer);
		// == Set the Shaders
		Star.pVertexShader = pVertexColor_VS;
		Star.pPixelShader = pVertexColor_PS;
		// == Set the InputLayout
		pDevice->CreateInputLayout(Layout_Vertex_PositionColor, 2, VertexColor_VS, sizeof(VertexColor_VS), &Star.pInputLayout);
		// == Set the VertexSize
		Star.VertexSize = sizeof(Vertex_PositionColor);
		// == Set the Number of Vertices
		Star.NumIndexes = sizeof(indexes) / sizeof(unsigned int);
	}

	// === Load the Ground
	{
		// == Set the WorldMatrix
		XMStoreFloat4x4(&Ground.WorldMatrix, XMMatrixIdentity());
		// == Set the Vertex Buffer
		Vertex groundVerts[4];
		float radius = 4.0f;
		groundVerts[0] = Vertex(-radius, 0, radius, 1, 0, 0, 0, 0, 1, 0);
		groundVerts[1] = Vertex(-radius, 0, -radius, 1, 0, 4, 0, 0, 1, 0);
		groundVerts[2] = Vertex(radius, 0, radius, 1, 4, 0, 0, 0, 1, 0);
		groundVerts[3] = Vertex(radius, 0, -radius, 1, 4, 4, 0, 0, 1, 0);
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(groundVerts);
		bufferDesc.CPUAccessFlags = NULL;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

		initData.pSysMem = groundVerts;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer(&bufferDesc, &initData, &Ground.pVertexBuffer);
		// == Set the Index Buffer
		unsigned int indexes[] = { 0, 3, 1, 0, 2, 3 };
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(indexes);
		bufferDesc.CPUAccessFlags = NULL;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;

		initData.pSysMem = indexes;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer(&bufferDesc, &initData, &Ground.pIndexBuffer);
		// == Set the Shaders
		Ground.pVertexShader = pModel_VS;
		Ground.pPixelShader = pModel_PS;
		// == Set the Texture and ShaderResourceView
		CreateDDSTextureFromFile(pDevice, L"SMGrass_Seamless.dds", NULL, &Ground.pShaderResourceView);
		// == Set the Sampler State
		pDevice->CreateSamplerState(&samplerDesc, &Ground.pSamplerState);
		// == Set the InputLayout
		pDevice->CreateInputLayout(Layout_Vertex, sizeof(Layout_Vertex) / sizeof(D3D11_INPUT_ELEMENT_DESC), Model_VS, sizeof(Model_VS), &Ground.pInputLayout);
		// == Set the VertexSize
		Ground.VertexSize = sizeof(Vertex);
		// == Set the Number of Vertices
		Ground.NumIndexes = sizeof(indexes) / sizeof(unsigned int);
	}

	// === Load the Bamboo
	{
		// == Set the WorldMatrix
		Bamboo.WorldMatrix = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -3, 0.2, 3, 1);
		// == Load the Obj File, Set up Vertex and Index Buffers
		LoadObjectModel("SingleBamboo.obj", Bamboo);
		// == Set the Shaders
		Bamboo.pVertexShader = pModel_VS;
		Bamboo.pPixelShader = pModel_PS;
		// == Set the Texture and ShaderResourceView
		CreateDDSTextureFromFile(pDevice, L"BambooT.dds", NULL, &Bamboo.pShaderResourceView);
		// == Set the Sampler State
		pDevice->CreateSamplerState(&samplerDesc, &Bamboo.pSamplerState);
		// == Set the InputLayout
		pDevice->CreateInputLayout(Layout_Vertex, sizeof(Layout_Vertex) / sizeof(D3D11_INPUT_ELEMENT_DESC), Model_VS, sizeof(Model_VS), &Bamboo.pInputLayout);
	}

	// === Load the Barrel
	{
		// == Set the WorldMatrix
		XMStoreFloat4x4(&Barrel.WorldMatrix, XMMatrixMultiply(XMMATRIX(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 2, 0, 2, 1), XMMatrixScaling(0.5f, 0.5f, 0.5f)));
		// == Load the Obj File, Set up Vertex and Index Buffers
		LoadObjectModel("Barrel.obj", Barrel);
		// == Set the Shaders
		Barrel.pVertexShader = pModel_VS;
		Barrel.pPixelShader = pModel_PS;
		// == Set the Texture and ShaderResourceView
		CreateDDSTextureFromFile(pDevice, L"barrel_diffuse.dds", NULL, &Barrel.pShaderResourceView);
		// == Set the Sampler State
		pDevice->CreateSamplerState(&samplerDesc, &Barrel.pSamplerState);
		// == Set the InputLayout
		pDevice->CreateInputLayout(Layout_Vertex, sizeof(Layout_Vertex) / sizeof(D3D11_INPUT_ELEMENT_DESC), Model_VS, sizeof(Model_VS), &Barrel.pInputLayout);
		// == Setup the MoveComponent
		Barrel.pMoveComponent = new MoveComponent(&Barrel);
		XMFLOAT3* Waypoints = new XMFLOAT3[2];
		Waypoints[0] = XMFLOAT3(-3, 0, 2); Waypoints[1] = XMFLOAT3(3, 0, 2);
		Barrel.pMoveComponent->SetWaypoints(Waypoints, 2);
		Barrel.pMoveComponent->Patrol(0);
	}

	// === Load the RTObject
	{
		// == Set the WorldMatrix
		RTObject.WorldMatrix = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 4, 1);
		// == Setup the Verts Buffer
		Vertex verts[4];
		verts[0] = Vertex(-0.5f, -1, 0, 1, 1, 1, 0, 0, 0, -1);
		verts[1] = Vertex(-0.5f, 1, 0, 1, 1, 0, 0, 0, 0, -1);
		verts[2] = Vertex(0.5f, 1, 0, 1, 0, 0, 0, 0, 0, -1);
		verts[3] = Vertex(0.5f, -1, 0, 1, 0, 1, 0, 0, 0, -1);
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(verts);
		bufferDesc.CPUAccessFlags = NULL;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;

		initData.pSysMem = verts;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer(&bufferDesc, &initData, &RTObject.pVertexBuffer);
		// == Setup the Index Buffer
		unsigned int indexes[] = { 0, 1, 3, 1, 2, 3 };
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.ByteWidth = sizeof(indexes);
		bufferDesc.CPUAccessFlags = NULL;
		bufferDesc.MiscFlags = 0;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;

		initData.pSysMem = indexes;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		pDevice->CreateBuffer(&bufferDesc, &initData, &RTObject.pIndexBuffer);
		// == Set the Shaders
		RTObject.pVertexShader = pModel_VS;
		RTObject.pPixelShader = pModel_PS;
		// == Set the Texture and ShaderResourceView
		pDevice->CreateShaderResourceView(pRenderTexture, NULL, &RTObject.pShaderResourceView);
		// == Set the Sampler State
		pDevice->CreateSamplerState(&samplerDesc, &RTObject.pSamplerState);
		// == Set the InputLayout
		pDevice->CreateInputLayout(Layout_Vertex, sizeof(Layout_Vertex) / sizeof(D3D11_INPUT_ELEMENT_DESC), Model_VS, sizeof(Model_VS), &RTObject.pInputLayout);
		// == Set the VertexSize
		RTObject.VertexSize = sizeof(Vertex);
		// == Set the Number of Vertices
		RTObject.NumIndexes = sizeof(indexes) / sizeof(unsigned int);
	}

	// === Load the PatrolPointLight
	{
		// == Set the WorldMatrix
		PatrolPointLight.WorldMatrix = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 3, 1, 3, 1);
		// == Setup the MoveComponent
		PatrolPointLight.pMoveComponent = new MoveComponent(&PatrolPointLight);
		XMFLOAT3* Waypoints = new XMFLOAT3[4];
		Waypoints[0] = XMFLOAT3(3, 1, 3); Waypoints[1] = XMFLOAT3(3, 1, -3); Waypoints[2] = XMFLOAT3(-3, 1, -3); Waypoints[3] = XMFLOAT3(-3, 1, 3);
		PatrolPointLight.pMoveComponent->SetWaypoints(Waypoints, 4);
		PatrolPointLight.pMoveComponent->Patrol(0);
	}
}

void ApplicationWindow::DrawObject(Object* _object)
{
	// === Update the ObjectConstantBuffer
	toShaderObject.worldMatrix = _object->WorldMatrix;
	D3D11_MAPPED_SUBRESOURCE objectSubResource;
	pDeviceContext->Map(pObjectConstantBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &objectSubResource);
	memcpy(objectSubResource.pData, &toShaderObject, sizeof(toShaderObject));
	pDeviceContext->Unmap(pObjectConstantBuffer, 0);
	pDeviceContext->VSSetConstantBuffers(0, 1, &pObjectConstantBuffer);

	// === Set the VertexBuffer
	UINT strides[] = { _object->VertexSize };
	UINT offsets[] = { 0 };
	pDeviceContext->IASetVertexBuffers(0, 1, &_object->pVertexBuffer, strides, offsets);

	// === Set the IndexBuffer
	pDeviceContext->IASetIndexBuffer(_object->pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// === Set the Shaders
	pDeviceContext->VSSetShader(_object->pVertexShader, NULL, 0);
	pDeviceContext->PSSetShader(_object->pPixelShader, NULL, 0);

	// === Set the Layout
	pDeviceContext->IASetInputLayout(_object->pInputLayout);

	// === Is there a Texture to take into account?
	pDeviceContext->PSSetShaderResources(0, 1, &_object->pShaderResourceView);
	pDeviceContext->PSSetSamplers(0, 1, &_object->pSamplerState);

	// === Draw 
	pDeviceContext->DrawIndexed(_object->NumIndexes, 0, 0);
}

void ApplicationWindow::DrawScene()
{
	// === Draw the Objects
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->RSSetState(pRS_CullBack);
	DrawObject(&Ground);
	DrawObject(&Star);
	DrawObject(&Bamboo);
	DrawObject(&Barrel);
}

void ApplicationWindow::UpdateSceneBuffer(Camera _camera, XMFLOAT4X4 _projMatrix)
{
	toShaderScene.viewMatrix = _camera.GetViewMatrix();
	toShaderScene.projectionMatrix = _projMatrix;

	D3D11_MAPPED_SUBRESOURCE sceneSubResource;
	pDeviceContext->Map(pSceneConstantBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &sceneSubResource);
	memcpy(sceneSubResource.pData, &toShaderScene, sizeof(toShaderScene));
	pDeviceContext->Unmap(pSceneConstantBuffer, 0);

	pDeviceContext->VSSetConstantBuffers(1, 1, &pSceneConstantBuffer);
}

void ApplicationWindow::UpdateLighting()
{
	// === Activate / Deactivate Lights
	if (GetAsyncKeyState('1') && !KeyBuffer) {
		// == Directional Light
		KeyBuffer = true;
		mLights.mDirectionalLight.LightColor.z == 0 ? mLights.mDirectionalLight = mDirectionalLight : mLights.mDirectionalLight = DirectionalLight();
	}
	else if (GetAsyncKeyState('2') && !KeyBuffer) {
		// == Point Light
		KeyBuffer = true;
		mLights.mPointLight.LightColor.z == 0 ? mLights.mPointLight = mPointLight : mLights.mPointLight = PointLight();
	}
	else if (GetAsyncKeyState('3') && !KeyBuffer) {
		// == Spot Light
		KeyBuffer = true;
		mLights.mSpotLight.LightColor.z == 0 ? mLights.mSpotLight = mSpotLight : mLights.mSpotLight = SpotLight();
	}
	if (!GetAsyncKeyState('1') && !GetAsyncKeyState('2') && !GetAsyncKeyState('3') && KeyBuffer) {
		KeyBuffer = false;
	}

	// === Light Input
	mLights.mSpotLight.HandleInput(Time.Delta());

	// === PointLight Position
	mLights.mPointLight.Position = XMFLOAT4(PatrolPointLight.GetPosition().x, PatrolPointLight.GetPosition().y, PatrolPointLight.GetPosition().z, 1);

	// === Rotate Directional Lighting
	XMFLOAT3 lightDir;
	XMStoreFloat3(&lightDir, XMVector3Rotate(XMLoadFloat4(&mLights.mDirectionalLight.LightDirection), XMLoadFloat4(&XMFLOAT4(0, Time.Delta() / 4.0f, 0, 1))));
	mLights.mDirectionalLight.LightDirection = XMFLOAT4(lightDir.x, lightDir.y, lightDir.z, 1);

	// === Update the Constant Buffera
	D3D11_MAPPED_SUBRESOURCE sceneSubResource;
	pDeviceContext->Map(pLightConstantBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &sceneSubResource);
	memcpy(sceneSubResource.pData, &mLights, sizeof(Lights));
	pDeviceContext->Unmap(pLightConstantBuffer, 0);

	pDeviceContext->PSSetConstantBuffers(0, 1, &pLightConstantBuffer);
}

void ApplicationWindow::UpdateObjects()
{
	// === Update any Objects that need to be
	Barrel.Update(Time.Delta());
	PatrolPointLight.Update(Time.Delta());
}
// ============================= //

// ===== Windows Related ===== //	
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,	int nCmdShow );						   
LRESULT CALLBACK WndProc(HWND hWnd,	UINT message, WPARAM wparam, LPARAM lparam );		
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE, LPTSTR, int )
{
	srand(unsigned int(time(0)));
	pApplication = new ApplicationWindow(hInstance, (WNDPROC)WndProc);
    MSG msg; ZeroMemory( &msg, sizeof( msg ) );
	while (msg.message != WM_QUIT && pApplication->Run())
    {	
	    if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        { 
            TranslateMessage( &msg );
            DispatchMessage( &msg ); 
        }
    }
	pApplication->ShutDown();
	delete pApplication;
	return 0; 
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if(GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
    switch ( message )
    {
        case ( WM_DESTROY ): { 
			PostQuitMessage( 0 ); 
		}
        break;
		case (WM_SIZE) : {
			if (pApplication) {
				pApplication->ResizeWindow(LOWORD(lParam), HIWORD(lParam));
			}
		}
			break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}
// =========================== //