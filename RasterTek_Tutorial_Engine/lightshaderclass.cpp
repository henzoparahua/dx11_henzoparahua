#include "lightshaderclass.h"

LightShaderClass::LightShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_sampleState = 0;
	m_matrixBuffer = 0;

	m_lightBuffer = 0;
}

LightShaderClass::LightShaderClass(const LightShaderClass& other)
{

}

LightShaderClass::~LightShaderClass()
{

}

bool LightShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	wchar_t vsFilename[128];
	wchar_t psFilename[128];
	int error;
	bool result;

//	Set the filename of the vertex shader
	error = wcscpy_s(vsFilename, 128, L"light.vs");
	if (error != 0)
	{
		return false;
	}
//	Set the filename of the pixel shader
	error = wcscpy_s(psFilename, 128, L"light.ps");
	if (error != 0)
	{
		return false;
	}

//	Initialize the vertex and pixel shaders.
	result = InitializeShader(device, hwnd, vsFilename, psFilename);
	if (error != 0)
	{
		return false;
	}

	return true;
}

void LightShaderClass::Shutdown()
{
//	Shutdown the vertex and pixel shader as well as the related objects.
	ShutdownShader();

	return;
}

//	The render function now takes in the light direction and light diffuse
//	color as inputs. These variables are then sent into the SetShaderParameters
//	function and finally set inside the shader itself.

bool LightShaderClass::Render(ID3D11DeviceContext* deviceContext, int indexCount,
	XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix,
	ID3D11ShaderResourceView* texture, XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
	bool result;

//	Set the shader parameters that it will use for rendering.
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, 
		texture, lightDirection, diffuseColor);
	if (!result)
	{
		return false;
	}

//	Now render the prepared buffer with the shader.	
	RenderShader(deviceContext, indexCount);

	return true;
}

bool LightShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;


//	The polygonLayout variable has been changed to have three elements instead of two. This is so
//	that it can accommodate a normal vector in the layout.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;

//	We also add a new description variable for the light constant buffer.
	D3D11_BUFFER_DESC lightBufferDesc;

//	Initialize the pointers this function will use to null
	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

//	Load in the new light vertex shader.
//	Compile the vertex shader code.
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "LightVertexShader", "vs_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
//	If the shader failed to compile it should have writen something to
//	the error message.
		if (errorMessage) 
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
//	If there was nothing in the error message then it simply could not find the
//	shader file itself.
		else
		{
			MessageBox(hwnd, psFilename, L"Missing shader file", MB_OK);
		}
		return false;
	}

//	Create the vertex shader from the buffer
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

//	Create the pixel shader from the buffer
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

//	Create the vertex input layout description
//	This setup needs to match the VertexType structure in the ModelClass and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

//	One of the major changes to the shader initialization is here in the polygonLayout.
//	We add a third element for the normal vector that will be used for lighting. The 
// 	semantic name is NORMAL and the format is the regular DXGI_FORMAT_R32G32B32_FLOAT
// 	which handles 3 floats for the x, y and z of the normal vector. The layout will now
// 	match the expected input to the HLSL vertex shader.

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

//	Get a count of the elements in the layout
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

//	Create the vertex input layout
	result = device->CreateInputLayout(polygonLayout, numElements,
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

//	Release the vertex shader buffer and pixel shader buffer since theyre not longer needed.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

//	Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

//	Create the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

//	Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void LightShaderClass::ShutdownShader()
{
//	The new light constant buffer is released in the ShutdownShader function.

	if (m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = 0;
	}
	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}
	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}
	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}
	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

	return;
}

void LightShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned __int64 bufferSize, i;
	ofstream fout;

//	Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

//	Get a length of the message.
	bufferSize = errorMessage->GetBufferSize();

//	Open a file to write the error message to.
	fout.open("shader-error.txt");

//	Write out the error message.
	for (i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}

//	Close the file.
	fout.close();

//	Release the error message.
	errorMessage->Release();
	errorMessage = 0;

//	Pop a message up on the screen to notify the user to check the text file for copile errors.
	MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt for message.", shaderFilename, MB_OK);

	return;
}

bool LightShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext,
	XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture,
	XMFLOAT3 lightDirection, XMFLOAT4 diffuseColor)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNumber;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;

//	Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

//	Lock the constant buffer so it can be written to.
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

//	Get a pointer to the data in the contant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

//	Copy the matrices into the constant buffer..
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

//	Unlock the constant buffer.
	deviceContext->Unmap(m_matrixBuffer, 0);

//	Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

//	Now set the constant buffer in the vertex shader with the updated values.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

//	Set shader texture resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);

//	The light constant buffer is setup the same way as the matrix constant buffer. We first lock 
//	the buffer and get a pointer to it. After that we set the diffuse color and light direction
// 	using that pointer. Once the data is set, we unlock the buffer and then set it in the pixel
// 	shader. Note that we use the PSSetConstantBuffers function instead of VSSetConstantBuffers
// 	since this is a pxel shader buffer we are setting.

//	Lock the light constant buffer so it can be written to.
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

//	Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBufferType*)mappedResource.pData;
}