#include "DX11Framework.h"
#include <string>
#include "DDSTextureLoader.h"
#include "json.hpp"
using json = nlohmann::json;
//#define RETURNFAIL(x) if(FAILED(x)) return x;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;


    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

HRESULT DX11Framework::Initialise(HINSTANCE hInstance, int nShowCmd)
{
    HRESULT hr = S_OK;

    hr = CreateWindowHandle(hInstance, nShowCmd);
    if (FAILED(hr)) return E_FAIL;

    hr = CreateD3DDevice();
    if (FAILED(hr)) return E_FAIL;

    hr = CreateSwapChainAndFrameBuffer();
    if (FAILED(hr)) return E_FAIL;

    hr = InitShadersAndInputLayout();
    if (FAILED(hr)) return E_FAIL;

    hr = InitPipelineVariables();
    if (FAILED(hr)) return E_FAIL;

    hr = InitVertexIndexBuffers();
    if(FAILED(hr)) return E_FAIL;

    hr = InitRunTimeData();
    if (FAILED(hr)) return E_FAIL;

    return hr;
}

HRESULT DX11Framework::CreateWindowHandle(HINSTANCE hInstance, int nCmdShow)
{
    const wchar_t* windowName  = L"DX11Framework";

    WNDCLASSW wndClass;
    wndClass.style = 0;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = 0;
    wndClass.hIcon = 0;
    wndClass.hCursor = 0;
    wndClass.hbrBackground = 0;
    wndClass.lpszMenuName = 0;
    wndClass.lpszClassName = windowName;

    RegisterClassW(&wndClass);

    _windowHandle = CreateWindowExW(0, windowName, windowName, WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 
        _WindowWidth, _WindowHeight, nullptr, nullptr, hInstance, nullptr);

    return S_OK;
}

HRESULT DX11Framework::CreateD3DDevice()
{
    HRESULT hr = S_OK;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
    };

    ID3D11Device* baseDevice;
    ID3D11DeviceContext* baseDeviceContext;

    DWORD createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    hr = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT | createDeviceFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &baseDevice, nullptr, &baseDeviceContext);
    if (FAILED(hr)) return hr;

    ///////////////////////////////////////////////////////////////////////////////////////////////

    hr = baseDevice->QueryInterface(__uuidof(ID3D11Device), reinterpret_cast<void**>(&_device));
    hr = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext), reinterpret_cast<void**>(&_immediateContext));

    baseDevice->Release();
    baseDeviceContext->Release();

    ///////////////////////////////////////////////////////////////////////////////////////////////

    hr = _device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&_dxgiDevice));
    if (FAILED(hr)) return hr;

    IDXGIAdapter* dxgiAdapter;
    hr = _dxgiDevice->GetAdapter(&dxgiAdapter);
    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&_dxgiFactory));
    dxgiAdapter->Release();

    return S_OK;
}

HRESULT DX11Framework::CreateSwapChainAndFrameBuffer()
{
    HRESULT hr = S_OK;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    swapChainDesc.Width = 0; // Defer to WindowWidth
    swapChainDesc.Height = 0; // Defer to WindowHeight
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //FLIP* modes don't support sRGB backbuffer
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    hr = _dxgiFactory->CreateSwapChainForHwnd(_device, _windowHandle, &swapChainDesc, nullptr, nullptr, &_swapChain);
    if (FAILED(hr)) return hr;

    ///////////////////////////////////////////////////////////////////////////////////////////////

    ID3D11Texture2D* frameBuffer = nullptr;

    hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&frameBuffer));
    if (FAILED(hr)) return hr;

    D3D11_RENDER_TARGET_VIEW_DESC framebufferDesc = {};
    framebufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; //sRGB render target enables hardware gamma correction
    framebufferDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

    hr = _device->CreateRenderTargetView(frameBuffer, &framebufferDesc, &_frameBufferView);

    D3D11_TEXTURE2D_DESC depthBufferDesc = {};
    frameBuffer->GetDesc(&depthBufferDesc);

    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    _device->CreateTexture2D(&depthBufferDesc, nullptr, &_depthStencilBuffer);
    _device->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

    frameBuffer->Release();


    return hr;
}

