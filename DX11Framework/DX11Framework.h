#pragma once

#include "OBJLoader.h"
#include "GameObject.h"
#include "BaseCamera.h"
#include "DebugFlyCamera.h"
#include "FollowObjectCamera.h"
#include <windows.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <list>
#include <array>
#include <vector>
#include "Terrain.h"
//#include <wrl.h>

struct MeshData;

struct ConstantBuffer
{
	XMMATRIX Projection;
	XMMATRIX View;
	XMMATRIX World;
	XMFLOAT4 DiffuseLight;
	XMFLOAT3 LightDir;
	float count;
	XMFLOAT4 AmbientLight;
	XMFLOAT4 specularLight;
	XMFLOAT4 specularMaterial;
	XMFLOAT3 cameraPosition;
	float specPower;
	std::array<PositionalLight, 16> lights;
	UINT hasTexture;
	UINT hasSpecMap;
};

struct SkyboxConstantBuffer
{
	XMMATRIX Projection;
	XMMATRIX View;
	XMMATRIX World;
};

using namespace DirectX;
//using Microsoft::WRL::ComPtr;

class DX11Framework
{
	int _WindowWidth = 1280;
	int _WindowHeight = 768;

	ID3D11DeviceContext* _immediateContext = nullptr;
	ID3D11Device* _device;
	IDXGIDevice* _dxgiDevice = nullptr;
	IDXGIFactory2* _dxgiFactory = nullptr;
	ID3D11RenderTargetView* _frameBufferView = nullptr;
	IDXGISwapChain1* _swapChain;
	D3D11_VIEWPORT _viewport;

	ID3D11RasterizerState* _fillState;
	ID3D11RasterizerState* _wireframeState;
	ID3D11RasterizerState* _noCullState;
	ID3D11VertexShader* _vertexShader;
	ID3D11VertexShader* _skyboxVertexShader;
	ID3D11InputLayout* _inputLayout;
	ID3D11PixelShader* _pixelShader;
	ID3D11PixelShader* _skyboxPixelShader;
	ID3D11Buffer* _constantBuffer;
	ID3D11Buffer* _skyboxConstantBuffer;

	HWND _windowHandle;

	std::vector<BaseCamera*> _cameras;
	int activeCameraIndex = 0;

	ConstantBuffer _cbData;
	SkyboxConstantBuffer _skyboxCBData;

	ID3D11Texture2D* _depthStencilBuffer;
	ID3D11DepthStencilView* _depthStencilView;
	ID3D11DepthStencilState* _depthStencilState;
	ID3D11DepthStencilState* _depthStencilSkybox;

	bool wireFrameActive = false;

	ID3D11BlendState* _transparency;

	//Lighting
	XMFLOAT4 _diffuseLight;
	XMFLOAT3 _lightDir;

	XMFLOAT4 _ambientLight;

	XMFLOAT4 specularLight;
	XMFLOAT4 specularMaterial;
	XMFLOAT3 cameraPosition;
	float specPower;

	//Hardcoded Objects
	ID3D11Buffer* _cubeVertexBuffer;
	ID3D11Buffer* _pyramidVertexBuffer;
	ID3D11Buffer* _cubeIndexBuffer;
	ID3D11Buffer* _pyramidIndexBuffer;
	float currentCubeDir = 1.0f;
	float timeTilFlip = 1.5f;
	float cubeSpeed = 50.0f;
	XMFLOAT3 _CubePos = XMFLOAT3(0, 1, 0);

	XMFLOAT4X4 _CubeWorld;
	XMFLOAT4X4 _PyramidWorld;

	ID3D11ShaderResourceView* _CrateTexture;

	//GameObjects
	std::map<std::string, Prefab> m_Prefabs;
	std::vector<GameObject> m_GameObjects;
	Skybox m_skybox;

	//Terrain
	Terrain m_terrain;

	//Texture
	ID3D11SamplerState* _bilinearSamplerState;

	//Input
	std::map<char, bool> m_inputKeyStates;

public:
	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);
	HRESULT CreateWindowHandle(HINSTANCE hInstance, int nCmdShow);
	HRESULT CreateD3DDevice();
	HRESULT CreateSwapChainAndFrameBuffer();
	HRESULT InitShadersAndInputLayout();
	HRESULT InitPipelineVariables();
	HRESULT InitRunTimeData();
	HRESULT InitVertexIndexBuffers();
	~DX11Framework();
	void Update();
	void Draw();
	bool JSONLoad();
};