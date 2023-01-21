#pragma once

namespace dae
{
	class Texture;

	class Effect
	{
	public:
		Effect() = delete;
		explicit Effect(ID3D11Device* pDevice, const std::wstring& assetFile);

		virtual ~Effect();
		
		Effect(Effect& rhs) = delete;
		Effect(Effect&& rhs) = delete;
		Effect& operator=(Effect& rhs) = delete;
		Effect& operator=(Effect&& rhs) = delete;

		ID3DX11Effect* GetEffect() const { return m_pEffect; };
		ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; };

		void SetMatrices(const Matrix& wvpmatrix, const Matrix& worldMatrix, const Matrix& viewinverse);
		void SetDiffuseMap(Texture* pDiffuseTexture);

		virtual void SetGlossmap(Texture* pGlossMap){};
		virtual void SetNormalMap(Texture* pNormalMap){};
		virtual void SetSpecularMap(Texture* pSpecularMap){};

		void ToggleFilter(FilterState filter);

	protected:
		ID3DX11Effect* m_pEffect{};
		ID3DX11EffectTechnique* m_pTechnique{};

		ID3D11SamplerState* m_pPoint{};
		ID3D11SamplerState* m_pLinear{};
		ID3D11SamplerState* m_pAnisotropic{};

		ID3DX11EffectSamplerVariable* m_pSamplerVariable{};

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
		ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};
		ID3DX11EffectMatrixVariable* m_pMatInvViewVariable{};


		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable;
		

		ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	};
}

