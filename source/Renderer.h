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

		void ToggleRasterizer();

		void ToggleRotation();

		void ToggleFire();

		void CycleShadingMode();

		void ToggleNormalMap();

		void ToggleDepthBuffer();

		void ToggleBoundingBox();

		void ToggleClearColor();

		void ToggleFilterState();

	private:
		SDL_Window* m_pWindow{};

		RasterizerMode m_RasterizerMode;
		FilterState m_CurrentFilterState;

		bool m_RenderFire{ true };;
		bool m_RenderBoundingBox{ false };
		bool m_RenderDepth{ false };
		bool m_RotationEnabled{ true };
		bool m_UseNormalMap{ true };
		bool m_UniformClearColor{ false };
		ShadingMode m_ShadingMode{ ShadingMode::Combined };

		const Vector3 m_LightDirection = Vector3{ .577f, -.577f, .577f }.Normalized();
		float m_LightIntensity{ 7.f };
		float m_Shininess{ 25.f };
		ColorRGB m_Ambient{ .025f, .025f, .025f };

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};
		float* m_pDepthBufferPixels{};

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

		void RenderHardware() const;

		void RenderSoftware() const;

		//function that returns the bounding box for a triangle
		BoundingBox GetBoundingBox(Vector2 v0, Vector2 v1, Vector2 v2) const;

		//function that renders a single mesh
		void RenderMesh(Mesh* mesh) const;

		//function that renders a single triangle
		void RenderTriangle(const Triangle& triangle) const;

		//function to setup current triangle
		bool CalculateTriangle(Triangle& triangle, Mesh* mesh, std::vector<Vector2>& vertices_ScreenSpace, int startIdx, bool flipTriangle = false) const;

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunction(Mesh* mesh) const; //W1 Version

		//Function that shades a single pixel
		ColorRGB PixelShading(Pixel_Out& pixel) const;

		ColorRGB CalculateSpecular(const Pixel_Out& pixel, const Vector3& sampeledNormal) const;




		void PrintControls() const;
	};
}
