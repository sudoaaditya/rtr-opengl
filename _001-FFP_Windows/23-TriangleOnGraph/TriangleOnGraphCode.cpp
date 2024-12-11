#include<stdio.h>	//header for FileIO
#include<windows.h>
#include<gl/GL.h>	//Basic OGL Header
#include<gl/GLU.h>	//header for gluPerspeective
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"opengl32.lib")
#define Win_Width 800
#define Win_Height 600

//global variables
bool gbFullScreen = false;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev;
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC hrc = NULL;
bool gbActiveWindow = false;
FILE *fptr = NULL;

float gfCoord = 0.0f;

LRESULT CALLBACK MyCallback(HWND, UINT, WPARAM, LPARAM);
void ToggleFullScreen();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

	//function dcel
	int initialize(void);
	void display(void);

	//variable decl
	MSG msg;
	WNDCLASSEX wndclass;
	HWND hwnd;
	TCHAR szApp[] = TEXT("TriGraph");
	int iRet = 0;
	bool bDone = false;

	if (fopen_s(&fptr, "TriGraphLog.txt", "w") != 0) {
		MessageBox(NULL, TEXT("Cannot Create log!!.."), TEXT("ErrMsg"), MB_OK);
		exit(0);
	}
	else {
		fprintf(fptr, "Log Created Successfully!!..\n\n");
	}


	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = MyCallback;
	wndclass.lpszClassName = szApp;
	wndclass.lpszMenuName = NULL;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szApp,
		TEXT("TriangleOnGraph"),
		WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
		100,
		100,
		Win_Width,
		Win_Height,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	iRet = initialize();
	if (iRet == -1) {
		fprintf(fptr, "ChosePixelFormat Failed!!\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -2) {
		fprintf(fptr, "SetPixelFormat Failed!!\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -3) {
		fprintf(fptr, "CreateContext Failed!!\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -4) {
		fprintf(fptr, "wglMakeCurrent Failed!!\n");
		DestroyWindow(hwnd);
	}
	else {
		fprintf(fptr, "Initialization Successful!!\n");
	}
	ToggleFullScreen();
	ShowWindow(hwnd, iCmdShow);

	SetFocus(hwnd);
	SetForegroundWindow(hwnd);

	//game loop
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
			if (gbActiveWindow == true) {
				//call to Update
			}
			display();
		}
	}
	return((int)msg.wParam);
}

LRESULT CALLBACK MyCallback(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	//function declaration.
	void ToggleFullScreen(void);
	void resize(int, int);
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

	case WM_ERASEBKGND:
		return(0);

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
		fprintf(fptr, "ChoosePixelFormat Successful!!..\n");
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) {
		return(-2);
	}
	else {
		fprintf(fptr, "SetPixelFormat SuccessFul!!..\n");
	}

	hrc = wglCreateContext(ghdc);
	if (hrc == NULL) {
		return(-3);
	}
	else {
		fprintf(fptr, "wglCreateContext SuccessFul!!..\n");
	}

	if (wglMakeCurrent(ghdc, hrc) == FALSE) {
		return(-4);
	}
	else {
		fprintf(fptr, "wglMakeCurrent SuccessFul!!..\n");
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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

	glClear(GL_COLOR_BUFFER_BIT);

	glTranslatef(0.0f, 0.0f, -1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLineWidth(3.0f);
	glBegin(GL_LINES);
	
	//X-axis
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(2.2f, 0.0f, 0.0f);
	glVertex3f(-2.2f, 0.0f, 0.0f);

	//Y-axis
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 2.0f, 0.0f);
	glVertex3f(0.0f, -2.0f, 0.0f);
	
	glEnd();

	//Graph
	gfCoord = 0.05f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLineWidth(1.0f);
	glTranslatef(0.0f, 0.0f, -1.0f);
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f);

	for (; gfCoord <= 2.0f; gfCoord += 0.06f) {
	
		glVertex3f(2.0f, gfCoord, 0.0f);
		glVertex3f(-2.0f, gfCoord, 0.0f);

		glVertex3f(-gfCoord, 2.0f, 0.0f);
		glVertex3f(-gfCoord, -2.0f, 0.0f);


		glVertex3f(gfCoord, 2.0f, 0.0f);
		glVertex3f(gfCoord, -2.0f, 0.0f);

		glVertex3f(2.0f, -gfCoord, 0.0f);
		glVertex3f(-2.0f, -gfCoord, 0.0f);

	}
	glEnd();

	//Triabgle
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -1.0f);
	glLineWidth(4.0f);

	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 0.0f);
	//left edge
	glVertex3f(0.0f, 0.78f, 0.0f);
	glVertex3f(-1.01f, -0.71f, 0.0f);
	//right edge
	glVertex3f(0.0f, 0.78f, 0.0f);
	glVertex3f(1.01f, -0.71f, 0.0f);
	//bottom edge
	glVertex3f(-1.01f, -0.71f, 0.0f);
	glVertex3f(1.01f, -0.71f, 0.0f);
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

	if (wglGetCurrentContext() == hrc) {
		wglMakeCurrent(NULL, NULL);
	}
	if (hrc) {
		wglDeleteContext(hrc);
		hrc = NULL;
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