HRESULT DX11Framework::InitShadersAndInputLayout()
{
    HRESULT hr = S_OK;
    ID3DBlob* errorBlob;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif
    
    ID3DBlob* vsBlob;

    hr =  D3DCompileFromFile(L"SimpleShaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_main", "vs_5_0", dwShaderFlags, 0, &vsBlob, &errorBlob);
    if (FAILED(hr))
    {
        MessageBoxA(_windowHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
        errorBlob->Release();
        return hr;
    }

    hr = _device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &_vertexShader);

    if (FAILED(hr)) return hr;

   hr = D3DCompileFromFile(L"SkyboxShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_main", "vs_5_0", dwShaderFlags, 0, &vsBlob, &errorBlob);
   if (FAILED(hr))
   {
       MessageBoxA(_windowHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
       errorBlob->Release();
       return hr;
   }

   hr = _device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &_skyboxVertexShader);

    if (FAILED(hr)) return hr;

    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    hr = _device->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &_inputLayout);
    if (FAILED(hr)) return hr;

    ///////////////////////////////////////////////////////////////////////////////////////////////

    ID3DBlob* psBlob;

    hr = D3DCompileFromFile(L"SimpleShaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_main", "ps_5_0", dwShaderFlags, 0, &psBlob, &errorBlob);
    if (FAILED(hr))
    {
        MessageBoxA(_windowHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
        errorBlob->Release();
        return hr;
    }

    hr = _device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &_pixelShader);

    hr = D3DCompileFromFile(L"SkyboxShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_main", "ps_5_0", dwShaderFlags, 0, &psBlob, &errorBlob);
    if (FAILED(hr))
    {
        MessageBoxA(_windowHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
        errorBlob->Release();
        return hr;
    }

    hr = _device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &_skyboxPixelShader);

    m_terrain = Terrain();
    std::string texPath = "Textures\\stone.dds";
    CreateDDSTextureFromFile(_device, std::wstring(texPath.begin(), texPath.end()).c_str(), nullptr, &m_terrain.m_texture);

    m_terrain.GenFlatGrid(_device, 100, 100, 513, 513);
    m_terrain.HeightMapLoad(513, 513, "Heightmaps\\Heightmap 513x513.RAW");

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(SimpleVertex) * m_terrain.m_vertexCount;
    vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexData = { m_terrain.m_vertexArray };

    hr = _device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_terrain.m_vertexBuffer);
    if (FAILED(hr)) { return hr; }

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(UINT32) * m_terrain.m_indexCount;
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexData = { m_terrain.m_indexArray };

    hr = _device->CreateBuffer(&indexBufferDesc, &indexData, &m_terrain.m_indexBuffer);
    if (FAILED(hr)) { return hr; }

    vsBlob->Release();
    psBlob->Release();

    return hr;
}

