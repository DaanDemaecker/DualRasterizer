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
		m_Camera.Initialize(45.f, { .0f,.0f, 0.f }, m_AspectRatio);

		//Create Buffers
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

		m_pDepthBufferPixels = new float[m_Width * m_Height];

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			//std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		LoadMeshes();

		m_WorldMatrix = Matrix::CreateTranslation(0.f, 0.f, 50.f);

		PrintControls();
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

		m_pVehicleMesh->SetTopology(dae::PrimitiveTopology::TriangeList);


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

		delete[] m_pDepthBufferPixels;
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);

		const float rotationSpeed = 1.f;
		if(m_RotationEnabled)
			m_WorldMatrix = Matrix::CreateRotationY(rotationSpeed * pTimer->GetElapsed()) * m_WorldMatrix;

		m_pVehicleMesh->SetMatrices(m_WorldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix, m_WorldMatrix, m_Camera.invViewMatrix);

		m_pFireMesh->SetMatrices(m_WorldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix, m_WorldMatrix, m_Camera.invViewMatrix);
	}


	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		switch (m_RasterizerMode)
		{
		case RasterizerMode::Hardware:
			RenderHardware();
			break;
		case RasterizerMode::Software:
			RenderSoftware();
			break;
		default:
			break;
		}
		
	}


	void Renderer::RenderHardware() const
	{
		//1. CLEAR RTV & DSV
		ColorRGB clearColor = ColorRGB{ .39f, .59f, .93f};
		if (m_UniformClearColor)
			clearColor = ColorRGB{ 0.14f, 0.14f, 0.14f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. SET PIPELINE + INVOKE DRAWCALLS (= RENDER)
		//...
		m_pVehicleMesh->Render(m_pDeviceContext);
		if(m_RenderFire)
			m_pFireMesh->Render(m_pDeviceContext);

		//3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);
	}

	void Renderer::RenderSoftware() const
	{
		//@START
		if(m_UniformClearColor)
			SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 36, 36, 36));
		else
			SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));
		std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);
		//Lock BackBuffer
		SDL_LockSurface(m_pBackBuffer);

		RenderMesh(m_pVehicleMesh);

		//@END
		//Update SDL Surface
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);
	}

	void dae::Renderer::RenderMesh(Mesh* mesh) const
	{
		VertexTransformationFunction(mesh);

		std::vector<Vector2> vertices_ScreenSpace{};

		auto& vertices_out{ mesh->GetVerticesOut() };

		for (const auto& vertex : vertices_out)
		{
			vertices_ScreenSpace.push_back(
				{
					(vertex.position.x + 1) / 2.0f * m_Width,
					(1.0f - vertex.position.y) / 2.0f * m_Height
				});
		}

		Triangle triangle{};

		auto& indices{ mesh->GetIndices() };
		switch (mesh->GetTopology())
		{
		case PrimitiveTopology::TriangeList:
			for (int i{}; i < indices.size(); i += 3)
			{
				if (CalculateTriangle(triangle, mesh, vertices_ScreenSpace, i))
				{
					RenderTriangle(triangle);
				}
			}
			break;
		case PrimitiveTopology::TriangleStrip:
			for (int i{}; i < indices.size() - 2; i++)
			{
				if (CalculateTriangle(triangle, mesh, vertices_ScreenSpace, i, (i % 2) == 1))
				{
					RenderTriangle(triangle);
				}
			}
			break;
		default:
			break;
		}
	}

	bool dae::Renderer::CalculateTriangle(Triangle& triangle, Mesh* mesh, std::vector<Vector2>& vertices_ScreenSpace, int startIdx, bool flipTriangle) const
	{
		auto& indices{ mesh->GetIndices() };
		auto& vertices_out{ mesh->GetVerticesOut() };

		const uint32_t index0{ indices[startIdx] };
		const uint32_t index1{ indices[startIdx + 1 + 1 * flipTriangle] };
		const uint32_t index2{ indices[startIdx + 1 + 1 * !flipTriangle] };

		if (index0 == index1 || index1 == index2 || index2 == index0)return false;

		triangle.screen[0] = { vertices_ScreenSpace[index0] };
		triangle.screen[1] = { vertices_ScreenSpace[index1] };
		triangle.screen[2] = { vertices_ScreenSpace[index2] };

		triangle.ndc[0] = vertices_out[index0];
		triangle.ndc[1] = vertices_out[index1];
		triangle.ndc[2] = vertices_out[index2];

		triangle.boundingBox = GetBoundingBox(triangle.screen[0], triangle.screen[1], triangle.screen[2]);

		triangle.CheckOutsideFrustum(m_Camera);

		return true;
	}

	void dae::Renderer::RenderTriangle(const Triangle& triangle) const
	{
		if (triangle.isOutsideFrustum[0] ||
			triangle.isOutsideFrustum[1] ||
			triangle.isOutsideFrustum[2])
		{
			return;
		}

		const Vector2 edgeV0V1{ triangle.screen[1] - triangle.screen[0] };
		const Vector2 edgeV1V2{ triangle.screen[2] - triangle.screen[1] };
		const Vector2 edgeV2V0{ triangle.screen[0] - triangle.screen[2] };

		const float inverseTriangleArea{ 1.f / Vector2::Cross(edgeV1V2,edgeV2V0) };

		ColorRGB finalColor{};

		for (int px{ triangle.boundingBox.minX }; px < triangle.boundingBox.maxX; ++px)
		{
			for (int py{ triangle.boundingBox.minY }; py < triangle.boundingBox.maxY; ++py)
			{
				const int pixelIdx{ px + py * m_Width };

				if (m_RenderBoundingBox)
				{
					finalColor = ColorRGB{ 1, 1, 1 };

					m_pBackBufferPixels[pixelIdx] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));

					continue;
				}

				const Vector2 point{ static_cast<float>(px), static_cast<float>(py) };

				const Vector2 v0ToPoint{ point - triangle.screen[0] };
				const Vector2 v1ToPoint{ point - triangle.screen[1] };
				const Vector2 v2ToPoint{ point - triangle.screen[2] };

				// Calculate cross product from edge to start to point
				const float edge01PointCross{ Vector2::Cross(edgeV0V1, v0ToPoint) };
				const float edge12PointCross{ Vector2::Cross(edgeV1V2, v1ToPoint) };
				const float edge20PointCross{ Vector2::Cross(edgeV2V0, v2ToPoint) };

				if (!(edge01PointCross >= 0 && edge12PointCross >= 0 && edge20PointCross >= 0)) continue;

				const float weightV0{ edge12PointCross * inverseTriangleArea };
				const float weightV1{ edge20PointCross * inverseTriangleArea };
				const float weightV2{ edge01PointCross * inverseTriangleArea };

				float interpolatedZDepth
				{
					1.0f /
							(weightV0 / triangle.ndc[0].position.z +
							weightV1 / triangle.ndc[1].position.z +
							weightV2 / triangle.ndc[2].position.z)
				};

				if (interpolatedZDepth < 0.0f || interpolatedZDepth > 1.0f ||
					m_pDepthBufferPixels[pixelIdx] < interpolatedZDepth)
					continue;

				m_pDepthBufferPixels[pixelIdx] = interpolatedZDepth;

				if (!m_RenderDepth)
				{
					const float interpolatedWDepth = 1.0f /
						(weightV0 / triangle.ndc[0].position.w +
							weightV1 / triangle.ndc[1].position.w +
							weightV2 / triangle.ndc[2].position.w);

					Pixel_Out pixelOut{ Vector4{float(px), float(py), interpolatedZDepth, interpolatedWDepth} };

					pixelOut.uv = ((weightV0 * triangle.ndc[0].uv / triangle.ndc[0].position.w) +
						(weightV1 * triangle.ndc[1].uv / triangle.ndc[1].position.w) +
						(weightV2 * triangle.ndc[2].uv / triangle.ndc[2].position.w)) * interpolatedWDepth;

					if (pixelOut.uv.x < 0 || pixelOut.uv.x > 1 ||
						pixelOut.uv.y < 0 || pixelOut.uv.y > 1)
						std::cout << "fuck why \n";

					pixelOut.normal =
					{
						(((weightV0 * triangle.ndc[0].normal / triangle.ndc[0].position.w) +
						(weightV1 * triangle.ndc[1].normal / triangle.ndc[1].position.w) +
						(weightV2 * triangle.ndc[2].normal / triangle.ndc[2].position.w)) * interpolatedWDepth)
					};
					pixelOut.normal.Normalize();

					pixelOut.tangent =
					{
						(((weightV0 * triangle.ndc[0].tangent / triangle.ndc[0].position.w) +
						(weightV1 * triangle.ndc[1].tangent / triangle.ndc[1].position.w) +
						(weightV2 * triangle.ndc[2].tangent / triangle.ndc[2].position.w)) * interpolatedWDepth)
					};

					pixelOut.viewDirection =
					{
						(((weightV0 * triangle.ndc[0].viewDirection / triangle.ndc[0].position.w) +
						(weightV1 * triangle.ndc[1].viewDirection / triangle.ndc[1].position.w) +
						(weightV2 * triangle.ndc[2].viewDirection / triangle.ndc[2].position.w)) * interpolatedWDepth)
					};

					finalColor = PixelShading(pixelOut);
				}
				else
				{
					const float depthColor{ Remap(interpolatedZDepth, 0.997f, 1.0f) };

					finalColor = { depthColor, depthColor , depthColor };
				}

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}

	void Renderer::VertexTransformationFunction(Mesh* mesh) const
	{
		auto& vertices{ mesh->GetVertices() };
		auto& vertices_out{ mesh->GetVerticesOut() };

		vertices_out.clear();
		vertices_out.reserve(vertices.size());

		Matrix worldprojectionMatrix{ m_WorldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix };

		for (auto& vertex : vertices)
		{
			// Tranform the vertex using the inversed view matrix
			Vertex_Out outVertex{ worldprojectionMatrix.TransformPoint({vertex.position, 1.f}),
				vertex.uv,
				m_WorldMatrix.TransformVector(vertex.normal).Normalized(),
				m_WorldMatrix.TransformVector(vertex.tangent).Normalized(),
				worldprojectionMatrix.TransformVector(vertex.viewDirection).Normalized()
			};

			outVertex.viewDirection = Vector3{ outVertex.position.x, outVertex.position.y, outVertex.position.z };
			outVertex.viewDirection.Normalize();

			outVertex.position.x /= outVertex.position.w;
			outVertex.position.y /= outVertex.position.w;
			outVertex.position.z /= outVertex.position.w;

			// Add the new vertex to the list of NDC vertices
			vertices_out.emplace_back(outVertex);
		}
	}

	ColorRGB dae::Renderer::PixelShading(Pixel_Out& pixel) const
	{
		Vector3 sampledNormal{ pixel.normal };
		if (m_UseNormalMap)
		{
			const Vector3 binormal{ Vector3::Cross(pixel.normal, pixel.tangent) };
			const Matrix tangentSpaceAxis{ pixel.tangent, binormal.Normalized(), pixel.normal, {0.f, 0.f, 0.f} };

			const ColorRGB normalColor{ m_pNormalMap->Sample(pixel.uv) };
			sampledNormal = { normalColor.r, normalColor.g, normalColor.b };

			sampledNormal = 2 * sampledNormal - Vector3{ 1.f, 1.f, 1.f };
			sampledNormal = tangentSpaceAxis.TransformVector(sampledNormal);
		}
		sampledNormal.Normalize();

		const float observedArea{ std::max(0.f, Vector3::Dot(sampledNormal, -m_LightDirection)) };
		const float kd{ .5f };

		ColorRGB finalColor{};

		switch (m_ShadingMode)
		{
		case ShadingMode::ObservedArea:
			finalColor = ColorRGB{ 1, 1, 1 } *observedArea;
			break;
		case ShadingMode::Diffuse:
		{
			ColorRGB diffuse{ (m_pDiffuseTextureVehicle->Sample(pixel.uv) * kd) / PI * m_LightIntensity };
			finalColor = diffuse * observedArea;
		}
		break;
		case ShadingMode::Specular:
		{
			finalColor = CalculateSpecular(pixel, sampledNormal) * observedArea;
		}
		break;
		case ShadingMode::Combined:
			ColorRGB diffuse{ (m_pDiffuseTextureVehicle->Sample(pixel.uv) * kd) / PI * m_LightIntensity };

			finalColor = (diffuse * observedArea) + CalculateSpecular(pixel, sampledNormal);
			break;
		}

		return finalColor;
	}

	ColorRGB dae::Renderer::CalculateSpecular(const Pixel_Out& pixel, const Vector3& sampledNormal) const
	{
		const Vector3 reflect{ Vector3::Reflect(m_LightDirection, sampledNormal) };

		const float cosAngle{ std::max(0.f, Vector3::Dot(reflect, -pixel.viewDirection)) };

		const float exp{ m_pGlossMap->Sample(pixel.uv).r * m_Shininess };

		const float phongSpecular{ powf(cosAngle, exp) };

		return m_pSpecularMap->Sample(pixel.uv) * phongSpecular;
	}

	BoundingBox Renderer::GetBoundingBox(Vector2 v0, Vector2 v1, Vector2 v2) const
	{
		BoundingBox box{ m_Width, m_Height };

		box.UpdateMin(v0);
		box.UpdateMin(v1);
		box.UpdateMin(v2);

		box.UpdateMax(v0);
		box.UpdateMax(v1);
		box.UpdateMax(v2);

		return box;
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

	void Renderer::ToggleRasterizer()
	{
		switch (m_RasterizerMode)
		{
		case RasterizerMode::Hardware:
			m_RasterizerMode = RasterizerMode::Software;
			std::cout << "\033[33m" << "**(SHARED) Rasterizer Mode = SOFTWARE\n" << "\033[0m";
			break;
		case RasterizerMode::Software:
			m_RasterizerMode = RasterizerMode::Hardware;
			std::cout << "\033[33m" << "**(SHARED) Rasterizer Mode = HARDWARE\n" << "\033[0m";
			break;
		default:
			break;
		}
	}

	void Renderer::ToggleRotation()
	{
		m_RotationEnabled = !m_RotationEnabled;

		std::cout << "\033[33m" << "**(SHARED) Vehicle Rotation";

		if (m_RotationEnabled)
		{
			std::cout << "ON \n";
		}
		else
		{
			std::cout << "OFF \n";
		}
		std::cout << "\033[0m";
	}

	void Renderer::ToggleFire()
	{
		if (m_RasterizerMode != RasterizerMode::Hardware)
			return;

		m_RenderFire = !m_RenderFire;

		std::cout << "\033[32m" << "**(HARDWARE) FireFX ";

		if (m_RenderFire)
		{
			std::cout << "ON \n";
		}
		else
		{
			std::cout << "OFF \n";
		}
		std::cout << "\033[0m";
	}

	void Renderer::CycleShadingMode()
	{
		if (m_RasterizerMode != RasterizerMode::Software)
			return;

		m_ShadingMode = static_cast<ShadingMode>((int(m_ShadingMode) + 1) % 4);

		std::cout << "\033[35m" << "**(SOFTWARE) Shading mode = ";

		switch (m_ShadingMode)
		{
		case ShadingMode::ObservedArea:
			std::cout << "Observed area \n";
			break;
		case ShadingMode::Diffuse:
			std::cout << "Diffuse \n";
			break;
		case ShadingMode::Specular:
			std::cout << "Specular \n";
			break;
		case ShadingMode::Combined:
			std::cout << "Combined \n";
			break;
		default:
			break;
		}
		std::cout << "\033[0m";
	}

	void Renderer::ToggleNormalMap()
	{
		if (m_RasterizerMode != RasterizerMode::Software)
			return;

		m_UseNormalMap = !m_UseNormalMap;

		std::cout << "\033[35m" << "**(SOFTWARE) NormalMap ";

		if (m_UseNormalMap)
		{
			std::cout << "ON \n";
		}
		else
		{
			std::cout << "OFF \n";
		}
		std::cout << "\033[0m";
	}

	void Renderer::ToggleDepthBuffer()
	{
		if (m_RasterizerMode != RasterizerMode::Software)
			return;

		m_RenderDepth = !m_RenderDepth;

		std::cout << "\033[35m" << "**(SOFTWARE) Depth Visualization ";

		if (m_RenderDepth)
		{
			std::cout << "ON \n";
		}
		else
		{
			std::cout << "OFF \n";
		}
		std::cout << "\033[0m";
	}

	void Renderer::ToggleBoundingBox()
	{
		if (m_RasterizerMode != RasterizerMode::Software)
			return;

		m_RenderBoundingBox = !m_RenderBoundingBox;

		std::cout << "\033[35m" << "**(SOFTWARE) BoundingBox Visualization ";

		if (m_RenderBoundingBox)
		{
			std::cout << "ON \n";
		}
		else
		{
			std::cout << "OFF \n";
		}
		std::cout << "\033[0m";
	}

	void Renderer::ToggleClearColor()
	{
		m_UniformClearColor = !m_UniformClearColor;

		std::cout << "\033[33m" << "**(SHARED) Uniform ClearColor";

		if (m_UniformClearColor)
		{
			std::cout << "ON \n";
		}
		else
		{
			std::cout << "OFF \n";
		}
		std::cout << "\033[0m";
	}

	void Renderer::ToggleFilterState()
	{
		if (m_RasterizerMode != RasterizerMode::Hardware)
			return;

		m_CurrentFilterState = static_cast<FilterState>((int(m_CurrentFilterState) + 1) % 3);

		std::cout << "\033[32m";

		std::cout << "**(HARDWARE) Sampler Filter = ";

		switch (m_CurrentFilterState)
		{
		case FilterState::Point:
			std::cout << "POINT\n";
			break;
		case FilterState::Linear:
			std::cout << "LINEAR\n";
			break;
		case FilterState::Anisotropic:
			std::cout << "ANISOTROPIC\n";
			break;
		default:
			break;
		}

		std::cout << "\033[0m";

		m_pVehicleMesh->ToggleFilter(m_CurrentFilterState);
		m_pFireMesh->ToggleFilter(m_CurrentFilterState);

	}




	void Renderer::PrintControls() const
	{
		std::cout << "\033[33m" << "[Key Bindings - SHARED] \n";
		std::cout << "   [F1]  Toggle Rasterizer Mode (HARDWARE/SOFTWARE)\n";
		std::cout << "   [F2]  Toggle Vehicle Rotation (ON/OFF)\n";
		std::cout << "   [F9]  Cycle CullMode (BACK/FRONT/NONE)\n";
		std::cout << "   [F10]  Toggle Uniform ClearColor (ON/OFF)\n";
		std::cout << "   [F11]  Toggle Print FPS (ON/OFF)\n \n" << "\033[0m";
		
		std::cout << "\033[32m" << "[Key Bindings - HARDWARE] \n";
		std::cout << "   [F3]  Toggle FireFX (ON/OFF)\n";
		std::cout << "   [F4]  Cycle Sampler State (ON/OFF)\n \n" << "\033[0m";

		std::cout << "\033[35m" << "[Key Bindings - SHARED] \n";
		std::cout << "   [F5]  Cycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)\n";
		std::cout << "   [F6]  Toggle NormalMap (ON/OFF)\n";
		std::cout << "   [F7]  Toggle DepthBuffer Visualization (ON/OFF)\n";
		std::cout << "   [F8]  Toggle BoundingBox Visualization (ON/OFF)\n \n" << "\033[0m";
	}
}
