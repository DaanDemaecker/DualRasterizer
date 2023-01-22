#pragma once
#include "Math.h"
#include "vector"
#include "Camera.h"

namespace dae
{
	struct Vertex_Out
	{
		Vector4 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
	};

	struct Pixel_Out
	{
		Vector4 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector3 viewDirection{};
	};

	struct BoundingBox
	{
		BoundingBox()
		{

		}
		BoundingBox(int screenWidth, int screenHeight)
		{
			clampX = screenWidth + 1;
			clampY = screenHeight + 1;
		}

		int minX{ INT_MAX };
		int minY{ INT_MAX };

		int maxX{ -INT_MAX };
		int maxY{ -INT_MAX };

		int margin{ 1 };

		int clampX{};
		int clampY{};

		void UpdateMin(Vector2 point)
		{
			minX = Clamp(std::min(minX, int(point.x)) - margin, 0, clampX);
			minY = Clamp(std::min(minY, int(point.y)) - margin, 0, clampY);
		}

		void UpdateMax(Vector2 point)
		{
			maxX = Clamp(std::max(maxX, int(point.x)) + margin, 0, clampX);
			maxY = Clamp(std::max(maxY, int(point.y)) + margin, 0, clampY);
		}
	};

	enum class PrimitiveTopology
	{
		TriangeList,
		TriangleStrip
	};

	struct Triangle
	{
		Triangle() {
			ndc.resize(3); screen.resize(3); isOutsideFrustum.resize(3);
		};

		void CheckOutsideFrustum(const Camera& camera)
		{
			for (int i{}; i < ndc.size(); i++)
			{
				isOutsideFrustum[i] = camera.IsOutsideFrustum(ndc[i].position);
			}
		};

		void ScreenToNdc(const Triangle& other, float width, float height)
		{
			for (int i{}; i < screen.size(); i++)
			{
				const Vector2 point{ screen[i] };
				ndc[i].position.x = (screen[i].x / width) * 2 - 1;
				ndc[i].position.y = -((screen[i].y / height) * 2 - 1);

				const Vector2 edgeV0V1{ other.screen[1] - other.screen[0] };
				const Vector2 edgeV1V2{ other.screen[2] - other.screen[1] };
				const Vector2 edgeV2V0{ other.screen[0] - other.screen[2] };

				const float inverseTriangleArea{ 1.f / Vector2::Cross(edgeV1V2,edgeV2V0) };

				const Vector2 v0ToPoint{ point - other.screen[0] };
				const Vector2 v1ToPoint{ point - other.screen[1] };
				const Vector2 v2ToPoint{ point - other.screen[2] };

				// Calculate cross product from edge to start to point
				const float edge01PointCross{ Vector2::Cross(edgeV0V1, v0ToPoint) };
				const float edge12PointCross{ Vector2::Cross(edgeV1V2, v1ToPoint) };
				const float edge20PointCross{ Vector2::Cross(edgeV2V0, v2ToPoint) };

				if (!(edge01PointCross >= 0 && edge12PointCross >= 0 && edge20PointCross >= 0)) continue;

				const float weightV0{ edge12PointCross * inverseTriangleArea };
				const float weightV1{ edge20PointCross * inverseTriangleArea };
				const float weightV2{ edge01PointCross * inverseTriangleArea };

				float interpolatedZDepth
				{
							1 / (weightV0 / other.ndc[0].position.z +
							weightV1 / other.ndc[1].position.z +
							weightV2 / other.ndc[2].position.z)
				};
				ndc[i].position.z = interpolatedZDepth;

				const float interpolatedWDepth =
					1 / (weightV0 / other.ndc[0].position.w +
						weightV1 / other.ndc[1].position.w +
						weightV2 / other.ndc[2].position.w);
				ndc[i].position.w = interpolatedWDepth;

				ndc[i].uv = ((weightV0 * other.ndc[0].uv / other.ndc[0].position.w) +
					(weightV1 * other.ndc[1].uv / other.ndc[1].position.w) +
					(weightV2 * other.ndc[2].uv / other.ndc[2].position.w)) * interpolatedWDepth;

				ndc[i].normal =
				{
					(((weightV0 * other.ndc[0].normal / other.ndc[0].position.w) +
					(weightV1 * other.ndc[1].normal / other.ndc[1].position.w) +
					(weightV2 * other.ndc[2].normal / other.ndc[2].position.w)) * interpolatedWDepth)
				};
				ndc[i].normal.Normalize();

				ndc[i].tangent =
				{
					(((weightV0 * other.ndc[0].tangent / other.ndc[0].position.w) +
					(weightV1 * other.ndc[1].tangent / other.ndc[1].position.w) +
					(weightV2 * other.ndc[2].tangent / other.ndc[2].position.w)) * interpolatedWDepth)
				};

				ndc[i].viewDirection =
				{
					(((weightV0 * other.ndc[0].viewDirection / other.ndc[0].position.w) +
					(weightV1 * other.ndc[1].viewDirection / other.ndc[1].position.w) +
					(weightV2 * other.ndc[2].viewDirection / other.ndc[2].position.w)) * interpolatedWDepth)
				};
			}
		};


		std::vector<bool> isOutsideFrustum;
		std::vector<Vertex_Out> ndc;
		std::vector<Vector2> screen;
		BoundingBox boundingBox;
	};
}