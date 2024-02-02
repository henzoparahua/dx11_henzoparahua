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


	// Store the vsync setting.
	m_vsync_enabled = vsync;

//	Before initializing Direct3D we have to get the refresh rate from the video card/monitor.
// 	Each computer may be slightly different so we will need to query for that information.
// 	We query the numerator and denominator values and then pass them to DirectX during the
// 	setup and itwill calculate the proper refresh rate. If we don't do this and just set the
// 	refresh rate to default value which may not exist on all computers then DX will respond
// 	by performing a blit instead of a buffer flip which will degrade performance and give us
//	annoying errors in the debug output.
// 

// Create a DirectX graphics interface factory.
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result))
	{
		return false;
	}

// Use the factory to create an adapter for the primary graphics interface (video card).
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

// Enumerate the primary adapter output (monitor).
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM display format for the adapter output (monitor).
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

// Create a list to hold all the possible display modes for this monitor/video card combination.
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

// Now fill the display mode list structures.
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

// Now go through all the display modes and find the one that matches the screen width and height.
// When a match is found store the numerator and denominator of the refresh rate for that monitor.
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenWidth)
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

// Get the adapter (video card) description.
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

// Store the dedicated video card memory in megabytes.
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

// Convert the name of the video card to a character array and store it.
	error = wcstombs_s(&stringLength, m_videoCardDescription, 128, adapterDesc.Description, 128);
	if (error != 0)
	{
		return false;
	}

//	Now that we have stored the numerator and denominator for the refresh rate and the video card information
//	we can release the structures and interfaces used to get that information.
	
// Release the display mode list.
	delete[] displayModeList;
	displayModeList = 0;

// Release the adapter output.
	adapterOutput->Release();
	adapterOutput = 0;

// Release the adapter.
	adapter->Release();
	adapter = 0;

// Release the factory.
	factory->Release();
	factory = 0;

//	Now with the refresh rate from the system, we can start the DirectX initialization.
//	The first thing we'll do is fill out the description of the swap chain.
//	The Swap Chain is the front and back buffer to which the graphics will be drawn.
//	Generally, you use a single back buffer, do all your drawing to it, and then swap it
//	to the front buffer which then displays on the user's screen. That's why its called swap chain.

// Initialize the swap chain description.
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

// Set to a single back buffer.
	swapChainDesc.BufferCount = 1;

// Set the width and height of the back buffer.
	swapChainDesc.BufferDesc.Width = screenWidth;
	swapChainDesc.BufferDesc.Height = screenHeight;

// Set regular 32-bit surface for the back buffer.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;


//	The next part of the description of the swap chain is the refresh rate. The refresh rate is how
//	many times it draws the back buffer to the front buffer. If vsync is set to true in our application.h
//	header then this will lock the refresh rate to the system settings (for example 60hz). That means it
//	will only draw the screen 60 times a second (or higher if the system refresh rate is more than 60).
//	However, if we set vsync to false, then it will draw the screen as many times as it can, despite
//	it may cause some visual artifacts.

// Set the refresh rate of the back buffer.
	if (m_vsync_enabled)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

// Set the usage of the back buffer.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

// Set the handle for the window to render to.
	swapChainDesc.OutputWindow = hwnd;

// Turn multisampling off.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

// Set to full screen or windowed mode.
	if (fullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

// Set the scan line ordering and scaling to unspecified.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

// Discard the back buffer contents after presenting.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

// Don't set the advanced flags.
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

// Create the swap chain, Direct3D device, and Direct3D device context.
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, NULL, &m_deviceContext);
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

// Get the pointer to the back buffer.
	result = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result))
	{
		return false;
	}

// Create the render target view with the back buffer pointer.
	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

// Release pointer to the back buffer as we no longer need it.
	backBufferPtr->Release();
	backBufferPtr = 0;

//	We will also need to set up a depth buffer description. We'll use this to create a depth
//	buffer so that our polygons can be rendered properly in 3D space. At the same time,
//	we will attach a stencil buffer to our depth buffer. The stencil buffer can be used to
//	achieve effects such as motion blur, volumetric shadows, and other things.

// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

// Set up the description of the depth buffer.
	depthBufferDesc.Width = screenWidth;
	depthBufferDesc.Height = screenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

//	Now we create the depth/stencil buffer using that description. You will notice we use
//	the CreateTexture2D function to make the buffer, hence the bufer is just a 2D texture.
//	The reason for this is that once your polygons are sorted and then rasterized, they just
//	ends up being colored pixels in this 2D buffer. Then this 2D buffer is drawn to the screen.

// Create the texture for the depth buffer using the filled out description.
	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

//	Now we need to setup the depth stencil description. This allows us to control what type of
//	depth test Direct3D will do for each pixel.

// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

// Create the depth stencil state.
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result))
	{
		return false;
	}

// Set the depth stencil state.
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

//	The next thing we need to create is the description of the view of the depth
//	stencil buffer. We do this so that Direct3D knows to use the depth buffer as
//	a depth stencil texture. After filling out the description we then call the
//	function CreateDepthStencilView to create it.

// Initialize the depth stencil view.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

// Create the depth stencil view.
	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

//	With that created we can now call OMSetRenderTargets. This will bind the
//	render target view and the depth stencil buffer to the output render pi-
//	peline. This way the graphics that the pipeline renders will get drawn to
//	our back buffer that we previously created. With the graphics written to
//	the back buffer we can then swap it to the front and display our graphics
//	on the users screen.
// 
// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);