HRESULT DX11Framework::InitPipelineVariables()
{
    HRESULT hr = S_OK;

    //Input Assembler
    _immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    _immediateContext->IASetInputLayout(_inputLayout);

    //Rasterizer - Fill
    D3D11_RASTERIZER_DESC fillDesc = {};
    fillDesc.FillMode = D3D11_FILL_SOLID;
    fillDesc.CullMode = D3D11_CULL_BACK;

    hr = _device->CreateRasterizerState(&fillDesc, &_fillState);
    if (FAILED(hr)) return hr;

    //Rasterizer - Wireframe
    D3D11_RASTERIZER_DESC wireframeDesc = {};
    wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
    wireframeDesc.CullMode = D3D11_CULL_NONE;

    hr = _device->CreateRasterizerState(&wireframeDesc, &_wireframeState);
    if (FAILED(hr)) return hr;

    //Rasterizer - No Cull
    D3D11_RASTERIZER_DESC skyboxDesc = {};
    skyboxDesc.FillMode = D3D11_FILL_SOLID;
    skyboxDesc.CullMode = D3D11_CULL_NONE;

    hr = _device->CreateRasterizerState(&skyboxDesc, &_noCullState);
    if (FAILED(hr)) return hr;

    _immediateContext->RSSetState(_fillState);


    D3D11_DEPTH_STENCIL_DESC dsDescSkybox = { };
    dsDescSkybox.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDescSkybox.DepthEnable = true;
    dsDescSkybox.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; 

    _device->CreateDepthStencilState(&dsDescSkybox, &_depthStencilSkybox);

    //Viewport Values
    _viewport = { 0.0f, 0.0f, (float)_WindowWidth, (float)_WindowHeight, 0.0f, 1.0f };
    _immediateContext->RSSetViewports(1, &_viewport);

    //Constant Buffer
    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = _device->CreateBuffer(&constantBufferDesc, nullptr, &_constantBuffer);
    if (FAILED(hr)) { return hr; }

    //Constant Buffer
    D3D11_BUFFER_DESC skyboxConstantBufferDesc = {};
    skyboxConstantBufferDesc.ByteWidth = sizeof(ConstantBuffer);
    skyboxConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    skyboxConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    skyboxConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = _device->CreateBuffer(&skyboxConstantBufferDesc, nullptr, &_skyboxConstantBuffer);
    if (FAILED(hr)) { return hr; }

    _immediateContext->VSSetConstantBuffers(0, 1, &_constantBuffer);
    _immediateContext->PSSetConstantBuffers(0, 1, &_constantBuffer);

    //Sampler
    D3D11_SAMPLER_DESC bilinearSamplerDesc = {};
    bilinearSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    bilinearSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    bilinearSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    bilinearSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    bilinearSamplerDesc.MaxLOD = 1;
    bilinearSamplerDesc.MinLOD = 0;

    hr = _device->CreateSamplerState(&bilinearSamplerDesc, &_bilinearSamplerState);
    if (FAILED(hr)) return hr;

    _immediateContext->PSSetSamplers(0, 1, &_bilinearSamplerState);

    D3D11_RENDER_TARGET_BLEND_DESC rtbd = {};
    rtbd.BlendEnable = true;
    rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
    rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
    rtbd.BlendOp = D3D11_BLEND_OP_ADD;
    rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
    rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
    rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.RenderTarget[0] = rtbd;

    _device->CreateBlendState(&blendDesc, &_transparency);

    return S_OK;
}

HRESULT DX11Framework::InitRunTimeData()
{
    HRESULT hr = S_OK;

    //HardCoded Objects
    XMStoreFloat4x4(&_PyramidWorld, XMMatrixIdentity() * XMMatrixTranslation(0, -3.0f, 0));
    XMStoreFloat4x4(&_CubeWorld, XMMatrixIdentity() * XMMatrixTranslation(0, 1.0f, 0));
    std::string crateTexPath = "Textures\\Crate_COLOR.dds";
    CreateDDSTextureFromFile(_device, std::wstring(crateTexPath.begin(), crateTexPath.end()).c_str(), nullptr, &_CrateTexture);

    //Skybox
    m_skybox = Skybox();
    m_skybox.m_vertexShader = _skyboxVertexShader;
    m_skybox.m_pixelShader = _skyboxPixelShader;
    std::string objPath = "Meshes\\cube.obj";
    std::string texPath = "Textures\\skybox.dds";
    m_skybox.m_meshData = OBJLoader::Load(objPath.c_str(), _device);
    CreateDDSTextureFromFile(_device, std::wstring(texPath.begin(), texPath.end()).c_str(), nullptr, &m_skybox.m_texture);

    //Camera
    _cameras = std::vector<BaseCamera*>();
    _cameras.push_back(new DebugFlyCamera(XMFLOAT3(0, 0, -6.0f), XMFLOAT3(0, 0, 1), XMFLOAT3(0, 1, 0)));
    _cameras.push_back(new BaseCamera(XMFLOAT3(-6.0f, 0, 0), XMFLOAT3(1, 0, 0), XMFLOAT3(0, 1, 0)));
    _cameras.push_back(new FollowObjectCamera(XMFLOAT3(0, 10.0f, 0.0f), XMFLOAT3(0, 0, 1), &_CubePos));

    //Lighting
    _diffuseLight = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    _lightDir = XMFLOAT3(-0.1f, -0.8f, -0.3f);

    _ambientLight = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);

    specularLight = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    specularMaterial = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    specPower = 10.0f;
    
    cameraPosition = _cameras[activeCameraIndex]->GetEye();

    m_inputKeyStates = std::map<char, bool>();
    //PreviousCamera
    m_inputKeyStates['Q'] = false;
    m_inputKeyStates['E'] = false;
    
    if (!JSONLoad())
    {
        bool i = false;
    }


    return hr;
}

