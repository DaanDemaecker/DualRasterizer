#include "pch.h"
#include "EffectShaded.h"
#include "Texture.h"

namespace dae
{
	EffectShaded::EffectShaded(ID3D11Device* pDevice, const std::wstring& assetFile)
		:Effect{pDevice, assetFile}
	{
		m_pGlossMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
		if (!m_pGlossMapVariable->IsValid())
		{
			std::wcout << L"m_pGlossMapVariable is not valid \n";
		}

		m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapVariable->IsValid())
		{
			std::wcout << L"m_pNormalMapVariable is not valid \n";
		}

		m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		if (!m_pSpecularMapVariable->IsValid())
		{
			std::wcout << L"m_pSPecularMapVariable is not valid \n";
		}
	}

	EffectShaded::~EffectShaded()
	{
		m_pGlossMapVariable->Release();
		m_pNormalMapVariable->Release();
		m_pSpecularMapVariable->Release();
	}



	void EffectShaded::SetGlossmap(Texture* pGlossMap)
	{
		if (m_pGlossMapVariable)
			m_pGlossMapVariable->SetResource(pGlossMap->GetResource());
	}

	void EffectShaded::SetNormalMap(Texture* pNormalMap)
	{
		if (m_pNormalMapVariable)
			m_pNormalMapVariable->SetResource(pNormalMap->GetResource());
	}

	void EffectShaded::SetSpecularMap(Texture* pSpecularMap)
	{
		if (m_pSpecularMapVariable)
			m_pSpecularMapVariable->SetResource(pSpecularMap->GetResource());
	}
}