#pragma once

namespace dae
{
	class Effect;
	class Texture;

	struct Vertex
	{
		Vector3 position;
		Vector3 normal;
		Vector3 tangent;
		Vector2 uv;
	};

	class Mesh final
	{
	public:
		Mesh() = delete;
		Mesh(ID3D11Device* pDevice, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indicess, EffectType type);

		~Mesh();

		Mesh& operator=(Mesh& rhs) = delete;
		Mesh& operator=(Mesh&& rhs) = delete;
		Mesh(Mesh& rhs) = delete;
		Mesh(Mesh&& rhs) = delete;

		void Render(ID3D11DeviceContext* pDeviceContext) const;

		void SetMatrices(const Matrix& wvpmatrix, const Matrix& worldMatrix, const Matrix& viewinverse);
		void SetDiffuseMap(Texture* pDiffuseTexture);
		void SetGlossmap(Texture* pGlossMap);
		void SetNormalMap(Texture* pNormalMap);
		void SetSpecularMap(Texture* pSpecularMap);

		void ToggleFilter(FilterState filter);

	private:
		Effect* m_pEffect;

		ID3D11InputLayout* m_pInputLayout{};

		ID3D11Buffer* m_pVertexBuffer{};

		uint32_t m_NumIndices{};
		ID3D11Buffer* m_pIndexBuffer{};
	};
}