HRESULT DX11Framework::InitVertexIndexBuffers()
{
    HRESULT hr = S_OK;

    SimpleVertex CubeVertexData[] =
    {
        //Position                     //Normal           
        {XMFLOAT3(-1.00f,  1.00f, -1.0f), XMFLOAT3(-1.00f,  1.00f, -1.0f), XMFLOAT2(1.0f, 1.0f)}, //Top Right Front
        {XMFLOAT3(1.00f,  1.00f, -1.0f), XMFLOAT3(1.00f,  1.00f, -1.0f), XMFLOAT2(1.0f, 0)}, //Bottom Right Front
        {XMFLOAT3(-1.00f, -1.00f, -1.0f), XMFLOAT3(-1.00f, -1.00f, -1.0f), XMFLOAT2(0, 1.0f)}, //Top Left Front
        {XMFLOAT3(1.00f, -1.00f, -1.0f), XMFLOAT3(1.00f, -1.00f, -1.0f), XMFLOAT2()}, //Bottom Left Front
        {XMFLOAT3(-1.00f,  1.00f, 1.0f), XMFLOAT3(-1.00f,  1.00f, 1.0f), XMFLOAT2(0, 1.0f)}, //Top Right Back
        {XMFLOAT3(1.00f,  1.00f, 1.0f), XMFLOAT3(1.00f,  1.00f, 1.0f), XMFLOAT2(0, 0)}, //Bottom Right Back
        {XMFLOAT3(-1.00f, -1.00f, 1.0f), XMFLOAT3(-1.00f, -1.00f, 1.0f), XMFLOAT2(1.0f, 1.0f)}, //Top Left Back
        {XMFLOAT3(1.00f, -1.00f, 1.0f), XMFLOAT3(1.00f, -1.00f, 1.0f), XMFLOAT2(1.0f, 0)}, //Bottom Left Back
    };

    D3D11_BUFFER_DESC cubeVertexBufferDesc = {};
    cubeVertexBufferDesc.ByteWidth = sizeof(CubeVertexData);
    cubeVertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    cubeVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA _cubeVertexData = { CubeVertexData };

    hr = _device->CreateBuffer(&cubeVertexBufferDesc, &_cubeVertexData, &_cubeVertexBuffer);
    if (FAILED(hr)) return hr;

    ///////////////////////////////////////////////////////////////////////////////////////////////

    WORD IndexData[] =
    {
        //Indices
        0, 1, 2,
        2, 1, 3,
        6, 5, 4,
        7, 5, 6,
        0, 2, 4,
        4, 2, 6,
        5, 3, 1,
        7, 3, 5,
        4, 1, 0,
        5, 1, 4,
        2, 3, 6,
        6, 3, 7,
    };

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.ByteWidth = sizeof(IndexData);
    indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA indexData = { IndexData };

    hr = _device->CreateBuffer(&indexBufferDesc, &indexData, &_cubeIndexBuffer);
    if (FAILED(hr)) return hr;

    ////////////////////// Pyramid ///////////////////////////

    SimpleVertex pyramidVertexData[] =
    {
        //Position                     //Normal             
        { XMFLOAT3(-1.00f,  -1.00f, 1.0f), XMFLOAT3(-1.00f,  -1.00f, 1.0f), XMFLOAT2(0, 0)}, //Base Back Left
        { XMFLOAT3(1.00f,  -1.00f, 1.0f),  XMFLOAT3(1.00f,  -1.00f, 1.0f), XMFLOAT2(0, 0)}, //Base Back Right
        { XMFLOAT3(-1.00f, -1.00f, -1.0f), XMFLOAT3(-1.00f, -1.00f, -1.0f), XMFLOAT2(0, 0)}, //Base Front Left
        { XMFLOAT3(1.00f, -1.00f, -1.0f),  XMFLOAT3(1.00f, -1.00f, -1.0f), XMFLOAT2(0, 0)}, //Base Front Right
        { XMFLOAT3(0.00f,  1.00f, 0.0f),  XMFLOAT3(0.00f,  1.00f, 0.0f), XMFLOAT2(0, 0)}, //Top
    };

    D3D11_BUFFER_DESC pyramidVertexBufferDesc = {};
    pyramidVertexBufferDesc.ByteWidth = sizeof(pyramidVertexData);
    pyramidVertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    pyramidVertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA _pyramidVertexData = { pyramidVertexData };

    hr = _device->CreateBuffer(&pyramidVertexBufferDesc, &_pyramidVertexData, &_pyramidVertexBuffer);
    if (FAILED(hr)) return hr;

    //Pyramid Indices

    WORD pyramidIndexData[] =
    {
        //Indices
        2, 1, 0,
        3, 1, 2,
        3, 4, 1,
        1, 4, 0,
        0, 4, 2, 
        2, 4, 3
    };

    D3D11_BUFFER_DESC pyramidIndexBufferDesc = {};
    pyramidIndexBufferDesc.ByteWidth = sizeof(pyramidIndexData);
    pyramidIndexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    pyramidIndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA _pyramidIndexData = { pyramidIndexData };

    hr = _device->CreateBuffer(&pyramidIndexBufferDesc, &_pyramidIndexData, &_pyramidIndexBuffer);
    if (FAILED(hr)) return hr;

    return S_OK;
}

