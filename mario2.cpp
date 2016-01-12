#define DEBUG_MODE

#ifdef DEBUG_MODE
#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540
#define WINDOWS_STYLE WS_POPUP
#define WINDOWED_YN true
#else
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define WINDOWS_STYLE WS_EX_TOPMOST | WS_POPUP
#define WINDOWED_YN false
#endif

// include the basic windows header file
#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include <thread>
#include <d3d9.h>
#include <D3dx9tex.h> //C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Lib\x86   C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Include
#include "sprites2.h"

// include the Direct3D Library file
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")

// global declaration
time_t StartTime, CurrTime, TDebug1, TDebug2;
int FrameCount;
LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;    // the pointer to the device class
LPDIRECT3DVERTEXBUFFER9 v_buffer = NULL;    // the pointer to the vertex buffer
SpriteSheet *SStemp;
Sprite* S1;
Dict *SpriteSheetDict;
SpriteStack *SpriteStackMain;
bool debug_run = true;
bool graphics_lock = false;
bool kill_threads = false;


// function prototypes
void initD3D(HWND hWnd);    // sets up and initializes Direct3D
void cleanD3D(void);    // closes Direct3D and releases memory
void UpdateGraphics(void);
MSG MainGameLoop(void); //Main Game Loop

struct CUSTOMVERTEX { FLOAT X, Y, Z, RHW; FLOAT tu, tv; };
CUSTOMVERTEX *vertices = nullptr;
CUSTOMVERTEX vertex_debug;
#define CUSTOMFVF (D3DFVF_XYZRHW|D3DFVF_TEX1)

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

//INITIALIZATION METHODS


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
		WINDOWS_STYLE,    // window style
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
	MSG msg;
	msg = MainGameLoop();

	// clean up DirectX and COM
	cleanD3D();

	// return this part of the WM_QUIT message to Windows
	return msg.wParam;
}

// this function initializes and prepares Direct3D for use// Create initial sprites, backgrounds, and render stack - should only have to run once
void initD3D(HWND hWnd)
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface
	D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

	ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
	d3dpp.Windowed = WINDOWED_YN;    // program fullscreen, not windowed
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
	d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
	d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;    // set the back buffer format to 32-bit
	d3dpp.BackBufferWidth = SCREEN_WIDTH;    // set the width of the buffer
	d3dpp.BackBufferHeight = SCREEN_HEIGHT;    // set the height of the buffer

											   // create a device class using this information and information from the d3dpp stuct
	d3d->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev);

	SpriteSheetDict = DictCreate();
	SpriteStackMain = SpriteStackCreate();

	xmlSpriteConfig("C:\\Users\\Michael\\Documents\\Visual Studio 2015\\Projects\\DevelopmentTesting\\XMLTest\\mario.xml", d3ddev, SpriteSheetDict, SpriteStackMain);
	SpriteStack *temp;
	temp = SpriteStackMain;
	while ((temp = temp->Next) != nullptr)
	{
		if (temp->s->PlayerYN)
		{
			S1 = temp->s;
		}
	}
	
	/*
	S2 = (Sprite*)malloc(sizeof(Sprite));
	SpriteStackAdd(S2, SpriteStackMain);
	S2->Controller = NULL;
	S2->Height = SCREEN_HEIGHT;
	S2->Width = SCREEN_WIDTH;
	S2->Ys = SCREEN_HEIGHT - S2->Height;
	S2->Xs = SCREEN_WIDTH - S2->Width;
	S2->Phys.Xspd = 0;
	S2->Phys.Yspd = 0;
	S2->Phys.XAcceleration = 0;
	S2->Phys.IdleDeclerate = 0;
	S2->Phys.FallSpeed = 0;
	S2->Phys.MaxSpeed = 0;
	S2->Phys.JumpSpeed = 0;
	S2->Phys.OnGround = 1;
	S2->Phys.FacingDirection = FACING_LEFT;
	S2->CurrAct = ACTION_IDLE;
	S2->EnemyYN = 0; //UNUSED
	S2->PlayerYN = 0; //UNUSED
	S2->ProjectileYN = 0; //UNUSED
	S2->Time = 0;
	S2->ActDict = ActionDict; //Contains Pointers to SpriteActionStep Roots
	S2->CurrAct = ACTION_IDLE;
	strcpy_s(S2->CurrActString, 16, "BACKGROUND_IDLE");
	S2->SS = (SpriteSheet*)DictLookUp(SpriteSheetDict, "background");
	S2->CurrActImage = ((SpriteActionStep*)DictLookUp(ActionDict, S2->CurrActString))->ActionStepArray;

		
	S1 = (Sprite*)malloc(sizeof(Sprite));
	SpriteStackAdd(S1, SpriteStackMain);
	S1->Controller = &ContPlayer;
	S1->Height = 100;
	S1->Width = 180;
	S1->Ys = SCREEN_HEIGHT - S1->Height;
	S1->Xs = 200;
	S1->Phys.Xspd = 0;
	S1->Phys.Yspd = 0;
	S1->Phys.XAcceleration = 10;
	S1->Phys.IdleDeclerate = .25;
	S1->Phys.FallSpeed = 1;
	S1->Phys.MaxSpeed = 10;
	S1->Phys.JumpSpeed = 50;
	S1->Phys.OnGround = 1;
	S1->Phys.FacingDirection = FACING_LEFT;
	S1->EnemyYN = 0; //UNUSED
	S1->PlayerYN = 1; //UNUSED
	S1->ProjectileYN = 0; //UNUSED
	S1->Time = 0;
	S1->ActDict = ActionDict; //Contains Pointers to SpriteActionStep Roots
	S1->CurrAct = ACTION_IDLE;
	strcpy_s(S1->CurrActString, 9, "RUNRIGHT");
	S1->SS = (SpriteSheet*)DictLookUp(SpriteSheetDict, "mageattack");
	S1->CurrActImage = ((SpriteActionStep*)DictLookUp(ActionDict, S1->CurrActString))->ActionStepArray;
	*/
}

