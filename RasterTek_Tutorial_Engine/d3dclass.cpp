#include "d3dclass.h"

D3DClass::D3DClass()
{
	m_swapChain = 0;
	m_device = 0;
	m_deviceContext = 0;
	m_renderTargetView = 0;
	m_depthStencilBuffer = 0;
	m_depthStencilState = 0;
	m_depthStencilView = 0;
	m_rasterState = 0;
}

D3DClass::D3DClass(const D3DClass& other)
{

}

D3DClass::~D3DClass()
{
}

bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator;
	unsigned long long stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	float fieldOfView, screenAspect;

	m_vsync_enabled = vsync;




//	Before initializing Direct3D we have to get the refresh rate from the video card/monitor.
// 	Each computer may be slightly different so we will need to query for that information.
// 	We query the numerator and denominator values and then pass them to DirectX during the
// 	setup and itwill calculate the proper refresh rate. If we don't do this and just set the
// 	refresh rate to default value which may not exist on all computers then DX will respond
// 	by performing a blit instead of a buffer flip which will degrade performance and give us
//	annoying errors in the debug output.
// 


//	Create a DirectX graphics interface factory
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}
	
//	Use the factory to create an adapter for the primary graphics interface (video card)
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

//	Enumerate the primary adapter output (monitor).
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

//	Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}
	
//	Create a list to all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

//	Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

//	Now go through all the display modes and find the one that matches the screen width and height
//	When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenHeight)
		{
			if (displayModeList[i].Height == (unsigned int)screenHeight)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}


//	Now we have numerator and denominator for refresh rate. The last thing we will retrieve using
//	the adapter is the name of the video card and the amount of video memory.

//	Get the adapter (video card) description.
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

//	Store the dedicated video card memory in megabytes.
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

//	Convert the name of the video card to a character array and store it.
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

//	Now that we have stored the numerator and denominator for the refresh rate and the video card information
//	we can release the structures and interfaces used to get that information.
	
//	Release the display mode list.
	delete[] displayModeList;
	displayModeList = 0;

//	Release the adapter output.
	adapterOutput->Release();
	adapterOutput = 0;

//	Release the adapter.
	adapter->Release();
	factory = 0;

//	Now with the refresh rate from the system, we can start the DirectX initialization.
//	The first thing we'll do is fill out the description of the swap chain.
//	The Swap Chain is the front and back buffer to which the graphics will be drawn.
//	Generally, you use a single back buffer, do all your drawing to it, and then swap it
//	to the front buffer which then displays on the user's screen. That's why its called swap chain.

//	Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

//	Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

//	Set the width and the height of the back buffer.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

//	Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


//	The next part of the description of the swap chain is the refresh rate. The refresh rate is how
//	many times it draws the back buffer to the front buffer. If vsync is set to true in our application.h
//	header then this will lock the refresh rate to the system settings (for example 60hz). That means it
//	will only draw the screen 60 times a second (or higher if the system refresh rate is more than 60).
//	However, if we set vsync to false, then it will draw the screen as many times as it can, despite
//	it may cause some visual artifacts.

//	Set the refresh rate of the back buffer.
	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	} else {
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

//	Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

//	Set the handle for the window to render to.
	swapChainDesc.OutputWindow = hwnd;

//	Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

//	Set to full screen or windowed mode.
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else {
		swapChainDesc.Windowed = true;
	}

//	Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

//	Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

//	Don't set the advanced flags.
	swapChainDesc.Flags = 0;

//	After setting up the swap chain description we also need to setup one more variable called
//	feature level. This variable tells DirectX what version we plan to use. Here we set the
//	feature level to DirectX 11. 
	featureLevel = D3D_FEATURE_LEVEL_11_0;

//	Now that the swap chain description and feature level have been filled out, we can create
//	the swap chain, the Direct3D device and the Direct3D device context. they're the interface 
//	to all the Direct3D functions. We will use them for almost everything.
// 
//	Note that if the user does not have a Direct 11 video card this function will fail to 
//	create the device and the device context.
//  

//	Create the swap chain, Direct3D device and Direct3D device context.
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION,
											&swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
	if (FAILED(result))
	{
		return false;
	}

//	Sometimes this call to create device will fail if the primary video card is not compatible
//	with DirectX 11. Some machines may have the primary card as DirectX 10 video card and the
//	secondary card as DirectX 11 video card. Also some hybrid graphics card work that way with
//	the primary being the low power Intel card and the secondary being the high power Nvidia card.
//	TO get around this you will need to not use the default device and instead enumerate all the
//	video cards in the machine and have the user choose which one to use and then specify that card
//	when creating a device.

//	Now that we have a swap chain, we need to get a pointer buffer and then attach it to the swap
//	chain. We'll use the CreateRenderTargetView function to attach the back buffer to our swap chain.

//	Get the pointer to the back buffer.
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result)) {
		return false;
	}

//	Create the render target view with the back buffer pointer.
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

//	Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;

}