DX11Framework::~DX11Framework()
{
    if(_immediateContext)_immediateContext->Release();
    if(_device)_device->Release();
    if(_dxgiDevice)_dxgiDevice->Release();
    if(_dxgiFactory)_dxgiFactory->Release();
    if(_frameBufferView)_frameBufferView->Release();
    if(_swapChain)_swapChain->Release();

    if(_fillState)_fillState->Release();
    if (_wireframeState)_wireframeState->Release();
    if (_noCullState)_noCullState->Release();
    if(_vertexShader)_vertexShader->Release();
    if(_inputLayout)_inputLayout->Release();
    if(_pixelShader)_pixelShader->Release();
    if(_constantBuffer)_constantBuffer->Release();
    if (_depthStencilBuffer)_depthStencilBuffer->Release();
    if (_depthStencilView)_depthStencilView->Release();

    if (_cubeVertexBuffer)_cubeVertexBuffer->Release();
    if (_pyramidVertexBuffer)_pyramidVertexBuffer->Release();
    if (_cubeIndexBuffer)_cubeIndexBuffer->Release();
    if (_pyramidIndexBuffer)_pyramidIndexBuffer->Release();
}


void DX11Framework::Update()
{
    //Static initializes this value only once    
    static ULONGLONG frameStart = GetTickCount64();

    ULONGLONG frameNow = GetTickCount64();
    float deltaTime = (frameNow - frameStart) / 1000.0f;
    frameStart = frameNow;

    static float simpleCount = 0.0f;
    simpleCount += deltaTime;

    timeTilFlip -= deltaTime;
    if (timeTilFlip <= 0.0f)
    {
        timeTilFlip = 3.0f;
        currentCubeDir *= -1.0f;
    }

    XMStoreFloat4x4(&_CubeWorld, XMMatrixIdentity() * XMMatrixTranslation(_CubePos.x + (currentCubeDir * cubeSpeed * deltaTime/10.0f), 0.0f, 0.0f));
    XMMATRIX cubeTransform = XMLoadFloat4x4(&_CubeWorld);

    _CubePos = XMFLOAT3(cubeTransform.r[3].m128_f32[0], cubeTransform.r[3].m128_f32[1], cubeTransform.r[3].m128_f32[2]);

    XMStoreFloat4x4(&_PyramidWorld, XMMatrixIdentity() * XMMatrixRotationX(simpleCount) * XMMatrixTranslation(0, -3.0f, 0));

    for (int i = 0; i < m_GameObjects.size(); i++)
    {
        m_GameObjects[i].Update(deltaTime);
    }

    _cbData.count = simpleCount;

    if (GetAsyncKeyState(VK_LBUTTON) & 0x01)
    {
        if (wireFrameActive)
        {
            _immediateContext->RSSetState(_fillState);
            wireFrameActive = false;
        }
        else
        {
            _immediateContext->RSSetState(_wireframeState);
            wireFrameActive = true;
        }
    }

    if (GetKeyState('E') < 0)
    {
        if (!m_inputKeyStates['E'])
        {
            m_inputKeyStates['E'] = true;
            activeCameraIndex += 1;
            if (activeCameraIndex >= _cameras.size())
            {
                activeCameraIndex = 0;
            }
        }
    }
    else
    {
        m_inputKeyStates['E'] = false;
    }

    if (GetKeyState('Q') < 0)
    {
        if (!m_inputKeyStates['Q'])
        {
            m_inputKeyStates['Q'] = true;
            activeCameraIndex -= 1;
            if (activeCameraIndex < 0)
            {
                activeCameraIndex = _cameras.size() - 1;
            }
        }
    }
    else
    {
        m_inputKeyStates['Q'] = false;
    }

    _cameras[activeCameraIndex]->Update(deltaTime);
    cameraPosition = _cameras[activeCameraIndex]->GetEye();
    m_skybox.m_position = cameraPosition;
    XMStoreFloat4x4(&m_skybox.m_world, XMMatrixIdentity() * XMMatrixScaling(m_skybox.m_size.x, m_skybox.m_size.y, m_skybox.m_size.z) * XMMatrixTranslation(cameraPosition.x, cameraPosition.y, cameraPosition.z));
}