//	Now that the render targets are setup, we can continue on to some extra
//	functions that will give us more control over our scenes for future tutorials.
// 	First thing is we'll create is a rasterizer state. This will give us control
// 	over how polygons are rendered. We can do tings like make our scenes render
// 	wireframe mode or have DirectX draw both in front and back faces of polygons.
//	By default, DirectX already has a rasterizer state set up and working the 
// 	exact same as the one below but you have no control to change it unless you set
// 	up one yourself.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// Create the rasterizer state from the description we just filled out.
	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result))
	{
		return false;
	}

	// Now set the rasterizer state.
	m_deviceContext->RSSetState(m_rasterState);

//	The viewport also need to be setup so that Direct3D can map clip space
//	coordinates to the render target space. Set this to be the entire size
//	of the window.

// Setup the viewport for rendering.
	m_viewport.Width = (float)screenWidth;
	m_viewport.Height = (float)screenHeight;
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

// Create the viewport.
	m_deviceContext->RSSetViewports(1, &m_viewport);

//	Now we will create the matrix. The projetion matrix is used translate
//	the 3D scene into the 2D viewport space that we previously created.
//	We will need to keep a copy of this matrix so that we can pass it to
//	our shaders that will be used to render our scenes.	

// Setup the projection matrix.
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)screenWidth / (float)screenHeight;

// Create the projection matrix for 3D rendering.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

//	We will also create another matrix called the world matrix. This matrix
// 	is used to convert the vertices of our objects into vertices in the 3D
// 	scene. This matrix will also be used to rotate, translate, and scale our
// 	objects in 3D space. From the start we will just initialize the matrix to
// 	the identity matrix and keep a copy of it in this object. The copy will be
// 	needed to be passed to the shaders for rendering also.

//	Initialize the world matrix to the identity matrix.
	m_worldMatrix = XMMatrixIdentity();

//	This is where you would generally create a view matrix. The view matrix is
//	used to calculate the position of where we are looking at the scene from.
// 	You can think of it as a camera and you only view the scene through this
// 	camera. Because of its purpose, we are going to create it in a camera class
// 	in later tutorial since logically it fits better there and just skip it for
// 	now.

//	The final thing we will setup in the initialize function is an orthographic
//	projection matrix. This matrix is used for rendering 2D elements like user
//	interfaces on the screen allowing us to skip the 3D rendering. 
// Create an orthographic projection matrix for 2D rendering.
	m_orthoMatrix = XMMatrixOrthographicLH((float)screenWidth, (float)screenHeight, screenNear, screenDepth);

	return true;
}

//	Before doing the shutdown function, releasing and cleaning up all pointers, we
//	need to force the swap chain to go into windowed mode first. If this is not done
//	and you try to release the swap chain in full screen mode it will throw some 
//	exceptions. So, to avoid that we just always force windowed mode before shutting down.

void D3DClass::Shutdown()
{

// Before shutting down set to windowed mode or when you release the swap chain it will throw an exception.
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(false, NULL);
	}

	if (m_rasterState)
	{
		m_rasterState->Release();
		m_rasterState = 0;
	}

	if (m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = 0;
	}

	if (m_depthStencilState)
	{
		m_depthStencilState->Release();
		m_depthStencilState = 0;
	}

	if (m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = 0;
	}

	if (m_renderTargetView)
	{
		m_renderTargetView->Release();
		m_renderTargetView = 0;
	}

	if (m_deviceContext)
	{
		m_deviceContext->Release();
		m_deviceContext = 0;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = 0;
	}

	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = 0;
	}

	return;
}

//	The first two are BeginScene and EndScene. BeginScene will be are going
//	to draw a new 3D scene at the beginning of each frame. All it does is 
//	initializes the buffers so they are blank and ready to be drawn to. 
//	The other function is Endscene, it tells the swap chain to display
//	our 3D scene once all the drawing has completed at the end of each frame.
void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];


// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

// Clear the back buffer.
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);

// Clear the depth buffer.
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return;
}


void D3DClass::EndScene()
{
// Present the back buffer to the screen since rendering is complete.
	if (m_vsync_enabled)
	{
// Lock to screen refresh rate.
		m_swapChain->Present(1, 0);
	}
	else
	{
// Present as fast as possible.
		m_swapChain->Present(0, 0);
	}

	return;
}


//	These next functions simply get pointers to the Direct3D device and device context.
//	These helper functions will be called by the framework often.
ID3D11Device* D3DClass::GetDevice()
{
	return m_device;
}

ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return m_deviceContext;
}

//	The next helper functions give copies of the projection, world and orthographic
//	matrices to calling functions. Most shaders will need these matrices for rendering
//	so there needed to be an easy way for outside objects to get a copy of them.
void D3DClass::GetProjectionMatrix(XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
	return;
}

void D3DClass::GetWorldMatrix(XMMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
	return;
}

void D3DClass::GetOrthoMatrix(XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
	return;
}
void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDescription);
	memory = m_videoCardMemory;
	return;
}
//	The last two helper function will be used in the rener to texture tutorial later on.
void D3DClass::SetBackBufferRenderTarget()
{
	// Bind the render target view and depth stencil buffer to the output render pipeline.
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	return;
}
void D3DClass::ResetViewport()
{
	// Set the viewport.
	m_deviceContext->RSSetViewports(1, &m_viewport);

	return;
}