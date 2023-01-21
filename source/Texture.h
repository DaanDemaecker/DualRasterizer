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

	private:
		Texture(SDL_Surface* pSurface, ID3D11Device* pDevice);

		ID3D11Texture2D* m_pResource;
		ID3D11ShaderResourceView* m_pSRV;
	};
}

