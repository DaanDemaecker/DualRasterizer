#include "pch.h"
#include "Texture.h"
#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>
#include <iostream>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface, ID3D11Device* pDevice):
		m_pSurface {pSurface},m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = pSurface->w;
		desc.Height = pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

		HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);


		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);

		if (FAILED(hr))
		{
			std::cout << "failed to load texture from file \n";
		}
	}

	Texture::~Texture()
	{
		if (m_pSRV)m_pSRV->Release();
		if (m_pResource)m_pResource->Release();

		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path, ID3D11Device* pDevice)
	{
		//TODO
		//Load SDL_Surface using IMG_LOAD
		//Create & Return a new Texture Object (using SDL_Surface)

		return new Texture{ IMG_Load(path.c_str()), pDevice};
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//TODO
		//Sample the correct texel for the given uv

		//calculate the x and y coordinates on the uv map
		const int x{ static_cast<int>(uv.x * m_pSurface->w) };
		const int y{ static_cast<int>(uv.y * m_pSurface->h) };

		//getht the index of the pixel in the list
		const uint32_t pixel{ m_pSurfacePixels[x + y * m_pSurface->w] };

		//initialize the rgb values in the [0, 255] range
		Uint8 r{};
		Uint8 g{};
		Uint8 b{};

		//Get correct values from the texture
		SDL_GetRGB(pixel, m_pSurface->format, &r, &g, &b);

		//return color divided by 255 to get colors in [0, 1] range
		return ColorRGB{ r / 255.0f, g / 255.0f, b / 255.0f };
	}

}
