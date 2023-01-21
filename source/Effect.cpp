#include "pch.h"
#include "Effect.h"
#include "Texture.h"


namespace dae
{
	dae::Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
		:m_pEffect{ LoadEffect(pDevice, assetFile) }
	{
		m_pTechnique = m_pEffect->GetTechniqueByName("DefaultTechnique");
		if (!m_pTechnique->IsValid())
			std::wcout << L"Technique not valid \n";

		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";
		}

		m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
		if (!m_pMatWorldVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldVariable not valid!\n";
		}

		m_pMatInvViewVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
		if (!m_pMatInvViewVariable->IsValid())
		{
			std::wcout << L"m_pMatInvViewVariable is not valid!\n";
		}

		m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable not valid\n";
		}

		m_pSamplerVariable = m_pEffect->GetVariableByName("gSamPoint")->AsSampler();
		if (!m_pSamplerVariable->IsValid())
		{
			std::wcout << L"m_pSamplerVariable not valid \n";
		}


		D3D11_SAMPLER_DESC samplerDesc{};

		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MipLODBias = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		samplerDesc.MaxAnisotropy = 16;

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		pDevice->CreateSamplerState(&samplerDesc, &m_pPoint);

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		pDevice->CreateSamplerState(&samplerDesc, &m_pLinear);

		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		pDevice->CreateSamplerState(&samplerDesc, &m_pAnisotropic);

		ToggleFilter(FilterState::Point);

	}

	dae::Effect::~Effect()
	{
		if (m_pTechnique)m_pTechnique->Release();
		if(m_pEffect)m_pEffect->Release();
	}

	void Effect::SetMatrices(const Matrix& wvpmatrix, const Matrix& worldMatrix, const Matrix& viewinverse)
	{
		m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&wvpmatrix));
		m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
		m_pMatInvViewVariable->SetMatrix(reinterpret_cast<const float*>(&viewinverse));
	}

	void Effect::SetDiffuseMap(Texture* pDiffuseTexture)
	{
		if (m_pDiffuseMapVariable)
			m_pDiffuseMapVariable->SetResource(pDiffuseTexture->GetResource());
	}

	void Effect::ToggleFilter(FilterState filter)
	{
		switch (filter)
		{
		case FilterState::Point:
			m_pSamplerVariable->SetSampler(0, m_pPoint);
			break;
		case FilterState::Linear:
			m_pSamplerVariable->SetSampler(0, m_pLinear);
			break;
		case FilterState::Anisotropic:
			m_pSamplerVariable->SetSampler(0, m_pAnisotropic);
			break;
		default:
			break;
		}
	}

	ID3DX11Effect* dae::Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags{0};

		

#if defined(DEBUG) || defined(_DEBUG)
		shaderFlags |= D3DCOMPILE_DEBUG;
		shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		result = D3DX11CompileEffectFromFile
		(
			assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

				std::wstringstream ss;
				for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
				{
					ss << pErrors[i];
				}

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << std::endl;
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to createEffectFromFile! \nPath: " << assetFile;
				std::wcout << ss.str() << std::endl;
				return nullptr;
			}
		}
		return pEffect;
	}
}
