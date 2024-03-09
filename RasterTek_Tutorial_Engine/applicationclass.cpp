#include "applicationclass.h"

ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_Model = 0;
//	TextureShader = 0;

//	The light shader and light object are set to null in the class constructor
	m_LightShader = 0;
	m_Light = 0;
}

ApplicationClass::ApplicationClass(const ApplicationClass& other)
{

}

ApplicationClass::~ApplicationClass()
{

}


bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	char textureFilename[128];
	bool result;


// Create and initialize the Direct3D object.
	m_Direct3D = new D3DClass;

	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}
//	Create the camera object
	m_Camera = new CameraClass;

//	Set the initial position of the camera.
	m_Camera->SetPosition(0.0f, 1.0f, -7.0f);

//	Create and initialize the model object.
	m_Model = new ModelClass;

/*	result = m_Model->Initialize(m_Direct3D->GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	Create and initialize the color shader object.
	m_ColorShader = new ColorShaderClass;
	
	result = m_ColorShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the color shader object", L"Error", MB_OK);
		return false;
	}
*/
//	The ModelClass::Initialize function now takes in the name of the texture that will be used for
//	rendering the models as well as the device context.
// Set the name of the texture file that we will be loading.
	strcpy_s(textureFilename, "stone.tga");

	result = m_Model->Initialize(m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), textureFilename);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}
	
//	The new textureshaderclass object is created and initialized.

//	Create and initialize the texture shader object.
/*	m_TextureShader = new TextureShaderClass;

	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object", L"Error", MB_OK);
		return false;
	}
	return true;
*/
//	The light shader object is created and initialized here.
//	Create and initialize the light shader object.
	m_LightShader = new LightShaderClass;
	
	result = m_LightShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}
//	The nre light object is created here. The color of the light is set to white and the light
//	direction is set to point down the positive Z axis.
//	Create and initialize the light object. 
	m_Light = new LightClass;
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(0.0f, 0.0f, 1.0f);

	return true;
}

void ApplicationClass::Shutdown()
{
/*	Release the color shader object.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}
*/

//	Release the model object.
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = 0;
	}
//	Release the camera object.
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

//	Release the Direct3D object.
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = 0;
	}
	return;
}

bool ApplicationClass::Frame()
{
	bool result;

	result = Render();
	if (!result)
	{
		return false;
	}
	return true;
}

//	It begins with clearing the scene except that is cleared to black.
//	After that it calls the Render function for the camera object to create
//	a view matrix based on the camera's location that was set in the Initialize
//	function. Once the view matrix is created, we get a copy of it from the camera class.
//	We also  get copies of the world and projection matrix from the D3DClass object. We
//	then call the ModelClass::Render function to put the green triangle model geometry on
//	the graphics pipeline. With the vertices now prepared we call the color shader to draw
//	the vertices using the model information and the three triangle model geometry on the
//	graphics pipeline. With the vertices now prepared we call the color shader to draw the 
//	vertices using the model information and the three matrices for positioning each vertex.
//	The green triangle is now drawn to the back buffer. With that the scene is complete and
//	we call EndScene to display it to the screen.
bool ApplicationClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

//	Clear the buffers to begin the scene.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

//	Generate the view matrix basd on the camera's position.
	m_Camera->Render();

//	Get the world, view and projection matrices from the camera and d3d objects.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

//	Put the model vertex and index buffers on the graphics pipeline to prepare them for drawing.
	m_Model->Render(m_Direct3D->GetDeviceContext());

//	Render the model using the color shader.
	result = m_TextureShader->Render(m_Direct3D->GetDeviceContext(), m_Model->GetIndexCount(), 
		worldMatrix, viewMatrix, projectionMatrix, m_Model->GetTexture());
	if (!result)
	{
		return false;
	}

//	Present the rendered scene to the screen.
	m_Direct3D->EndScene();
	return true;
}