//	In shader programs you begin with the global variables. These globals can be modified
//	externally from your C++ code. You can use many of variables such as int or float and
//	then set them externally for the shaders program to use. Generally, you will put most
//	globals in buffer object types called "cbuffer" even if it is just a single global 
//	variable.
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix
};


//	Similar to C we can create our ownty
struct VertexInputType
{
	float4 position : POSITION;
	float4 color : COLOR;
};
struct PixelInputType
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

PixelInputType ColorVertexShader(VertexInputType input)
{
	PixelInputType output;

//	Change the position vector to be 4 units for matrix calculations
	input.position.w = 1.0f;

//	Calculate the position of the vertex against the world, view and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

//	Store the input color for the pixel shader to use.
	output.color = input.color;

	return output;
}