#include "applicationclass.h"

ApplicationClass::ApplicationClass()
{
	m_Direct3D = 0;
	m_Camera = 0;
	m_Model = 0;
	m_ColorShader = 0;
}

ApplicationClass::ApplicationClass(const ApplicationClass& other)
{

}

ApplicationClass::~ApplicationClass()
{

}

bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
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
	m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

//	Create and initialize the model object.
	m_Model = new ModelClass;

	result = m_Model->Initialize(m_Direct3D->GetDevice());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

//	Create and initialize the color shader object.
	m_ColorShader = new ColorShaderClass;
	
	result = m_ColorShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the color shader object", L"Error", MB_OK);
		return false;
	}

	return true;
}

void ApplicationClass::Shutdown()
{
//	Release the color shader object.
	if (m_ColorShader)
	{
		m_ColorShader->Shutdown();
		delete m_ColorShader();
		m_ColorShader = 0;
	}
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
	m_Direct3D->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);
	m_Direct3D->EndScene();
	return true;
}