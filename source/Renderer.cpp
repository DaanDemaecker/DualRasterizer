#include "pch.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Texture.h"
#include "Utils.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//calculate aspect ratio
		m_AspectRatio = static_cast<float>(m_Width) / m_Height;

		//Initialize Camera
		m_Camera.Initialize(45.f, { .0f,.0f, -20.f }, m_AspectRatio);

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		LoadMeshes();
	}

	void Renderer::LoadMeshes()
	{
		//Create some data for our mesh
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		Utils::ParseOBJ("resources/Vehicle.obj", vertices, indices);

		m_pVehicleMesh = new Mesh{ m_pDevice, vertices, indices, EffectType::Shaded };

		m_pDiffuseTextureVehicle = Texture::LoadFromFile("Resources/vehicle_diffuse.png", m_pDevice);
		m_pGlossMap = Texture::LoadFromFile("Resources/vehicle_gloss.png", m_pDevice);
		m_pNormalMap = Texture::LoadFromFile("Resources/vehicle_normal.png", m_pDevice);
		m_pSpecularMap = Texture::LoadFromFile("Resources/vehicle_specular.png", m_pDevice);

		m_pVehicleMesh->SetDiffuseMap(m_pDiffuseTextureVehicle);
		m_pVehicleMesh->SetGlossmap(m_pGlossMap);
		m_pVehicleMesh->SetNormalMap(m_pNormalMap);
		m_pVehicleMesh->SetSpecularMap(m_pSpecularMap);


		Utils::ParseOBJ("resources/fireFX.obj", vertices, indices);

		m_pFireMesh = new Mesh{ m_pDevice, vertices, indices, EffectType::Transparent };
		m_pDiffuseTextureFire = Texture::LoadFromFile("Resources/fireFX_diffuse.png", m_pDevice);

		m_pFireMesh->SetDiffuseMap(m_pDiffuseTextureFire);
	}

	Renderer::~Renderer()
	{
		if (m_pRenderTargetView)
		{
			m_pRenderTargetView->Release();
		}

		if (m_pRenderTargetBuffer)
		{
			m_pRenderTargetBuffer->Release();
		}

		if (m_pDepthStencilView)
		{
			m_pDepthStencilView->Release();
		}

		if (m_pDepthStencilBuffer)
		{
			m_pDepthStencilBuffer->Release();
		}

		if (m_pSwapChain)
		{
			m_pSwapChain->Release();
		}

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}

		if (m_pDevice)
		{
			m_pDevice->Release();
		}

		delete m_pVehicleMesh;
		delete m_pDiffuseTextureVehicle;
		delete m_pGlossMap;
		delete m_pNormalMap;
		delete m_pSpecularMap;

		delete m_pFireMesh;
		delete m_pDiffuseTextureFire;
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);

		const float rotationSpeed = 1.f;
		//m_WorldMatrix = Matrix::CreateRotationY(rotationSpeed * pTimer->GetElapsed()) * m_WorldMatrix;

		m_pVehicleMesh->SetMatrices(m_WorldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix, m_WorldMatrix, m_Camera.invViewMatrix);

		m_pFireMesh->SetMatrices(m_WorldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix, m_WorldMatrix, m_Camera.invViewMatrix);
	}


	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		//1. CLEAR RTV & DSV
		ColorRGB clearColor = ColorRGB{ 0.f, 0.f, 0.3f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. SET PIPELINE + INVOKE DRAWCALLS (= RENDER)
		//...
		m_pVehicleMesh->Render(m_pDeviceContext);
		m_pFireMesh->Render(m_pDeviceContext);

		//3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);
	}

	void Renderer::ToggleFilterState()
	{
		m_CurrentFilterState = static_cast<FilterState>((int(m_CurrentFilterState) + 1) % 3);

		switch (m_CurrentFilterState)
		{
		case FilterState::Point:
			std::cout << "SAMPLER_STATE = POINT\n";
			break;
		case FilterState::Linear:
			std::cout << "SAMPLER_STATE = LINEAR\n";
			break;
		case FilterState::Anisotropic:
			std::cout << "SAMPLER_STATE = ANISOTROPIC\n";
			break;
		default:
			break;
		}

		m_pVehicleMesh->ToggleFilter(m_CurrentFilterState);
		m_pFireMesh->ToggleFilter(m_CurrentFilterState);

	}

	HRESULT Renderer::InitializeDirectX()
	{
		//1. Create Device & DeviceContext
		//====
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
			1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);
		if (FAILED(result))
			return result;


		//Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
			return result;


		//2. Create Swapchain
		//====
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;


		//Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version)
			SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create SwapChain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
			return result;

		//3. Create DepthStancil (DS) & DepthStencilView (DSV)
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
			return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
			return result;

		//4. Create RenderTarget (RT) & RenderTargetView (RTV)
		//====

		//Resource
		result == m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
			return result;

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
			return result;


		//5. Bind RTV & DSV to output Merger Stage
		//====
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6. Set Viewport
		//====
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return result;

	}

}
