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
		Vector3 viewDirection{};
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

		std::vector<Vertex>& GetVertices() { return m_Vertices; }
		std::vector<Vertex_Out>& GetVerticesOut() { return m_Vertices_out; }
		std::vector<uint32_t>& GetIndices() { return m_Indices; }
		PrimitiveTopology GetTopology() { return m_PrimitiveTopology; }

		void SetTopology(PrimitiveTopology topology) { m_PrimitiveTopology = topology; }

	private:
		Effect* m_pEffect;

		std::vector<Vertex> m_Vertices{};
		std::vector<uint32_t> m_Indices{};
		PrimitiveTopology m_PrimitiveTopology{ PrimitiveTopology::TriangleStrip };

		std::vector<Vertex_Out> m_Vertices_out{};

		ID3D11InputLayout* m_pInputLayout{};

		ID3D11Buffer* m_pVertexBuffer{};

		uint32_t m_NumIndices{};
		ID3D11Buffer* m_pIndexBuffer{};
	};
}

