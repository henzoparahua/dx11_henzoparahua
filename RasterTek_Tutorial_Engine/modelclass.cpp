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

//	First create two temporary arrays to hold the vertex and index data that we will use 
//	later to populate the final buffers with.
//	Set the number of vertices in the vertex array.
	m_vertexCount = 3;

//	Set the number of indices in the index array.
	m_indexCount = 3;

//	Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

//	Create the index array.
	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

//	Now fill both the vertex and index array with the three points of the triangle as well as the
//	index to each of the points. Please note that I created the points in the clockwise order of
//	drawing them. If you do this counter clockwise it will think the triangle is facing the opposite
//	direction and not draw it due to back face culling. Always remember that the order in which you
//	send your vertices to the GPU is very important. The color is set here as well since it is part
//	of the vertex description.

//	Loaf the vertex array with data.
	vertices[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f); // Bottom left.
	vertices[0].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	
	vertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f); // Top middle.
	vertices[1].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	vertices[2].position = XMFLOAT3(-1.0f, -1.0f, 0.0f); // Bottom right.
	vertices[2].position = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

//	Load the index array with data.
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
}