MSG MainGameLoop(void)
{
	// this struct holds Windows event messages
	MSG msg;
	SpriteStack *temp;
	StartTime = clock();
	FrameCount = 0;

	std::thread graphics (UpdateGraphics);

	// Enter the infinite message loop
	while (TRUE)
	{
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
			/*
			time_diff = difftime(current_time, last_time);
			if (time_diff > (1 / 60))*/
			break;
		}

		// If the message is WM_QUIT, exit the while loop
		if (msg.message == WM_QUIT)
			break;

		CurrTime = clock();
		FrameCount = (int)(((float)CurrTime - (float)StartTime) / CLOCKS_PER_SEC * 60);
		while (graphics_lock);
		graphics_lock = true;
		temp = SpriteStackMain;
		while((temp = temp->Next) != nullptr)
		{
			if(temp->s->Controller!=nullptr) temp->s->Controller(FrameCount, temp->s);
		}
		graphics_lock = false;
		Sleep(17);
	}
	kill_threads = true;
	graphics.join();
	return msg;
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
		switch (wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
		case VK_LEFT:
			S1->CurrAct = ACTION_MOVE_LEFT;
			break;
		case VK_RIGHT:
			S1->CurrAct = ACTION_MOVE_RIGHT;
			break;
		case VK_UP:
			S1->PriorAct = S1->CurrAct;
			S1->CurrAct = ACTION_JUMP;
			break;
		}
	} break; 
	case WM_KEYUP:
	{
		switch (wParam)
		{
		case VK_LEFT:
			S1->CurrAct = ACTION_IDLE;
			break;
		case VK_RIGHT:
			S1->CurrAct = ACTION_IDLE;
			break;
		case VK_UP:
			break;
		}
	} break;
	}

	// Handle any messages the switch statement didn't
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void DumpVertex(FILE *f, CUSTOMVERTEX *x, int last_array)
{
	for (int i = 0; i <= last_array; i++) {
		fprintf_s(f, "%f", x[i].X);
		fprintf_s(f, ", ");
		fprintf_s(f, "%f", x[i].Y);
		fprintf_s(f, ", ");
		fprintf_s(f, "%f", x[i].Z);
		fprintf_s(f, ", ");
		fprintf_s(f, "%f", x[i].RHW);
		fprintf_s(f, ", ");
		fprintf_s(f, "%f", x[i].tu);
		fprintf_s(f, ", ");
		fprintf_s(f, "%f", x[i].tv);
		fprintf_s(f, "\n");
	}
}

