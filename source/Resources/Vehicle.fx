//-------------------------
//	Globals
//-------------------------
float4x4 gWorldViewProj : WorldViewProjection;
float4x4 gWorldMatrix : WorldMatrix;
float4x4 gViewInverse : InverseViewMatrix;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

SamplerState gSamPoint : Sampler;

float3 gLightDirection = normalize(float3(0.577f, -0.577f, 0.577f));

float gPI = 3.14159265358979f;
float gLightIntensity = 7.0f;
float gShininess = 25.0f;

RasterizerState gRasterizerState
{
	CullMode = back;
	FrontCounterClockwise = false; // default
};

BlendState gBlendState
{
	BlendEnable[0] = false;
	RenderTargetWriteMask[0] = 0x0F;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = 1;
	DepthFunc = less;
	StencilEnable = false;

	//others are redundant because
	//StencilEnable is false
	//(for demo purposes only)
	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};

//-------------------------
//	Input/Output Structs
//-------------------------
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 uv : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : W_Position;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 uv : TEXCOORD;
};

//-------------------------
//	Vertex Shader
//-------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorldViewProj);
	output.uv = input.uv;
	return output;
}


//---------------------------
// Calculate the right Normal
//---------------------------
float3 CalculateNormal(VS_OUTPUT input)
{
	float3 binormal = cross(input.Normal, input.Tangent);
	float4x4 tangentSpaceAxis = float4x4(float4(input.Tangent, 0.0f), float4(binormal, 0.0f), float4(input.Normal, 0.0), float4(0.0f, 0.0f, 0.0f, 1.0f));

	float3 sampledNormal = gNormalMap.Sample(gSamPoint, input.uv).rgb;
	sampledNormal = 2 * sampledNormal - float3(1.f, 1.f, 1.f);

	return mul(float4(sampledNormal, 0.0f), tangentSpaceAxis).xyz;

}

//----------------------
// Calculate Specular
//------------
float4 CalculateSpecular(VS_OUTPUT input, float3 normal, float3 viewDirection)
{
	float3 reflected = reflect(gLightDirection, normal);

	float cosAngle = saturate(dot(reflected, viewDirection));

	float exp = gGlossinessMap.Sample(gSamPoint, input.uv).r * gShininess;

	float phongSpecular = pow(cosAngle, exp);

	return gSpecularMap.Sample(gSamPoint, input.uv) * phongSpecular;
}



//-------------------------
//	Pixel Shader
//-------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);

	float3 normal = normalize(CalculateNormal(input));

	float observedArea = saturate(dot(input.Normal, -gLightDirection));
	float kd = 0.5;

	float4 diffuse = (gDiffuseMap.Sample(gSamPoint, input.uv) * kd) / gPI * gLightIntensity;

	float4 specular = CalculateSpecular(input, normal, viewDirection);

	return (diffuse + specular) * observedArea;
}

//-------------------------
//	Technique
//-------------------------
technique11 DefaultTechnique
{
	pass P0
	{
		SetRasterizerState(gRasterizerState);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
