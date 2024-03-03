#ifndef _SYSTEMCLASS_H_
#define _SYSTEMCLASS_H_

//	Here we define WIN32_LEAN_AND_MEAN. We do this to speed up the build process.	
//	It reduces the size of the Win32 header files by excluding some of the less used APIs.
#define WIN32_LEAN_AND_MEAN

//	We've included the headers to the other two classes in the framework at this point
//	so we can use them in the system class.

#include <windows.h>

//	My class includes
#include "inputclass.h"
#include "applicationclass.h"

class SystemClass {
public:
	SystemClass();
	SystemClass(const SystemClass&);
	~SystemClass();
	
	bool Initialize();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();
	
	LPCWSTR m_applicationName;
	HINSTANCE m_hinstance;
	HWND m_hwnd;

	InputClass* m_Input;
	ApplicationClass* m_Application;
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

static SystemClass* ApplicationHandle = 0;

#endif