void DX11Framework::Draw()
{
    //Present unbinds render target, so rebind and clear at start of each frame
    float backgroundColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
    _immediateContext->OMSetRenderTargets(1, &_frameBufferView, _depthStencilView);
    _immediateContext->ClearRenderTargetView(_frameBufferView, backgroundColor);
    _immediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0.0f);

    //Store this frames data in constant buffer struct
    XMFLOAT4X4 tempView = _cameras[activeCameraIndex]->GetView();
    XMFLOAT4X4 tempProj = _cameras[activeCameraIndex]->GetProjection();
    _cbData.View = XMMatrixTranspose(XMLoadFloat4x4(&tempView));
    _cbData.Projection = XMMatrixTranspose(XMLoadFloat4x4(&tempProj));
    _skyboxCBData.View = XMMatrixTranspose(XMLoadFloat4x4(&tempView));
    _skyboxCBData.Projection = XMMatrixTranspose(XMLoadFloat4x4(&tempProj));

    //Lighting
    _cbData.DiffuseLight = _diffuseLight;
    _cbData.LightDir = _lightDir;

    _cbData.AmbientLight = _ambientLight;

    _cbData.specularLight = specularLight;
    _cbData.specularMaterial = specularMaterial;
    _cbData.cameraPosition = cameraPosition;
    _cbData.specPower = specPower;

    //Write constant buffer data onto GPU
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;

    //Set object variables and draw
    UINT stride = { sizeof(SimpleVertex) };
    UINT offset = 0;
    _immediateContext->VSSetShader(_vertexShader, nullptr, 0);
    _immediateContext->PSSetShader(_pixelShader, nullptr, 0);

    _cbData.hasSpecMap = false;


   for (GameObject object : m_GameObjects)
    {
        _cbData.hasSpecMap = object.m_hasSpecMap;
        _immediateContext->Map(_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
        XMFLOAT4X4 _objWorld = object.GetWorld();
        _cbData.World = XMMatrixTranspose(XMLoadFloat4x4(&_objWorld));
        memcpy(mappedSubresource.pData, &_cbData, sizeof(_cbData));
        _immediateContext->Unmap(_constantBuffer, 0);

        if (object.m_objectType == "TranslucentCrate")
        {
            FLOAT blendFactor[4] = { 0.75f, 0.75f, 0.75f, 1.0f };
            _immediateContext->OMSetBlendState(_transparency, blendFactor, 0xffffffff);
        }
        else
        {
            _immediateContext->OMSetBlendState(0, 0, 0xffffffff);
        }

        if (object.m_objectType == "Fence" || object.m_objectType == "Tree")
        {
            _immediateContext->RSSetState(_noCullState);
        }
        else
        {
            if (wireFrameActive)
            {
                _immediateContext->RSSetState(_wireframeState);
            }
            else
            {
                _immediateContext->RSSetState(_fillState);
            }
        }
        object.Draw(_immediateContext);
    }

   _immediateContext->OMSetBlendState(0, 0, 0xffffffff);

   //HardCoded Objects
   _cbData.hasSpecMap = false;
   _immediateContext->VSSetShader(_vertexShader, nullptr, 0);
   _immediateContext->PSSetShader(_pixelShader, nullptr, 0);
   _immediateContext->PSSetShaderResources(0, 1, &_CrateTexture);

   _immediateContext->IASetVertexBuffers(0, 1, &_cubeVertexBuffer, &stride, &offset);
   _immediateContext->IASetIndexBuffer(_cubeIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
   _immediateContext->VSSetShader(_vertexShader, nullptr, 0);
   _immediateContext->PSSetShader(_pixelShader, nullptr, 0);
   _immediateContext->Map(_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
   _cbData.World = XMMatrixTranspose(XMLoadFloat4x4(&_CubeWorld));
   memcpy(mappedSubresource.pData, &_cbData, sizeof(_cbData));
   _immediateContext->Unmap(_constantBuffer, 0);
   _immediateContext->DrawIndexed(36, 0, 0);

   
   _immediateContext->IASetVertexBuffers(0, 1, &_pyramidVertexBuffer, &stride, &offset);
   _immediateContext->IASetIndexBuffer(_pyramidIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
   _immediateContext->Map(_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
   _cbData.World = XMMatrixTranspose(XMLoadFloat4x4(&_PyramidWorld));
   memcpy(mappedSubresource.pData, &_cbData, sizeof(_cbData));
   _immediateContext->Unmap(_constantBuffer, 0);
   _immediateContext->DrawIndexed(18, 0, 0);

   _cbData.specularLight = XMFLOAT4(0, 0, 0, 0);
   _cbData.specularMaterial = XMFLOAT4( 0, 0, 0, 0 );

   //Terrain
   _immediateContext->Map(_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
   XMFLOAT4X4 _terrainWorld = m_terrain.GetWorld();
   _cbData.World = XMMatrixTranspose(XMLoadFloat4x4(&_terrainWorld));
   memcpy(mappedSubresource.pData, &_cbData, sizeof(_cbData));
   _immediateContext->Unmap(_constantBuffer, 0);
   m_terrain.Draw(_immediateContext);

   //Skybox
   _immediateContext->Map(_skyboxConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
    _skyboxCBData.World = XMMatrixTranspose(XMLoadFloat4x4(&m_skybox.m_world));
   memcpy(mappedSubresource.pData, &_skyboxCBData, sizeof(_skyboxCBData));
   _immediateContext->Unmap(_skyboxConstantBuffer, 0);
   
    _immediateContext->RSSetState(_noCullState);
    _immediateContext->VSSetShader(_skyboxVertexShader, nullptr, 0);
    _immediateContext->PSSetShader(_skyboxPixelShader, nullptr, 0);
    _immediateContext->OMSetDepthStencilState(_depthStencilSkybox, 0);
    _immediateContext->PSSetShaderResources(0, 1, &m_skybox.m_texture);
    _immediateContext->PSSetConstantBuffers(0, 1, &_skyboxConstantBuffer);
    _immediateContext->VSSetConstantBuffers(0, 1, &_skyboxConstantBuffer);

    _immediateContext->IASetVertexBuffers(0, 1, &m_skybox.m_meshData.m_vertexBuffer, &m_skybox.m_meshData.m_VBStride, &m_skybox.m_meshData.m_VBOffset);
    _immediateContext->IASetIndexBuffer(m_skybox.m_meshData.m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    _immediateContext->DrawIndexed(m_skybox.m_meshData.m_indexCount, 0, 0);
    _immediateContext->PSSetConstantBuffers(0, 1, &_constantBuffer);
    _immediateContext->VSSetConstantBuffers(0, 1, &_constantBuffer);

    _immediateContext->OMSetDepthStencilState(NULL, 0);

    //Present Backbuffer to screen
    _swapChain->Present(0, 0);
}

bool DX11Framework::JSONLoad()
{
    std::array<PositionalLight, 16> lightList = { PositionalLight(), PositionalLight(), PositionalLight(), PositionalLight(),
                                      PositionalLight(), PositionalLight(), PositionalLight(), PositionalLight(),
                                      PositionalLight(), PositionalLight(), PositionalLight(), PositionalLight(),
                                      PositionalLight(), PositionalLight(), PositionalLight(), PositionalLight() };

                                      
    //Json Parser
    json jFile;

    std::ifstream fileOpen("SceneData.json");

    jFile = json::parse(fileOpen);

    //Lights

    json& lights = jFile["Lights"];
    int size = lights.size();
    for (int i = 0; i < size; i++)
    {
        PositionalLight L;
        json& objectDesc = lights.at(i);

        L.m_position = XMFLOAT3(objectDesc["PositionX"], objectDesc["PositionY"], objectDesc["PositionZ"]);
        L.m_enabled = objectDesc["Enabled"];
        L.m_colour = XMFLOAT4(objectDesc["ColourR"], objectDesc["ColourG"], objectDesc["ColourB"], objectDesc["ColourA"]);
        L.m_direction = XMFLOAT3(objectDesc["DirectionX"], objectDesc["DirectionY"], objectDesc["DirectionZ"]);
        L.m_maxDistance = objectDesc["MaxDistance"];

        lightList[i] = L;
    }
    _cbData.lights = lightList;

    //Prefabs
    m_Prefabs = std::map<std::string, Prefab>();

    json& prefabs = jFile["ObjectData"];
    int prefabSize = prefabs.size();
    for (int i = 0; i < prefabSize; i++)
    {
        json& prefabDesc = prefabs.at(i);
        
        std::string prefabName = prefabDesc["Name"];
        std::string meshDataPath = prefabDesc["MeshData"];
        std::string texturePath = prefabDesc["Texture"];
        std::string specMapTexturePath = prefabDesc["SpecMap"];

        m_Prefabs[prefabName] = Prefab(meshDataPath, texturePath, specMapTexturePath);

    }

    //Objects
    m_GameObjects = std::vector<GameObject>();

    json& objects = jFile["SceneObjects"];
    int objectSize = objects.size();

    for (int i = 0; i < objectSize; i++)
    {
        json& objectDesc = objects.at(i);
        
        std::string prefabName = objectDesc["ObjectName"];
        XMFLOAT3 position = { objectDesc["PositionX"] , objectDesc["PositionY"] , objectDesc["PositionZ"] };
        XMFLOAT3 rotation = { objectDesc["RotationX"] , objectDesc["RotationY"] , objectDesc["RotationZ"] };
        XMFLOAT3 scale = { objectDesc["ScaleX"] , objectDesc["ScaleY"] , objectDesc["ScaleZ"] };

        GameObject tempObject = GameObject(m_Prefabs[prefabName], position, rotation, scale, _device, prefabName);
        if (prefabName == "Tree")
        {
            tempObject.m_isRotating = true;
            tempObject.m_rotationSpeed.y = 2.0f;
        }
        m_GameObjects.push_back(tempObject);
    }

    fileOpen.close();

    return true;
}