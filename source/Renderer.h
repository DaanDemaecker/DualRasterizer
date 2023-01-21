#pragma once
#include "Camera.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Mesh;
	class Texture;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

		void ToggleFilterState();

	private:
		SDL_Window* m_pWindow{};

		FilterState m_CurrentFilterState;

		int m_Width{};
		int m_Height{};

		bool m_IsInitialized{ false };

		Mesh* m_pVehicleMesh{};

		Mesh* m_pFireMesh{};

		Matrix m_WorldMatrix{};

		Texture* m_pDiffuseTextureVehicle;
		Texture* m_pGlossMap;
		Texture* m_pNormalMap;
		Texture* m_pSpecularMap;

		Texture* m_pDiffuseTextureFire;

		float m_AspectRatio{};
		Camera m_Camera{};

		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;

		IDXGISwapChain* m_pSwapChain;

		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;

		ID3D11Resource* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		//DIRECTX
		HRESULT InitializeDirectX();
		//...


		void LoadMeshes();
	};
}
