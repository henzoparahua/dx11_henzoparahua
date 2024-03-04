#ifdef _LIGHTSHADERCLASS_H_
#define _LIGHTSHADERCLASS_H_

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <fstream>

class LightShaderClass
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

//	The new LightBufferType structure will be used for holding
//	lighting information. This typedef is the same as the new
//	typedef in the pixel shader. Do note that I add an extra float
// 	for size padding to ensure the structure is a multiple of 16.
// 	Since the structure without an extra float is only 28 bytes
// 	CreateBuffer would have failed if we used a sizeof(LightBufferType)
// 	because it requires sizes that are multiple of 16 to succeed.
//
	struct LightBufferType
	{
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float padding; // Added extra padding so sttructre is a multiple of 16 for CreateBuffer function requirements.
	};
public:
	LightShaderClass();
	LightShaderClass(const LightShaderClass&);
	~LightShaderClass();

	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX,
		XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT3, XMFLOAT4);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_matrixBuffer;

//	There is a new private constant for the light informtion (color and direction),
//	The light buffer will be used by this class to set the global light variables inside
//	the HLSL pixel shader.
	ID3D11Buffer* m_lightBuffer;
};

#endif;