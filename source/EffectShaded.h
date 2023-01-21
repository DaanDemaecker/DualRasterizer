#pragma once
class Texture;

#include "Effect.h"

namespace dae
{
	class EffectShaded final : public Effect
	{
	public:
		EffectShaded(ID3D11Device* pDevice, const std::wstring& assetFile);

		virtual ~EffectShaded();

		virtual void SetGlossmap(Texture* pGlossMap) override;
		virtual void SetNormalMap(Texture* pNormalMap) override;
		virtual void SetSpecularMap(Texture* pSpecularMap) override;

	private:
		ID3DX11EffectShaderResourceVariable* m_pGlossMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable;
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable;
	};
}

