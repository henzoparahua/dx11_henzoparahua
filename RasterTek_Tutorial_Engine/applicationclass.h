#ifndef _APPLICATIONCLASS_H_
#define _APPLICATIONCLASS_H_

#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "textureshaderclass.h"

const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.3f;
extern bool FULL_SCREEN;


class ApplicationClass
{
public:
	ApplicationClass();
	ApplicationClass(const ApplicationClass&);
	~ApplicationClass();

	bool ChangeScreen(int, int, HWND);
	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame();

private:
	bool Render();


private:
	D3DClass* m_Direct3D;
	CameraClass* m_Camera;
	ModelClass* m_Model;
	TextureShaderClass* m_TextureShader;
};

#endif