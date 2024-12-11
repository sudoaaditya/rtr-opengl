#include<stdio.h>
#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>

#define _USE_MATH_DEFINES 1
#include<math.h>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define LineToDrawCircle 560

//global variables
//For full screen 
bool gbFullScreen = false;
DWORD dwStyle;
HWND ghwnd = NULL;
WINDOWPLACEMENT wpPrev;
//for OGl
HDC ghdc = NULL;
HGLRC ghrc = NULL;
bool gbActivateWindow = false;
FILE *fptr = NULL;

//callback
LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevinstance, LPSTR lpszCmdLine, int iCmdShow) {

	//function & variables
	int initialize(void);
	void display(void);

	MSG msg;
	HWND hwnd;
	WNDCLASSEX wndclass;
	TCHAR szAppName[] = TEXT("OGL_PERSP");
	bool bDone = false;
	int iRet = 0;

	if (fopen_s(&fptr, "ConTriLog.txt", "w") != 0) {
		MessageBox(NULL, TEXT("Cannot Create Log!!.."), TEXT("ErrMsg"), MB_OKCANCEL);
		exit(0);
	}
	else {
		fprintf(fptr, "Log Created Successfully!!..\n\n");
	}

	wndclass.cbSize = sizeof(wndclass);
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
		TEXT("ConTriangles"),
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	iRet = initialize();

	if (iRet == -1) {
		fprintf(fptr, "ChoosePixelFormat Failed!!..\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -2) {
		fprintf(fptr, "SetPixelFormat Failed!!..\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -3) {
		fprintf(fptr, "wglCreateContext Failed!!..\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -4) {
		fprintf(fptr, "wglmakeCurrent Failed!!..\n");
		DestroyWindow(hwnd);
	}
	else {
		fprintf(fptr, "Initializtion Successeded!!..\n");
	}

	ShowWindow(hwnd, iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//game Loop

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
			if (gbActivateWindow == true) {
				//call to update
			}
			display();
		}
	}

	return((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	//Functions
	void resize(int, int);
	void uninitialize(void);
	void ToggleFullScreen(void);

	static bool bIsMax = false;

	switch (iMsg) {

	case WM_SETFOCUS:
		gbActivateWindow = true;
		break;

	case WM_KILLFOCUS:
		gbActivateWindow = false;
		break;

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_KEYDOWN:
		switch (wParam) {

		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 'f':
		case 'F':
			ToggleFullScreen();
			break;

		case 'S':
		case 's':
			if (!bIsMax) {
				ShowWindow(hwnd, SW_MAXIMIZE);
				bIsMax = true;
			}
			else {
				ShowWindow(hwnd, SW_SHOWNORMAL);
				bIsMax = false;
			}
			break;
		}
		break;

	case WM_ERASEBKGND:
		return(0);

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen() {

	MONITORINFO mi = { sizeof(MONITORINFO) };

	if (gbFullScreen == false) {

		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW) {

			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi)) {

				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

				SetWindowPos(ghwnd,
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

		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowPos(ghwnd,
			HWND_TOP,
			0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbFullScreen = false;
	}
}

int initialize(void) {

	//varb & func
	void resize(int, int);

	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cBlueBits = 8;
	pfd.cGreenBits = 8;
	pfd.cAlphaBits = 8;

	ghdc = GetDC(ghwnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0) {
		return(-1);
	}
	else {
		fprintf(fptr, "ChoosePixelFormat Successful!!..(Index = %d)\n", iPixelFormatIndex);
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) {
		return(-2);
	}
	else {
		fprintf(fptr, "SetPixelFormat SuccessFul!!..\n");
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL) {
		return(-3);
	}
	else {
		fprintf(fptr, "wglCreateContext SuccessFul!!..\n");
	}

	if (wglMakeCurrent(ghdc, ghrc) == FALSE) {
		return(-4);
	}
	else {
		fprintf(fptr, "wglMakeCurrent SuccessFul!!..\n");
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	resize(WIN_WIDTH, WIN_HEIGHT);

	return(0);
}

void resize(int width, int height) {

	if (height == 0) {
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f,
		((GLfloat)width / (GLfloat)height),
		0.1f,
		100.0f);
}


void display(void) {

	GLfloat fxCord = 1.0f, fyCord = 1.0f;
	GLfloat fR = 1.0f, fG = 1.0f, fB = 1.0f;

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -3.0f);
	glLineWidth(2.0f);

	glBegin(GL_LINES);

	for (int i = 0; i < 8; i++) {

		if (i == 0) {
			fR = 1.0f;
			fG = 0.0f;
			fB = 0.0f;
		}
		else if (i == 1) {
			fR = 0.0f;
			fG = 1.0f;
			fB = 0.0f;
		}
		else if (i == 2) {
			fR = 0.0f;
			fG = 0.0f;
			fB = 1.0f;
		}
		else if (i == 3) {
			fR = 0.0f;
			fG = 1.0f;
			fB = 1.0f;
		}
		else if (i == 4) {
			fR = 1.0f;
			fG = 0.0f;
			fB = 1.0f;
		}
		else if (i == 5) {
			fR = 1.0f;
			fG = 1.0f;
			fB = 0.0f;
		}
		else if (i == 6) {
			fR = 1.0f;
			fG = 0.5f;
			fB = 0.0f;
		}
		else if (i == 7) {
			fR = 0.5f;
			fG = 0.5f;
			fB = 0.5f;
		}
		glColor3f(fR, fG, fB);

		glVertex3f(0.0f, fyCord, 0.0f);
		glVertex3f(-fxCord, -fyCord, 0.0f);
		glVertex3f(-fxCord, -fyCord, 0.0f);
		glVertex3f(fxCord, -fyCord, 0.0f);
		glVertex3f(fxCord, -fyCord, 0.0f);
		glVertex3f(0.0f, fyCord, 0.0f);

		fxCord = fxCord - 0.12f;
		fyCord = fyCord - 0.12f;


	}
	glEnd();


	SwapBuffers(ghdc);
}


void uninitialize(void) {

	if (gbFullScreen == true) {

		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowPos(ghwnd,
			HWND_TOP,
			0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
	}

	if (wglGetCurrentContext() == ghrc) {
		wglMakeCurrent(NULL, NULL);
	}
	if (ghrc) {
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}
	if (ghdc) {
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}
	if (fptr) {
		fprintf(fptr, "\nFile Closed Successfully!!..\n");
		fclose(fptr);
		fptr = NULL;
	}
}

