cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

//	Both structures now have a 3-float normal vector. The normal vector is used
//	for calculating the amount of light by using the angle between the direction
//	of the normal and the direction of the light.

struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOOORD0;
	float3 normal : NORMAL;
};

PixelInputType LightVertexShader(VertexInputType input)
{
	PixelInputType output;

//	Change the position vector to be 4 units for proper matrix calculations.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

//	Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

//	The normal vector for this vertex is calculated in world space and then
//	normalized before being sent as input into the pixel shader. We only cal-
//	culated against the world matrix as we are just trying to find the lighting
//	values in the 3D world space. Note that sometimes these normals need to be 
//	re-normalized inside the pixel shader due to the interpolation that occours.

//	Calculate the normal vector against the world matrix only
	output.normal = mul(input.normal, (float3x3)worldMatrix);

//	Normalize the normal vector
	output.normal = normalize(output.normal);

	return output;
}