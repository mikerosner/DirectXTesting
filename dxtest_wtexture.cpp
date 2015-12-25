// include the basic windows header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <D3dx9tex.h>

// include the Direct3D Library file
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

// global declaration
LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;    // the pointer to the device class
LPDIRECT3DTEXTURE9 ppTexture;
LPDIRECT3DVERTEXBUFFER9 v_buffer = NULL;    // the pointer to the vertex buffer

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

// function prototypes
void initD3D(HWND hWnd);    // sets up and initializes Direct3D
void render_frame(void);    // renders a single frame
void cleanD3D(void);    // closes Direct3D and releases memory
void init_graphics(void);    // 3D declarations

struct CUSTOMVERTEX { FLOAT X, Y, Z, RHW; FLOAT tu, tv; };
#define CUSTOMFVF (D3DFVF_XYZRHW|D3DFVF_TEX1)

						// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	// the handle for the window, filled by a function
	HWND hWnd;
	// this struct holds information for the window class
	WNDCLASSEX wc;

	// clear out the window class for use
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	// fill in the struct with the needed information
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "WindowClass1";

	// register the window class
	RegisterClassEx(&wc);

	// create the window and use the result as the handle
	hWnd = CreateWindowEx(NULL,
		"WindowClass1",    // name of the window class
		"Our First Direct3D Program",    // title of the window
		WS_EX_TOPMOST | WS_POPUP,    // window style
		0,    // x-position of the window
		0,    // y-position of the window
		SCREEN_WIDTH,    // width of the window
		SCREEN_HEIGHT,    // height of the window
		NULL,    // we have no parent window, NULL
		NULL,    // we aren't using menus, NULL
		hInstance,    // application handle
		NULL);    // used with multiple windows, NULL

				  // display the window on the screen
	ShowWindow(hWnd, nCmdShow);

	// set up and initialize Direct3D
	initD3D(hWnd);

	// enter the main loop:

	// this struct holds Windows event messages
	MSG msg;

	// Enter the infinite message loop
	while (TRUE)
	{
		// Check to see if any messages are waiting in the queue
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			// translate keystroke messages into the right format
			TranslateMessage(&msg);

			// send the message to the WindowProc function
			DispatchMessage(&msg);
		}

		// If the message is WM_QUIT, exit the while loop
		if (msg.message == WM_QUIT)
			break;

		render_frame();
		// Run game code here
		// ...
		// ...
	}

	// clean up DirectX and COM
	cleanD3D();

	// return this part of the WM_QUIT message to Windows
	return msg.wParam;
}

// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// sort through and find what code to run for the message given
	switch (message)
	{
			// this message is read when the window is closed
		case WM_DESTROY:
		{
			// close the application entirely
			PostQuitMessage(0);
			return 0;
		} break;
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE) PostQuitMessage(0);
			return 0;
		} break;
	}

	// Handle any messages the switch statement didn't
	return DefWindowProc(hWnd, message, wParam, lParam);
}

// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface
	D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = FALSE;    // program fullscreen, not windowed
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;    // set the back buffer format to 32-bit
	d3dpp.BackBufferWidth = SCREEN_WIDTH;    // set the width of the buffer
	d3dpp.BackBufferHeight = SCREEN_HEIGHT;    // set the height of the buffer

											   // create a device class using this information and information from the d3dpp stuct
	d3d->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev);

	init_graphics();    // call the function to initialize the triangle

	const char *FileName1 = "C:\\Users\\Michael\\Downloads\\dbz_ssj_goku_sprite_sheet_by_cy689-d3ajy0h.png";
	HRESULT X;

	X = D3DXCreateTextureFromFile(d3ddev,
		FileName1,
		&ppTexture);
		
}

// this is the function used to render a single frame
void render_frame(void)
{
	// clear the window to a deep blue
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

	d3ddev->BeginScene();    // begins the 3D scene

        // select which vertex format we are using
        d3ddev->SetFVF(CUSTOMFVF);

		// select the vertex buffer to display
		d3ddev->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX));

		d3ddev->SetTexture(0, ppTexture);
		d3ddev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);


		HRESULT X;

		// copy the vertex buffer to the back buffer
		X = d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
		//d3ddev->DrawPrimitive(D3DPT_LINELIST, 0, 6);
		//d3ddev->DrawPrimitive(D3DPT_POINTLIST, 0, 3);

	d3ddev->EndScene();    // ends the 3D scene

	d3ddev->Present(NULL, NULL, NULL, NULL);    // displays the created frame
}

// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
	v_buffer->Release();    // close and release the vertex buffer
	ppTexture->Release();	//close and release Texture object
	d3ddev->Release();    // close and release the 3D device
	d3d->Release();    // close and release Direct3D
}

// this is the function that puts the 3D models into video RAM
void init_graphics(void)
{
	// create the vertices using the CUSTOMVERTEX struct
	CUSTOMVERTEX vertices[] =
	{
		{ 200.0f, 200.0f, 0.5f, 1.0f, 0.0f, 0.0f, },
		{ 800.0f, 200.0f, 0.5f, 1.0f, 1.0f, 0.0f, },
		{ 800.0f, 600.0f, 0.5f, 1.0f, 1.0f, 1.0f, },
		{ 800.0f, 600.0f, 0.5f, 1.0f, 1.0f, 1.0f, },
		{ 200.0f, 600.0f, 0.5f, 1.0f, 0.0f, 1.0f, },
		{ 200.0f, 200.0f, 0.5f, 1.0f, 0.0f, 0.0f, },
		
	};

	// create a vertex buffer interface called v_buffer
	d3ddev->CreateVertexBuffer(6 * sizeof(CUSTOMVERTEX),
		0,
		CUSTOMFVF,
		D3DPOOL_MANAGED,
		&v_buffer,
		NULL);

	VOID* pVoid;    // a void pointer

					// lock v_buffer and load the vertices into it
	v_buffer->Lock(0, 0, (void**)&pVoid, 0);
	memcpy(pVoid, vertices, sizeof(vertices));
	v_buffer->Unlock();
}
