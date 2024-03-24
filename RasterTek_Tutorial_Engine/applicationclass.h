#ifndef _APPLICATIONCLASS_H_
#define _APPLICATIONCLASS_H_

#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"

#include "lightshaderclass.h"
#include "lightclass.h"

const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;
const bool FULL_SCREEN = false;


class ApplicationClass
{
public:
	ApplicationClass();
	ApplicationClass(const ApplicationClass&);
	~ApplicationClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();

private:
	bool Render(float);


private:
	D3DClass* m_Direct3D;
	CameraClass* m_Camera;
	ModelClass* m_Model;
	
//	There are two new private variables for the light shader and the light object.
	LightShaderClass* m_LightShader;
	LightClass* m_Light;
};

#endif