#include<windows.h>
#include<stdio.h>//Header For FILE
#include<gl/GL.h> //Header For OGL Functions
#pragma comment(lib,"opengl32.lib") //Lib Linked here
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//Parametr Declarations
HWND gHwnd = NULL;	
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
bool gbFullScreen = false;
HDC gHdc = NULL;
HGLRC gHrc = NULL;	//rendering context ha astoo
bool gbActiveWindow = false;
//FILE IO for Error Checking
FILE *gfptr;


LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szlpCmdLine, int iCmdShow) {

	//Function Declaration
	int initialize(void);

	//variable declarations
	bool bDone = false;
	int iRet = 0;
	MSG msg;
	HWND hwnd;
	WNDCLASSEX wndclass;
	TCHAR szAppName[] = TEXT("BluRendering");

	if (fopen_s(&gfptr, "Log.txt", "w") != 0) {
		MessageBox(NULL, TEXT("Cannot Create File!!.."), TEXT("FileErr"), MB_OK);
		exit(0);
	}
	else {
		fprintf(gfptr, "Log File Created SuccessFully!!..\n\n");
	}

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.lpfnWndProc = MyCallBack;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("Rendered Window"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	gHwnd = hwnd;

	//first call to OPENGL's function
	iRet = initialize();

	//now FILE IO to Store Possible Failures.
	if (iRet == -1) {
		fprintf(gfptr, "ChosePixelFormat Failed!!..\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -2) {
		fprintf(gfptr, "SetPixelFormat Failed!!..\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -3) {
		fprintf(gfptr, "wglCreateContext Failed!!..\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -4) {
		fprintf(gfptr, "wglMakeCurrent Failed!!..\n");
		DestroyWindow(hwnd);
	}
	else {
		fprintf(gfptr, "Initialization Successful!!..\n");
	}

	ShowWindow(hwnd, iCmdShow);
	
	//We Don't Call UpdateWindow() as it paints window but we are in OpenGL....

	//To Set Window On TOP in ZORDER
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);	

	//Here Comes GAME LOOP

	while (bDone == false) {

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

			if (msg.message == WM_QUIT) {
				bDone = true;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {

			//here is the code to play game.
			if (gbActiveWindow == true) {
				//we ll calll UpdateWindow() here;
			}
			//here we are supposed to call display() but we ll do it inWM_PAINT for this code
		}
	}

	return((int)msg.wParam);
}


LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	//Function declarartion
	void ToggleFullScreen(void);
	void resize(int, int);
	void display(void);
	void uninitialize(void);

	switch (iMsg) {

	case WM_SETFOCUS:
		gbActiveWindow = true;
		break;

	case WM_KILLFOCUS:
		gbActiveWindow = false;
		break;

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_PAINT:
		display();	// This Should be done only in Single Buffering
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_KEYDOWN:
		switch (wParam) {

		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
			
		case 'F':
		case 'f':
			ToggleFullScreen();
			break;
		}
		break;

	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

//OpenGL cha Code................

void ToggleFullScreen() {

	MONITORINFO mi = { sizeof(MONITORINFO) };

	if (gbFullScreen == false) {

		dwStyle = GetWindowLong(gHwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW) {

			if (GetWindowPlacement(gHwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(gHwnd, MONITORINFOF_PRIMARY), &mi)) {

				SetWindowLong(gHwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				
				SetWindowPos(gHwnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
		gbFullScreen = true;
	}

	else {

		SetWindowLong(gHwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(gHwnd, &wpPrev);

		SetWindowPos(gHwnd,
			HWND_TOP,
			0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbFullScreen = false;

	}
}


int initialize() {

	void resize(int, int);

	//variables declaration
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	memset((void*)&pfd, NULL, sizeof(PIXELFORMATDESCRIPTOR));
	//here we can also call ZeroMemory();

	pfd.nSize = sizeof(PPIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1; //One because OGL is deprecated on Windows at 1.5
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;

	//now get plain DC
	gHdc = GetDC(gHwnd);

	//Asking Os for index of her context of Rendering
	iPixelFormatIndex = ChoosePixelFormat(gHdc, &pfd);
	//the return index is always 1 based if it returns 0 then failure is obtained.

	if (iPixelFormatIndex == 0) {
		return(-1);
	}

	if (SetPixelFormat(gHdc, iPixelFormatIndex, &pfd) == FALSE) {
		return(-2);
	}

	//now will use Bridging api to let HDC to be Superman i.e HGLRC

	gHrc = wglCreateContext(gHdc);
	if (gHrc == NULL) {
		return(-3);
	}
	if (wglMakeCurrent(gHdc, gHrc) == FALSE) {
		return(-4);
	}

	//clear the screen by OpenGL Color
	glClearColor(0.0f, 0.0f,1.0f,1.0f);

	//warmup Call to RESIZE()
	resize(WIN_WIDTH, WIN_HEIGHT);

	return(0);
}

void resize(int width, int height) {

	//this function maintains our Viewport as size of window varies...
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

}

void display() {

	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();

}

void uninitialize(void) {

	//this is convention that we must close window after letting it come into normal mode if we are doin so in fullscreen
	//so ha code of normalizing screen.

	if (gbFullScreen == true) {

		SetWindowLong(gHwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(gHwnd, &wpPrev);

		SetWindowPos(gHwnd,
			HWND_TOP,
			0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
	}

	//No We will folloe COM's SafeRelease Style and c++ Destructor Rule.
	if (wglGetCurrentContext() == gHrc) {
		wglMakeCurrent(NULL, NULL); //Unsetting gHrc.
	}

	if (gHrc) {
		wglDeleteContext(gHrc);
		gHrc = NULL;
	}

	if (gHdc) {
		ReleaseDC(gHwnd,gHdc);
		gHdc = NULL;
	}

	//Now Closing File
	if (gfptr) {
		fprintf(gfptr, "\nClosing Log File!!..\n");
		fclose(gfptr);
		gfptr = NULL;
	}

}

