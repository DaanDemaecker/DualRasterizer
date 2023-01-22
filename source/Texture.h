#pragma once
struct Vector2;

namespace dae
{
	class Texture
	{
	public:
		~Texture();

		static Texture* LoadFromFile(const std::string& path, ID3D11Device* pDevice);

		ID3D11ShaderResourceView* GetResource() const { return m_pSRV; };

		ColorRGB Sample(const Vector2& uv) const;

	private:
		Texture(SDL_Surface* pSurface, ID3D11Device* pDevice);

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };

		ID3D11Texture2D* m_pResource;
		ID3D11ShaderResourceView* m_pSRV;
	};
}

