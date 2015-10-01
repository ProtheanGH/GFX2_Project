#include <ctime>
#include <d3d11.h>
#include <DirectXMath.h>
#include <iostream>

#include "Camera.h"
#include "DDSTextureLoader.h"
#include "Object.h"
#include "Vertex_Inputs.h"
#include "XTime.h"

// === Include Compiled Shaders
#include "VertexColor_PS.h"
#include "VertexColor_VS.h"

using namespace DirectX;
using namespace std;

#pragma pack_matrix( row_major )

#define BACKBUFFER_WIDTH	1024
#define BACKBUFFER_HEIGHT	780

#define SAFE_RELEASE(p) { if(p) { p->Release(); p = nullptr; } }

#define _Debug 1

// === Window Class
class ApplicationWindow
{	
	// === Constant Buffer Structures
	struct SEND_TO_VRAM_OBJECT
	{
		XMMATRIX worldMatrix;
	};
	struct SEND_TO_VRAM_SCENE
	{
		XMMATRIX viewMatrix;
		XMMATRIX projectionMatrix;
	};

	// === Window Variables
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;
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
	ID3D11Buffer* pObjectConstantBuffer;
	ID3D11Buffer* pSceneConstantBuffer;
	// === Shaders
	ID3D11VertexShader*				pVertexColor_VS;
	ID3D11PixelShader*				pVertexColor_PS;
	SEND_TO_VRAM_SCENE				toShaderScene;
	SEND_TO_VRAM_OBJECT				toShaderObject;
	// === Scene Objects
	Object							Star;
	Object							Ground;
	// === Variables
	Camera							m_Camera;
	XMMATRIX						ProjectionMatrix;
	XTime							Time;
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
	// ===== Priavte Interface
	void LoadObjects();
	void DrawObject(Object* _object);
	void DrawScene();
	void UpdateSceneBuffer();
};

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

	window = CreateWindow(	L"DirectXApplication", L"GFX II Project",	WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME|WS_MAXIMIZEBOX), 
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
	// ===

	// === Other Initializations
	LoadObjects();
	// ===

	// === Setup Projection Matrix
	float nearPlane = 0.1, farPlane = 100;
	float VertFOV = XMConvertToRadians(65), AspectRatio = BACKBUFFER_WIDTH / BACKBUFFER_HEIGHT;
	float yScale = (1 / tan(VertFOV * 0.5)), xScale = yScale * AspectRatio;
	ProjectionMatrix = XMMATRIX(xScale, 0, 0, 0,
		0, yScale, 0, 0,
		0, 0, farPlane / (farPlane - nearPlane), 1,
		0, 0, (-(farPlane * nearPlane)) / (farPlane - nearPlane), 0);
	toShaderScene.projectionMatrix = ProjectionMatrix;
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
	
	// === Set D3D11
	pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, pDepthView);
	pDeviceContext->OMSetBlendState(pBlendState, NULL, 0xffffffff);
	pDeviceContext->RSSetViewports(1, &viewPort);
	pDeviceContext->ClearRenderTargetView(pRenderTargetView, BLUE);
	pDeviceContext->ClearDepthStencilView(pDepthView, D3D11_CLEAR_DEPTH, 1, NULL);

	UpdateSceneBuffer();

	DrawScene();

	pSwapChain->Present(0, 0);
	return true; 
}
// ============================ //

// ===== D3D11 Initialization Functions ===== //
void ApplicationWindow::InitializeDeviceAndSwapChain()
{
	UINT flag = NULL;
#if _Debug
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
	swapDesc.SampleDesc.Count = 2;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
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
	viewPort.Height = swapDesc.BufferDesc.Height;
	viewPort.Width = swapDesc.BufferDesc.Width;
	viewPort.MinDepth = 0;
	viewPort.MaxDepth = 1;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
}

void ApplicationWindow::InitializeDepthView()
{
	// === Create the Depth-Stencil
	D3D11_TEXTURE2D_DESC depthDesc;
	depthDesc.Width = BACKBUFFER_WIDTH;
	depthDesc.Height = BACKBUFFER_HEIGHT;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.SampleDesc.Count = 2;
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
	rasterDesc.AntialiasedLineEnable = true;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = true;
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
}
// ========================================== //

// ===== Private Interface ===== //
void ApplicationWindow::LoadObjects()
{
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA initData;
	// === Load the Star Object
	// == Set the WorldMatrix
	Star.WorldMatrix = XMMATRIX(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 2, 1);
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

	// === Load the Ground

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

	// === Draw 
	pDeviceContext->DrawIndexed(_object->NumIndexes, 0, 0);
}

void ApplicationWindow::DrawScene()
{
	// === Draw the Objects
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DrawObject(&Star);
}

void ApplicationWindow::UpdateSceneBuffer()
{
	toShaderScene.viewMatrix = m_Camera.GetViewMatrix();

	D3D11_MAPPED_SUBRESOURCE sceneSubResource;
	pDeviceContext->Map(pSceneConstantBuffer, 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, NULL, &sceneSubResource);
	memcpy(sceneSubResource.pData, &toShaderScene, sizeof(toShaderScene));
	pDeviceContext->Unmap(pSceneConstantBuffer, 0);

	pDeviceContext->VSSetConstantBuffers(1, 1, &pSceneConstantBuffer);
}
// ============================= //

// ===== Windows Related ===== //	
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,	int nCmdShow );						   
LRESULT CALLBACK WndProc(HWND hWnd,	UINT message, WPARAM wparam, LPARAM lparam );		
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE, LPTSTR, int )
{
	srand(unsigned int(time(0)));
	ApplicationWindow myApp(hInstance, (WNDPROC)WndProc);
    MSG msg; ZeroMemory( &msg, sizeof( msg ) );
    while ( msg.message != WM_QUIT && myApp.Run() )
    {	
	    if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        { 
            TranslateMessage( &msg );
            DispatchMessage( &msg ); 
        }
    }
	myApp.ShutDown(); 
	return 0; 
}

LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if(GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
    switch ( message )
    {
        case ( WM_DESTROY ): { PostQuitMessage( 0 ); }
        break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}
// =========================== //