// this is the function that puts the 3D models & textures into video RAM
void UpdateGraphics(void)
{
	int i;
	float Tl, Tr, Tt, Tb, Sl, Sr, St, Sb, SSh, SSw;
	SpriteStack *temp;

	while (kill_threads == false)
	{
		while (graphics_lock);
		graphics_lock = true;

		temp = SpriteStackMain;
		for (i = 0; (temp = temp->Next) != nullptr; i++);

		vertices = (CUSTOMVERTEX*)realloc(vertices, sizeof(CUSTOMVERTEX)*i * 6);

		temp = SpriteStackMain;
		for (i = 0; (temp = temp->Next) != nullptr; i++) {
			// create the vertices using the CUSTOMVERTEX struct
			Tl = (float)((temp->s->CurrActImage->Frame->x));
			Tr = (float)((temp->s->CurrActImage->Frame->x) + (temp->s->CurrActImage->Frame->w));
			Tt = (float)((temp->s->CurrActImage->Frame->y));
			Tb = (float)((temp->s->CurrActImage->Frame->y) + (temp->s->CurrActImage->Frame->h));
			Sl = (float)temp->s->Xs;
			Sr = (float)(temp->s->Xs + temp->s->Width);
			St = (float)temp->s->Ys;
			Sb = (float)(temp->s->Ys + temp->s->Height);
			SSh = (float)temp->s->CurrActImage->Frame->Sheet->SSHeight;
			SSw = (float)temp->s->CurrActImage->Frame->Sheet->SSWidth;

			vertices[i * 6 + 0] = { Sl, St, 0.5f, 1.0f, (Tl / SSw), (Tt / SSh), };
			vertices[i * 6 + 1] = { Sr, St, 0.5f, 1.0f, (Tr / SSw), (Tt / SSh), };
			vertices[i * 6 + 2] = { Sr, Sb, 0.5f, 1.0f, (Tr / SSw), (Tb / SSh), };
			vertices[i * 6 + 3] = { Sr, Sb, 0.5f, 1.0f, (Tr / SSw), (Tb / SSh), };
			vertices[i * 6 + 4] = { Sl, Sb, 0.5f, 1.0f, (Tl / SSw), (Tb / SSh), };
			vertices[i * 6 + 5] = { Sl, St, 0.5f, 1.0f, (Tl / SSw), (Tt / SSh), };


			vertex_debug = vertices[i * 6 + 0];
		}

		graphics_lock = false;

		if (debug_run) {
			FILE *file_output;
			if ((fopen_s(&file_output, "debug.log", "w")) != NULL)
			{
				printf("Error opening file\n");
				exit(1);
			}
			debug_run = false;
			DumpVertex(file_output, vertices, 11);
			fclose(file_output);
		}
		// create a vertex buffer interface called v_buffer
		d3ddev->CreateVertexBuffer(sizeof(CUSTOMVERTEX)*i * 6,
			0,
			CUSTOMFVF,
			D3DPOOL_MANAGED,
			&v_buffer,
			NULL);

		VOID* pVoid;    // a void pointer

						// lock v_buffer and load the vertices into it
		v_buffer->Lock(0, 0, (void**)&pVoid, 0);
		memcpy(pVoid, vertices, sizeof(CUSTOMVERTEX)*i * 6);
		v_buffer->Unlock();


		//RENDER FRAME

		temp = SpriteStackMain;
		// clear the window to a deep blue
		d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 40, 100), 1.0f, 0);

		d3ddev->BeginScene();    // begins the 3D scene

								 // select which vertex format we are using
		d3ddev->SetFVF(CUSTOMFVF);

		// select the vertex buffer to display
		d3ddev->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX));

		d3ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);    // turn on the color blending
		d3ddev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);    // set source factor
		d3ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);    // set dest factor
		d3ddev->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);    // set the operation

		d3ddev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		d3ddev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		d3ddev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		d3ddev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);


		HRESULT X;

		// copy the vertex buffer to the back buffer

		for (i = 0; (temp = temp->Next) != nullptr; i++) {
			d3ddev->SetTexture(0, temp->s->CurrActImage->Frame->Sheet->TextureLink);
			X = d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, i * 6, 2);
		}

		d3ddev->EndScene();    // ends the 3D scene

		d3ddev->Present(NULL, NULL, NULL, NULL);    // displays the created frame

		Sleep(17);
	}
}

// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{
	v_buffer->Release();    // close and release the vertex buffer
	d3ddev->Release();    // close and release the 3D device
	d3d->Release();    // close and release Direct3D
	SpriteStack *temp;
	temp = SpriteStackMain->Next;
	while (temp != nullptr) {
		free(temp->s);
		temp = temp->Next;
	}
	SpriteStackRelease(SpriteStackMain);
	SpriteSheetDictRelease(SpriteSheetDict); // Do not use dict release on these - object is a pointer that's targets need to be freed
	free(vertices);
}
