#include "pch.h"
#include "EffectTransparent.h"

dae::EffectTransparent::EffectTransparent(ID3D11Device* pDevice, const std::wstring& assetFile)
	:Effect{pDevice, assetFile}
{
}
