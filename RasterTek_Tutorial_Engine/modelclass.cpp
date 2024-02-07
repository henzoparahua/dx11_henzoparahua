#include "modelclass.h"

ModelClass::ModelClass()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
}
ModelClass::ModelClass(const ModelClass& other)
{
}
ModelClass::~ModelClass()
{
}
bool ModelClass::Initialize(ID3D11Device* device)
{
	bool result;

//	Initialize the vertex and index buffers.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
	return true;
}

//	Render is called from the ApplicationClass::Render function. This function calls
//	RenderBuffers to put the vertex and index buffers on the graphics pipeline so the 
//	color shaders will be able to render them.

void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
//	Put the vertex and index buffers on the graphics pipeline to rpepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}

//	GetIndexCount returns the number of indexes in the model. The color shader will need
//	this informatio to draw this model.

int ModelClass::GetIndexCount()
{
	return m_indexCount;
}

//	The InitializeBuffers function is where we handle creating the vertex and index buffers.
//	Usually, you would read in a model and create the buffers from that data file. We will
//	just set the points in the vertex and index buffer manually since its only a single triangle.

bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

//	First create two temporary arrays
}