#include<stdio.h>	//header for FileIO
#include<math.h>	//mathematics
#include<windows.h>
#include<gl/GL.h>	//Basic OGL Header
#include<gl/GLU.h>	//header for gluPerspeective
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"opengl32.lib")
#define Win_Width 900
#define Win_Height 600

//global variables
bool gbFullScreen = false;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC hrc = NULL;
bool gbActiveWindow = false;
FILE *fptr = NULL;

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
	TCHAR szApp[] = TEXT("Primitive");
	int iRet = 0;
	bool bDone = false;

	if (fopen_s(&fptr, "PtLog.txt", "w") != 0) {
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
		TEXT("Premitives"),
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
		fprintf(fptr, "ChoosePixelFormat Successful = %d !!..\n", iPixelFormatIndex);
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
	//RGB = 9,31,45
	glClearColor(0.03529f, 0.12156f, 0.17647f, 1.0f);

	//resize(Win_Width, Win_Height);

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

	static float fx = -1.0f, fy = 1.0f, r = 0.18f;

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -3.0f);

	//Moon
	glLineWidth(4.0f);
	glBegin(GL_LINES);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	for (int i = 0; i < 360; i++) {
		glVertex3f(fx, fy, 0.0f);
		glVertex3f((float)(fx + r * cos(i)), (float)(fy + r * sin(i)), 0.0f);
	}
	glEnd();

	//glLineWidth(1.0f);
	//glBegin(GL_LINES);
	//glColor3f(1.0f, 1.0f, 1.0f);
	//Y-Axis
	//glVertex3f(0.0f, 1.3f, 0.0f);
	//glVertex3f(0.0f, -1.2f, 0.0f);
	//Perpandicluar in +ve X
	//glVertex3f(0.0f, -1.2f, 0.0f);
	//glVertex3f(2.26f, 0.2f, 0.0f);
	//Perpandicular in -ve X
	//glVertex3f(0.0f, -1.2f, 0.0f);
	//glVertex3f(-2.26f, 0.2f, 0.0f);

	//center x
	//glEnd();

	int number = 42; //number of vertices,
	float radius = 0.9f; // radius of the circle
	float twopi = 2.0f*3.14159f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -2.0f);
	glBegin(GL_TRIANGLE_FAN); //drawing circle using triangle fan
	//RGB = 131,164,183
	glColor3f(0.5137f, 0.6431f, 0.7176f); //blue in color
	glVertex2f(0.0f, 0.0f); //placing at the origin
	for (int i = 0; i <= 21; i++) {
		glVertex2f(radius*cosf(i*twopi / number), radius*sinf(i*twopi / number));
	}
	glEnd();


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -3.0f);

	glBegin(GL_POLYGON);
	//roof of buld1
	//RGB = 88,84,121
	glColor3f(0.34509f, 0.32941f, 0.47450f);
	glVertex3f(0.0f, 0.9f, 0.0f);
	glVertex3f(-0.5f, 0.7f, 0.0f);
	glVertex3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.5f, 0.7f, 0.0f);
	glEnd();
	glBegin(GL_POLYGON);
	//roof of buld +2
	//RGB = 88,84,121
	glColor3f(0.34509f, 0.32941f, 0.47450f);
	glVertex3f(0.8f, 0.6f, 0.0f);
	glVertex3f(0.4f, 0.4f, 0.0f);
	glVertex3f(0.8f, 0.2f, 0.0f);
	glVertex3f(1.2f, 0.4f, 0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	//roof of buld -2
	//RGB = 88,84,121
	glColor3f(0.34509f, 0.32941f, 0.47450f);
	glVertex3f(-0.8f, 0.6f, 0.0f);
	glVertex3f(-0.4f, 0.4f, 0.0f);
	glVertex3f(-0.8f, 0.2f, 0.0f);
	glVertex3f(-1.2f, 0.4f, 0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	//roof of Buld +3
	//RGB = 88,84,121
	glColor3f(0.34509f, 0.32941f, 0.47450f);
	glVertex3f(1.4f, 0.4f, 0.0f);
	glVertex3f(1.0f, 0.2f, 0.0f);
	glVertex3f(1.4f, 0.0f, 0.0f);
	glVertex3f(1.8f, 0.2f, 0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	//roof of Buld -3
	//RGB = 88,84,121
	glColor3f(0.34509f, 0.32941f, 0.47450f);
	glVertex3f(-1.4f, 0.4f, 0.0f);
	glVertex3f(-1.0f, 0.2f, 0.0f);
	glVertex3f(-1.4f, 0.0f, 0.0f);
	glVertex3f(-1.8f, 0.2f, 0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	//Polygon +1
	//RGB = 0,54,71
	glColor3f(0.0f, 0.21176f, 0.27843f);
	glVertex3f(1.8f, 0.2f, 0.0f);
	glVertex3f(1.4f, 0.0f, 0.0f);
	glVertex3f(1.4f, -0.33f, 0.0f);
	glVertex3f(1.8f, -0.09f, 0.0f);
	glEnd();
	
	glBegin(GL_POLYGON);
	//Polygon -1
	//RGB = 0,54,71
	glColor3f(0.0f, 0.21176f, 0.27843f);
	glVertex3f(-1.8f, 0.2f, 0.0f);
	glVertex3f(-1.4f, 0.0f, 0.0f);
	glVertex3f(-1.4f, -0.33f, 0.0f);
	glVertex3f(-1.8f, -0.09f, 0.0f);
	glEnd();

	//Border of +1 & -1
	glLineWidth(3.5f);
	glBegin(GL_LINES);
	//RGB = 74,125,146
	glColor3f(0.29019f, 0.49019f, 0.57544f);
	glVertex3f(1.8f, 0.2f, 0.0f);
	glVertex3f(1.4f, 0.0f, 0.0f);
	glVertex3f(1.4f, 0.0f, 0.0f);
	glVertex3f(1.4f, -0.33f, 0.0f);
	glVertex3f(1.4f, -0.33f, 0.0f);
	glVertex3f(1.8f, -0.09f, 0.0f);
	glVertex3f(1.8f, -0.09f, 0.0f);
	glVertex3f(1.8f, 0.2f, 0.0f);

	glVertex3f(-1.8f, 0.2f, 0.0f);
	glVertex3f(-1.4f, 0.0f, 0.0f);
	glVertex3f(-1.4f, 0.0f, 0.0f);
	glVertex3f(-1.4f, -0.33f, 0.0f);
	glVertex3f(-1.4f, -0.33f, 0.0f);
	glVertex3f(-1.8f, -0.09f, 0.0f);
	glVertex3f(-1.8f, -0.09f, 0.0f);
	glVertex3f(-1.8f, 0.2f, 0.0f);

	glVertex3f(1.4f, 0.4f, 0.0f);
	glVertex3f(1.0f, 0.2f, 0.0f);
	glVertex3f(1.0f, 0.2f, 0.0f);
	glVertex3f(1.4f, 0.0f, 0.0f);
	glVertex3f(1.4f, 0.0f, 0.0f);
	glVertex3f(1.8f, 0.2f, 0.0f);
	glVertex3f(1.8f, 0.2f, 0.0f);
	glVertex3f(1.4f, 0.4f, 0.0f);

	glVertex3f(-1.4f, 0.4f, 0.0f);
	glVertex3f(-1.0f, 0.2f, 0.0f);
	glVertex3f(-1.0f, 0.2f, 0.0f);
	glVertex3f(-1.4f, 0.0f, 0.0f);
	glVertex3f(-1.4f, 0.0f, 0.0f);
	glVertex3f(-1.8f, 0.2f, 0.0f);
	glVertex3f(-1.8f, 0.2f, 0.0f);
	glVertex3f(-1.4f, 0.4f, 0.0f);

	glEnd();

	glBegin(GL_POLYGON);
	//polygon +2
	//RGB = 0,65,105
	glColor3f(0.0f, 0.25490f, 0.41176f);
	glVertex3f(1.4f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.2f, 0.0f);
	glVertex3f(1.0f, -0.09f, 0.0f);
	glVertex3f(1.4f, -0.33f, 0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	//polygon -2
	//RGB = 0,65,105
	glColor3f(0.0f, 0.25490f, 0.41176f);
	glVertex3f(-1.4f, 0.0f, 0.0f);
	glVertex3f(-1.0f, 0.2f, 0.0f);
	glVertex3f(-1.0f, -0.09f, 0.0f);
	glVertex3f(-1.4f, -0.33f, 0.0f);
	glEnd();

	//Border of +2 & -2
	glLineWidth(3.5f);
	glBegin(GL_LINES);
	//RGB = 74,125,146
	glColor3f(0.29019f, 0.49019f, 0.57544f);

	glVertex3f(1.4f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.2f, 0.0f);
	glVertex3f(1.0f, 0.2f, 0.0f);
	glVertex3f(1.0f, -0.09f, 0.0f);
	glVertex3f(1.0f, -0.09f, 0.0f);
	glVertex3f(1.4f, -0.33f, 0.0f);
	glVertex3f(1.4f, -0.33f, 0.0f);
	glVertex3f(1.4f, 0.0f, 0.0f);

	glVertex3f(-1.4f, 0.0f, 0.0f);
	glVertex3f(-1.0f, 0.2f, 0.0f);
	glVertex3f(-1.0f, 0.2f, 0.0f);
	glVertex3f(-1.0f, -0.09f, 0.0f);
	glVertex3f(-1.0f, -0.09f, 0.0f);
	glVertex3f(-1.4f, -0.33f, 0.0f);
	glVertex3f(-1.4f, -0.33f, 0.0f);
	glVertex3f(-1.4f, 0.0f, 0.0f);

	glEnd();
	glBegin(GL_POLYGON);
	//Polygon +3
	//RGB = 0,54,71
	glColor3f(0.0f, 0.21176f, 0.27843f);
	glVertex3f(1.2f, 0.4f, 0.0f);
	glVertex3f(0.8f, 0.2f, 0.0f);
	glVertex3f(0.8f, -0.7f, 0.0f);
	glVertex3f(1.2f, -0.46f, 0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	//Polygon -3
	//RGB = 0,54,71
	glColor3f(0.0f, 0.21176f, 0.27843f);
	glVertex3f(-1.2f, 0.4f, 0.0f);
	glVertex3f(-0.8f, 0.2f, 0.0f);
	glVertex3f(-0.8f, -0.7f, 0.0f);
	glVertex3f(-1.2f, -0.46f, 0.0f);
	glEnd();

	//Border of +3 & -3
	glLineWidth(3.5f);
	glBegin(GL_LINES);
	//RGB = 74,125,146
	glColor3f(0.29019f, 0.49019f, 0.57544f);
	glVertex3f(0.8f, 0.6f, 0.0f);
	glVertex3f(0.4f, 0.4f, 0.0f);
	glVertex3f(0.4f, 0.4f, 0.0f);
	glVertex3f(0.8f, 0.2f, 0.0f);
	glVertex3f(0.8f, 0.2f, 0.0f);
	glVertex3f(1.2f, 0.4f, 0.0f);
	glVertex3f(1.2f, 0.4f, 0.0f);
	glVertex3f(0.8f, 0.6f, 0.0f);

	glVertex3f(-0.8f, 0.6f, 0.0f);
	glVertex3f(-0.4f, 0.4f, 0.0f);
	glVertex3f(-0.4f, 0.4f, 0.0f);
	glVertex3f(-0.8f, 0.2f, 0.0f);
	glVertex3f(-0.8f, 0.2f, 0.0f);
	glVertex3f(-1.2f, 0.4f, 0.0f);
	glVertex3f(-1.2f, 0.4f, 0.0f);
	glVertex3f(-0.8f, 0.6f, 0.0f);

	glVertex3f(1.2f, 0.4f, 0.0f);
	glVertex3f(0.8f, 0.2f, 0.0f);
	glVertex3f(0.8f, 0.2f, 0.0f);
	glVertex3f(0.8f, -0.7f, 0.0f);
	glVertex3f(0.8f, -0.7f, 0.0f);
	glVertex3f(1.2f, -0.46f, 0.0f);
	glVertex3f(1.2f, -0.46f, 0.0f);
	glVertex3f(1.2f, 0.4f, 0.0f);

	glVertex3f(-1.2f, 0.4f, 0.0f);
	glVertex3f(-0.8f, 0.2f, 0.0f);
	glVertex3f(-0.8f, 0.2f, 0.0f);
	glVertex3f(-0.8f, -0.7f, 0.0f);
	glVertex3f(-0.8f, -0.7f, 0.0f);
	glVertex3f(-1.2f, -0.46f, 0.0f);
	glVertex3f(-1.2f, -0.46f, 0.0f);
	glVertex3f(-1.2f, 0.4f, 0.0f);

	glEnd();

	glBegin(GL_POLYGON);
	//Polygon +4
	//RGB = 0,65,105
	glColor3f(0.0f, 0.25490f, 0.41176f);
	glVertex3f(0.8f, 0.2f, 0.0f);
	glVertex3f(0.4f, 0.4f, 0.0f);
	glVertex3f(0.4f, -0.46f, 0.0f);
	glVertex3f(0.8f, -0.7f, 0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	//Polygon -4
	//RGB = 0,65,105
	glColor3f(0.0f, 0.25490f, 0.41176f);
	glVertex3f(-0.8f, 0.2f, 0.0f);
	glVertex3f(-0.4f, 0.4f, 0.0f);
	glVertex3f(-0.4f, -0.46f, 0.0f);
	glVertex3f(-0.8f, -0.7f, 0.0f);
	glEnd();

	//Border of +4 & -4
	glLineWidth(3.5f);
	glBegin(GL_LINES);
	//RGB = 74,125,146
	glColor3f(0.29019f, 0.49019f, 0.57544f);
	glVertex3f(0.8f, 0.2f, 0.0f);
	glVertex3f(0.4f, 0.4f, 0.0f);
	glVertex3f(0.4f, 0.4f, 0.0f);
	glVertex3f(0.4f, -0.46f, 0.0f);
	glVertex3f(0.4f, -0.46f, 0.0f);
	glVertex3f(0.8f, -0.7f, 0.0f);
	glVertex3f(0.8f, -0.7f, 0.0f);
	glVertex3f(0.8f, 0.2f, 0.0f);

	glVertex3f(-0.8f, 0.2f, 0.0f);
	glVertex3f(-0.4f, 0.4f, 0.0f);
	glVertex3f(-0.4f, 0.4f, 0.0f);
	glVertex3f(-0.4f, -0.46f, 0.0f);
	glVertex3f(-0.4f, -0.46f, 0.0f);
	glVertex3f(-0.8f, -0.7f, 0.0f);
	glVertex3f(-0.8f, -0.7f, 0.0f);
	glVertex3f(-0.8f, 0.2f, 0.0f);

	glEnd();

	glBegin(GL_POLYGON);
	//polygon+5
	//RGB = 0,54,71
	glColor3f(0.0f, 0.21176f, 0.27843f);
	glVertex3f(0.5f, 0.7f, 0.0f);
	glVertex3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, -1.2f, 0.0f);
	glVertex3f(0.5f, -0.89f, 0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	//polygon-5
	//RGB = 0,54,71
	glColor3f(0.0f, 0.21176f, 0.27843f);
	glVertex3f(-0.5f, 0.7f, 0.0f);
	glVertex3f(-0.0f, 0.5f, 0.0f);
	glVertex3f(-0.0f, -1.2f, 0.0f);
	glVertex3f(-0.5f, -0.89f, 0.0f);
	glEnd();

	glLineWidth(3.5f);
	glBegin(GL_LINES);
	//Border to  Buld 1
	//RGB = 74,125,146
	glColor3f(0.29019f, 0.49019f, 0.57544f);
	glVertex3f(0.0f, 0.9f, 0.0f);
	glVertex3f(-0.5f, 0.7f, 0.0f);
	glVertex3f(-0.5f, 0.7f, 0.0f);
	glVertex3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.5f, 0.7f, 0.0f);
	glVertex3f(0.5f, 0.7f, 0.0f);
	glVertex3f(0.0f, 0.9f, 0.0f);

	glVertex3f(0.5f, 0.7f, 0.0f);
	glVertex3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, -1.2f, 0.0f);
	glVertex3f(0.0f, -1.2f, 0.0f);
	glVertex3f(0.5f, -0.89f, 0.0f);
	glVertex3f(0.5f, -0.89f, 0.0f);
	glVertex3f(0.5f, 0.7f, 0.0f);

	glVertex3f(-0.5f, 0.7f, 0.0f);
	glVertex3f(-0.0f, 0.5f, 0.0f);
	glVertex3f(-0.0f, 0.5f, 0.0f);
	glVertex3f(-0.0f, -1.2f, 0.0f);
	glVertex3f(-0.0f, -1.2f, 0.0f);
	glVertex3f(-0.5f, -0.89f, 0.0f);
	glVertex3f(-0.5f, -0.89f, 0.0f);
	glVertex3f(-0.5f, 0.7f, 0.0f);
	glEnd();
	
	//Everyrhing For Windows

	//wind 1.0
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(1.50f, -0.018f, 0.0f);
	glVertex3f(1.43f, -0.07f, 0.0f);
	glVertex3f(1.43f, -0.20f, 0.0f);
	glVertex3f(1.50f, -0.15f, 0.0f);
	glEnd();

	//WInd -1.0
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(-1.50f, -0.018f, 0.0f);
	glVertex3f(-1.43f, -0.07f, 0.0f);
	glVertex3f(-1.43f, -0.20f, 0.0f);
	glVertex3f(-1.50f, -0.15f, 0.0f);
	glEnd();

	//Window Borders...
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	//RGB = 74,125,146
	glColor3f(0.29019f, 0.49019f, 0.57544f);
	
	glVertex3f(1.50f, -0.018f, 0.0f);
	glVertex3f(1.43f, -0.07f, 0.0f);
	glVertex3f(1.43f, -0.20f, 0.0f);
	glVertex3f(1.43f, -0.07f, 0.0f);
	glVertex3f(1.43f, -0.20f, 0.0f);
	glVertex3f(1.50f, -0.15f, 0.0f);
	glVertex3f(1.50f, -0.15f, 0.0f);
	glVertex3f(1.50f, -0.018f, 0.0f);

	glVertex3f(-1.50f, -0.018f, 0.0f);
	glVertex3f(-1.43f, -0.07f, 0.0f);
	glVertex3f(-1.43f, -0.20f, 0.0f);
	glVertex3f(-1.43f, -0.07f, 0.0f);
	glVertex3f(-1.43f, -0.20f, 0.0f);
	glVertex3f(-1.50f, -0.15f, 0.0f);
	glVertex3f(-1.50f, -0.15f, 0.0f);
	glVertex3f(-1.50f, -0.018f, 0.0f);
	glEnd();


	//wind 1.1
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(1.77f, 0.12f, 0.0f);
	glVertex3f(1.7f, 0.068f, 0.0f);
	glVertex3f(1.7f, -0.058f, 0.0f);
	glVertex3f(1.77f, -0.01f, 0.0f);
	glEnd();

	//wind -1.1
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(-1.77f, 0.12f, 0.0f);
	glVertex3f(-1.7f, 0.068f, 0.0f);
	glVertex3f(-1.7f, -0.058f, 0.0f);
	glVertex3f(-1.77f, -0.01f, 0.0f);
	glEnd();

	//Window Borders...
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	//RGB = 74,125,146
	glColor3f(0.29019f, 0.49019f, 0.57544f);

	glVertex3f(1.77f, 0.12f, 0.0f);
	glVertex3f(1.7f, 0.068f, 0.0f);
	glVertex3f(1.7f, 0.068f, 0.0f);
	glVertex3f(1.7f, -0.058f, 0.0f);
	glVertex3f(1.7f, -0.058f, 0.0f);
	glVertex3f(1.77f, -0.01f, 0.0f);
	glVertex3f(1.77f, -0.01f, 0.0f);
	glVertex3f(1.77f, 0.12f, 0.0f);

	glVertex3f(-1.77f, 0.12f, 0.0f);
	glVertex3f(-1.7f, 0.068f, 0.0f);
	glVertex3f(-1.7f, 0.068f, 0.0f);
	glVertex3f(-1.7f, -0.058f, 0.0f);
	glVertex3f(-1.7f, -0.058f, 0.0f);
	glVertex3f(-1.77f, -0.01f, 0.0f);
	glVertex3f(-1.77f, -0.01f, 0.0f);
	glVertex3f(-1.77f, 0.12f, 0.0f);

	glEnd();

	//Wind 1.3
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(1.65f, -0.049f, 0.0f);
	glVertex3f(1.55f, -0.11f, 0.0f);
	glVertex3f(1.55f, -0.23f, 0.0f);
	glVertex3f(1.65f, -0.17f, 0.0f);
	glEnd();

	//Wind -1.3
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(-1.65f, -0.049f, 0.0f);
	glVertex3f(-1.55f, -0.11f, 0.0f);
	glVertex3f(-1.55f, -0.23f, 0.0f);
	glVertex3f(-1.65f, -0.17f, 0.0f);
	glEnd();

	//Window Borders...
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	//RGB = 74,125,146
	glColor3f(0.29019f, 0.49019f, 0.57544f);

	glVertex3f(1.65f, -0.049f, 0.0f);
	glVertex3f(1.55f, -0.11f, 0.0f);
	glVertex3f(1.55f, -0.11f, 0.0f);
	glVertex3f(1.55f, -0.23f, 0.0f);
	glVertex3f(1.55f, -0.23f, 0.0f);
	glVertex3f(1.65f, -0.17f, 0.0f);
	glVertex3f(1.65f, -0.17f, 0.0f);
	glVertex3f(1.65f, -0.049f, 0.0f);

	glVertex3f(-1.65f, -0.049f, 0.0f);
	glVertex3f(-1.55f, -0.11f, 0.0f);
	glVertex3f(-1.55f, -0.11f, 0.0f);
	glVertex3f(-1.55f, -0.23f, 0.0f);
	glVertex3f(-1.55f, -0.23f, 0.0f);
	glVertex3f(-1.65f, -0.17f, 0.0f);
	glVertex3f(-1.65f, -0.17f, 0.0f);
	glVertex3f(-1.65f, -0.049f, 0.0f);
	glEnd();
	
	//Wind 2.0
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(1.0f, 0.21f, 0.0f);
	glVertex3f(0.9f, 0.15f, 0.0f);
	glVertex3f(0.9f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.05f, 0.0f);
	glEnd();

	//Wind 2.1
	glBegin(GL_POLYGON);
	//RGB = 2, 59,76
	glColor3f(0.00784f, 0.32137f, 0.29803f);
	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.9f, -0.057f, 0.0f);
	glVertex3f(0.9f, -0.35f, 0.0f);
	glVertex3f(1.0f, -0.3f, 0.0f);
	glEnd();

	//Wind 2.3
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(1.0f, -0.36f, 0.0f);
	glVertex3f(0.9f, -0.41f, 0.0f);
	glVertex3f(0.9f, -0.55f, 0.0f);
	glVertex3f(1.0f, -0.49f, 0.0f);
	glEnd();

	//Wind -2.0
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(-1.0f, 0.21f, 0.0f);
	glVertex3f(-0.9f, 0.15f, 0.0f);
	glVertex3f(-0.9f, 0.0f, 0.0f);
	glVertex3f(-1.0f, 0.05f, 0.0f);
	glEnd();

	//Wind -2.1
	glBegin(GL_POLYGON);
	//RGB = 2, 59,76
	glColor3f(0.00784f, 0.32137f, 0.29803f);
	glVertex3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-0.9f, -0.057f, 0.0f);
	glVertex3f(-0.9f, -0.35f, 0.0f);
	glVertex3f(-1.0f, -0.3f, 0.0f);
	glEnd();

	//Wind 2.3
	glBegin(GL_POLYGON);
	//RGB = 2, 59,76
	glColor3f(0.00784f, 0.32137f, 0.29803f);
	glVertex3f(-1.0f, -0.36f, 0.0f);
	glVertex3f(-0.9f, -0.41f, 0.0f);
	glVertex3f(-0.9f, -0.55f, 0.0f);
	glVertex3f(-1.0f, -0.49f, 0.0f);
	glEnd();

	//Window Borders...
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	//RGB = 74,125,146
	glColor3f(0.29019f, 0.49019f, 0.57544f);

	glVertex3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.9f, -0.057f, 0.0f);
	glVertex3f(0.9f, -0.057f, 0.0f);
	glVertex3f(0.9f, -0.35f, 0.0f);
	glVertex3f(0.9f, -0.35f, 0.0f);
	glVertex3f(1.0f, -0.3f, 0.0f);
	glVertex3f(1.0f, -0.3f, 0.0f);
	glVertex3f(1.0f, 0.0f, 0.0f);


	glVertex3f(1.0f, -0.36f, 0.0f);
	glVertex3f(0.9f, -0.41f, 0.0f);
	glVertex3f(0.9f, -0.41f, 0.0f);
	glVertex3f(0.9f, -0.55f, 0.0f);
	glVertex3f(0.9f, -0.55f, 0.0f);
	glVertex3f(1.0f, -0.49f, 0.0f);
	glVertex3f(1.0f, -0.49f, 0.0f);
	glVertex3f(1.0f, -0.36f, 0.0f);

	glVertex3f(1.0f, 0.21f, 0.0f);
	glVertex3f(0.9f, 0.15f, 0.0f);
	glVertex3f(0.9f, 0.15f, 0.0f);
	glVertex3f(0.9f, 0.0f, 0.0f);
	glVertex3f(0.9f, 0.0f, 0.0f);
	glVertex3f(1.0f, 0.05f, 0.0f);
	glVertex3f(1.0f, 0.05f, 0.0f);
	glVertex3f(1.0f, 0.21f, 0.0f);

	//-ve
	glVertex3f(-1.0f, 0.0f, 0.0f);
	glVertex3f(-0.9f, -0.057f, 0.0f);
	glVertex3f(-0.9f, -0.057f, 0.0f);
	glVertex3f(-0.9f, -0.35f, 0.0f);
	glVertex3f(-0.9f, -0.35f, 0.0f);
	glVertex3f(-1.0f, -0.3f, 0.0f);
	glVertex3f(-1.0f, -0.3f, 0.0f);
	glVertex3f(-1.0f, 0.0f, 0.0f);


	glVertex3f(-1.0f, -0.36f, 0.0f);
	glVertex3f(-0.9f, -0.41f, 0.0f);
	glVertex3f(-0.9f, -0.41f, 0.0f);
	glVertex3f(-0.9f, -0.55f, 0.0f);
	glVertex3f(-0.9f, -0.55f, 0.0f);
	glVertex3f(-1.0f, -0.49f, 0.0f);
	glVertex3f(-1.0f, -0.49f, 0.0f);
	glVertex3f(-1.0f, -0.36f, 0.0f);

	glVertex3f(-1.0f, 0.21f, 0.0f);
	glVertex3f(-0.9f, 0.15f, 0.0f);
	glVertex3f(-0.9f, 0.15f, 0.0f);
	glVertex3f(-0.9f, 0.0f, 0.0f);
	glVertex3f(-0.9f, 0.0f, 0.0f);
	glVertex3f(-1.0f, 0.05f, 0.0f);
	glVertex3f(-1.0f, 0.05f, 0.0f);
	glVertex3f(-1.0f, 0.21f, 0.0f);

	glEnd();

	//Wind 3.1
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(0.18f, 0.50f, 0.0f);
	glVertex3f(0.07f, 0.45f, 0.0f);
	glVertex3f(0.07f, 0.25f, 0.0f);
	glVertex3f(0.18f, 0.31f, 0.0f);
	glEnd();

	//Wind 3.2
	glBegin(GL_POLYGON);
	//RGB = 2, 59,76
	glColor3f(0.00784f, 0.32137f, 0.29803f);
	glVertex3f(0.18f, 0.26f, 0.0f);
	glVertex3f(0.07f, 0.20f, 0.0f);
	glVertex3f(0.07f, -0.1f, 0.0f);
	glVertex3f(0.18f, -0.05f, 0.0f);
	glEnd();
	
	//Wind 3.3
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(0.18f, -0.1f, 0.0f);
	glVertex3f(0.07f, -0.15f, 0.0f);
	glVertex3f(0.07f, -0.45f, 0.0f);
	glVertex3f(0.18f, -0.4f, 0.0f);
	glEnd();

	//Wind 3.4
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(0.18f, -0.45f, 0.0f);
	glVertex3f(0.07f, -0.50f, 0.0f);
	glVertex3f(0.07f, -0.70f, 0.0f);
	glVertex3f(0.18f, -0.65f, 0.0f);
	glEnd();

	//Wind 3.5
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(0.18f, -0.70f, 0.0f);
	glVertex3f(0.07f, -0.75f, 0.0f);
	glVertex3f(0.07f, -1.05f, 0.0f);
	glVertex3f(0.18f, -1.0f, 0.0f);
	glEnd();

	//Wind 3.1.1
	glBegin(GL_POLYGON);
	//RGB = 2, 59,76
	glColor3f(0.00784f, 0.32137f, 0.29803f);
	glVertex3f(0.43f, 0.60f, 0.0f);
	glVertex3f(0.30f, 0.54f, 0.0f);
	glVertex3f(0.30f, 0.35f, 0.0f);
	glVertex3f(0.43f, 0.41f, 0.0f);
	glEnd();

	//Wind 3.1.2
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(0.43f, 0.36f, 0.0f);
	glVertex3f(0.30f, 0.30f, 0.0f);
	glVertex3f(0.30f, 0.00f, 0.0f);
	glVertex3f(0.43f, 0.05f, 0.0f);
	glEnd();

	//Wind 3.1.3
	glBegin(GL_POLYGON);
	//RGB = 2, 59,76
	glColor3f(0.00784f, 0.32137f, 0.29803f);
	glVertex3f(0.43f, 0.00f, 0.0f);
	glVertex3f(0.30f, -0.05f, 0.0f);
	glVertex3f(0.30f, -0.35f, 0.0f);
	glVertex3f(0.43f, -0.30f, 0.0f);
	glEnd();

	//Wind 3.1.4
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(0.43f, -0.35f, 0.0f);
	glVertex3f(0.30f, -0.40f, 0.0f);
	glVertex3f(0.30f, -0.60f, 0.0f);
	glVertex3f(0.43f, -0.55f, 0.0f);
	glEnd();

	//Wind 3.1.5
	glBegin(GL_POLYGON);
	//RGB = 2, 59,76
	glColor3f(0.00784f, 0.32137f, 0.29803f);
	glVertex3f(0.43f, -0.60f, 0.0f);
	glVertex3f(0.30f, -0.66f, 0.0f);
	glVertex3f(0.30f, -0.93f, 0.0f);
	glVertex3f(0.43f, -0.85f, 0.0f);
	glEnd();

	//Wind -3.1
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(-0.18f, 0.50f, 0.0f);
	glVertex3f(-0.07f, 0.45f, 0.0f);
	glVertex3f(-0.07f, 0.25f, 0.0f);
	glVertex3f(-0.18f, 0.31f, 0.0f);
	glEnd();

	//Wind -3.2
	glBegin(GL_POLYGON);
	//RGB = 2, 59,76
	glColor3f(0.00784f, 0.32137f, 0.29803f);
	glVertex3f(-0.18f, 0.26f, 0.0f);
	glVertex3f(-0.07f, 0.20f, 0.0f);
	glVertex3f(-0.07f, -0.1f, 0.0f);
	glVertex3f(-0.18f, -0.05f, 0.0f);
	glEnd();

	//Wind -3.3
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(-0.18f, -0.1f, 0.0f);
	glVertex3f(-0.07f, -0.15f, 0.0f);
	glVertex3f(-0.07f, -0.45f, 0.0f);
	glVertex3f(-0.18f, -0.4f, 0.0f);
	glEnd();

	//Wind -3.4
	glBegin(GL_POLYGON);
	//RGB = 2, 59,76
	glColor3f(0.00784f, 0.32137f, 0.29803f);
	glVertex3f(-0.18f, -0.45f, 0.0f);
	glVertex3f(-0.07f, -0.50f, 0.0f);
	glVertex3f(-0.07f, -0.70f, 0.0f);
	glVertex3f(-0.18f, -0.65f, 0.0f);
	glEnd();

	//Wind -3.5
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(-0.18f, -0.70f, 0.0f);
	glVertex3f(-0.07f, -0.75f, 0.0f);
	glVertex3f(-0.07f, -1.05f, 0.0f);
	glVertex3f(-0.18f, -1.0f, 0.0f);
	glEnd();

	//Wind -3.1.1
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(-0.43f, 0.60f, 0.0f);
	glVertex3f(-0.30f, 0.54f, 0.0f);
	glVertex3f(-0.30f, 0.35f, 0.0f);
	glVertex3f(-0.43f, 0.41f, 0.0f);
	glEnd();

	//Wind -3.1.2
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(-0.43f, 0.36f, 0.0f);
	glVertex3f(-0.30f, 0.30f, 0.0f);
	glVertex3f(-0.30f, 0.00f, 0.0f);
	glVertex3f(-0.43f, 0.05f, 0.0f);
	glEnd();

	//Wind -3.1.3
	glBegin(GL_POLYGON);
	//RGB = 2, 59,76
	glColor3f(0.00784f, 0.32137f, 0.29803f);
	glVertex3f(-0.43f, 0.00f, 0.0f);
	glVertex3f(-0.30f, -0.05f, 0.0f);
	glVertex3f(-0.30f, -0.35f, 0.0f);
	glVertex3f(-0.43f, -0.30f, 0.0f);
	glEnd();

	//Wind -3.1.4
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(-0.43f, -0.35f, 0.0f);
	glVertex3f(-0.30f, -0.40f, 0.0f);
	glVertex3f(-0.30f, -0.60f, 0.0f);
	glVertex3f(-0.43f, -0.55f, 0.0f);
	glEnd();

	//Wind -3.1.5
	glBegin(GL_POLYGON);
	//RGB = 253,231,148
	glColor3f(0.99215f, 0.90588f, 0.58039f);
	glVertex3f(-0.43f, -0.60f, 0.0f);
	glVertex3f(-0.30f, -0.66f, 0.0f);
	glVertex3f(-0.30f, -0.93f, 0.0f);
	glVertex3f(-0.43f, -0.85f, 0.0f);
	glEnd();


	//Window Borders...
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	//RGB = 74,125,146
	glColor3f(0.29019f, 0.49019f, 0.57544f);
	//+ve
	glVertex3f(0.18f, 0.50f, 0.0f);
	glVertex3f(0.07f, 0.45f, 0.0f);
	glVertex3f(0.07f, 0.45f, 0.0f);
	glVertex3f(0.07f, 0.25f, 0.0f);
	glVertex3f(0.07f, 0.25f, 0.0f);
	glVertex3f(0.18f, 0.31f, 0.0f);
	glVertex3f(0.18f, 0.31f, 0.0f);
	glVertex3f(0.18f, 0.50f, 0.0f);

	glVertex3f(0.18f, 0.26f, 0.0f);
	glVertex3f(0.07f, 0.20f, 0.0f);
	glVertex3f(0.07f, 0.20f, 0.0f);
	glVertex3f(0.07f, -0.1f, 0.0f);
	glVertex3f(0.07f, -0.1f, 0.0f);
	glVertex3f(0.18f, -0.05f, 0.0f);
	glVertex3f(0.18f, -0.05f, 0.0f);
	glVertex3f(0.18f, 0.26f, 0.0f);

	glVertex3f(0.18f, -0.1f, 0.0f);
	glVertex3f(0.07f, -0.15f, 0.0f);
	glVertex3f(0.07f, -0.15f, 0.0f);
	glVertex3f(0.07f, -0.45f, 0.0f);
	glVertex3f(0.07f, -0.45f, 0.0f);
	glVertex3f(0.18f, -0.4f, 0.0f);
	glVertex3f(0.18f, -0.4f, 0.0f);
	glVertex3f(0.18f, -0.1f, 0.0f);

	glVertex3f(0.18f, -0.45f, 0.0f);
	glVertex3f(0.07f, -0.50f, 0.0f);
	glVertex3f(0.07f, -0.50f, 0.0f);
	glVertex3f(0.07f, -0.70f, 0.0f);
	glVertex3f(0.07f, -0.70f, 0.0f);
	glVertex3f(0.18f, -0.65f, 0.0f);
	glVertex3f(0.18f, -0.65f, 0.0f);
	glVertex3f(0.18f, -0.45f, 0.0f);

	glVertex3f(0.18f, -0.70f, 0.0f);
	glVertex3f(0.07f, -0.75f, 0.0f);
	glVertex3f(0.07f, -0.75f, 0.0f);
	glVertex3f(0.07f, -1.05f, 0.0f);
	glVertex3f(0.07f, -1.05f, 0.0f);
	glVertex3f(0.18f, -1.0f, 0.0f);
	glVertex3f(0.18f, -1.0f, 0.0f);
	glVertex3f(0.18f, -0.70f, 0.0f);

	glVertex3f(0.43f, 0.60f, 0.0f);
	glVertex3f(0.30f, 0.54f, 0.0f);
	glVertex3f(0.30f, 0.54f, 0.0f);
	glVertex3f(0.30f, 0.35f, 0.0f);
	glVertex3f(0.30f, 0.35f, 0.0f);
	glVertex3f(0.43f, 0.41f, 0.0f);
	glVertex3f(0.43f, 0.41f, 0.0f);
	glVertex3f(0.43f, 0.60f, 0.0f);

	glVertex3f(0.43f, 0.36f, 0.0f);
	glVertex3f(0.30f, 0.30f, 0.0f);
	glVertex3f(0.30f, 0.30f, 0.0f);
	glVertex3f(0.30f, 0.00f, 0.0f);
	glVertex3f(0.30f, 0.00f, 0.0f);
	glVertex3f(0.43f, 0.05f, 0.0f);
	glVertex3f(0.43f, 0.05f, 0.0f);
	glVertex3f(0.43f, 0.36f, 0.0f);

	glVertex3f(0.43f, 0.00f, 0.0f);
	glVertex3f(0.30f, -0.05f, 0.0f);
	glVertex3f(0.30f, -0.05f, 0.0f);
	glVertex3f(0.30f, -0.35f, 0.0f);
	glVertex3f(0.30f, -0.35f, 0.0f);
	glVertex3f(0.43f, -0.30f, 0.0f);
	glVertex3f(0.43f, -0.30f, 0.0f);
	glVertex3f(0.43f, 0.00f, 0.0f);

	glVertex3f(0.43f, -0.35f, 0.0f);
	glVertex3f(0.30f, -0.40f, 0.0f);
	glVertex3f(0.30f, -0.40f, 0.0f);
	glVertex3f(0.30f, -0.60f, 0.0f);
	glVertex3f(0.30f, -0.60f, 0.0f);
	glVertex3f(0.43f, -0.55f, 0.0f);
	glVertex3f(0.43f, -0.55f, 0.0f);
	glVertex3f(0.43f, -0.35f, 0.0f);

	glVertex3f(0.43f, -0.60f, 0.0f);
	glVertex3f(0.30f, -0.66f, 0.0f);
	glVertex3f(0.30f, -0.66f, 0.0f);
	glVertex3f(0.30f, -0.93f, 0.0f);
	glVertex3f(0.30f, -0.93f, 0.0f);
	glVertex3f(0.43f, -0.85f, 0.0f);
	glVertex3f(0.43f, -0.85f, 0.0f);
	glVertex3f(0.43f, -0.60f, 0.0f);

	//-ve
	glVertex3f(-0.18f, 0.50f, 0.0f);
	glVertex3f(-0.07f, 0.45f, 0.0f);
	glVertex3f(-0.07f, 0.45f, 0.0f);
	glVertex3f(-0.07f, 0.25f, 0.0f);
	glVertex3f(-0.07f, 0.25f, 0.0f);
	glVertex3f(-0.18f, 0.31f, 0.0f);
	glVertex3f(-0.18f, 0.31f, 0.0f);
	glVertex3f(-0.18f, 0.50f, 0.0f);
	
	glVertex3f(-0.18f, 0.26f, 0.0f);
	glVertex3f(-0.07f, 0.20f, 0.0f);
	glVertex3f(-0.07f, 0.20f, 0.0f);
	glVertex3f(-0.07f, -0.1f, 0.0f);
	glVertex3f(-0.07f, -0.1f, 0.0f);
	glVertex3f(-0.18f, -0.05f, 0.0f);
	glVertex3f(-0.18f, -0.05f, 0.0f);
	glVertex3f(-0.18f, 0.26f, 0.0f);

	glVertex3f(-0.18f, -0.1f, 0.0f);
	glVertex3f(-0.07f, -0.15f, 0.0f);
	glVertex3f(-0.07f, -0.15f, 0.0f);
	glVertex3f(-0.07f, -0.45f, 0.0f);
	glVertex3f(-0.07f, -0.45f, 0.0f);
	glVertex3f(-0.18f, -0.4f, 0.0f);
	glVertex3f(-0.18f, -0.4f, 0.0f);
	glVertex3f(-0.18f, -0.1f, 0.0f);

	glVertex3f(-0.18f, -0.45f, 0.0f);
	glVertex3f(-0.07f, -0.50f, 0.0f);
	glVertex3f(-0.07f, -0.50f, 0.0f);
	glVertex3f(-0.07f, -0.70f, 0.0f);
	glVertex3f(-0.07f, -0.70f, 0.0f);
	glVertex3f(-0.18f, -0.65f, 0.0f);
	glVertex3f(-0.18f, -0.65f, 0.0f);
	glVertex3f(-0.18f, -0.45f, 0.0f);

	glVertex3f(-0.18f, -0.70f, 0.0f);
	glVertex3f(-0.07f, -0.75f, 0.0f);
	glVertex3f(-0.07f, -0.75f, 0.0f);
	glVertex3f(-0.07f, -1.05f, 0.0f);
	glVertex3f(-0.07f, -1.05f, 0.0f);
	glVertex3f(-0.18f, -1.0f, 0.0f);
	glVertex3f(-0.18f, -1.0f, 0.0f);
	glVertex3f(-0.18f, -0.70f, 0.0f);

	glVertex3f(-0.43f, 0.60f, 0.0f);
	glVertex3f(-0.30f, 0.54f, 0.0f);
	glVertex3f(-0.30f, 0.54f, 0.0f);
	glVertex3f(-0.30f, 0.35f, 0.0f);
	glVertex3f(-0.30f, 0.35f, 0.0f);
	glVertex3f(-0.43f, 0.41f, 0.0f);
	glVertex3f(-0.43f, 0.41f, 0.0f);
	glVertex3f(-0.43f, 0.60f, 0.0f);

	glVertex3f(-0.43f, 0.36f, 0.0f);
	glVertex3f(-0.30f, 0.30f, 0.0f);
	glVertex3f(-0.30f, 0.30f, 0.0f);
	glVertex3f(-0.30f, 0.00f, 0.0f);
	glVertex3f(-0.30f, 0.00f, 0.0f);
	glVertex3f(-0.43f, 0.05f, 0.0f);
	glVertex3f(-0.43f, 0.05f, 0.0f);
	glVertex3f(-0.43f, 0.36f, 0.0f);

	glVertex3f(-0.43f, 0.00f, 0.0f);
	glVertex3f(-0.30f, -0.05f, 0.0f);
	glVertex3f(-0.30f, -0.05f, 0.0f);
	glVertex3f(-0.30f, -0.35f, 0.0f);
	glVertex3f(-0.30f, -0.35f, 0.0f);
	glVertex3f(-0.43f, -0.30f, 0.0f);
	glVertex3f(-0.43f, -0.30f, 0.0f);
	glVertex3f(-0.43f, 0.00f, 0.0f);

	glVertex3f(-0.43f, -0.35f, 0.0f);
	glVertex3f(-0.30f, -0.40f, 0.0f);
	glVertex3f(-0.30f, -0.40f, 0.0f);
	glVertex3f(-0.30f, -0.60f, 0.0f);
	glVertex3f(-0.30f, -0.60f, 0.0f);
	glVertex3f(-0.43f, -0.55f, 0.0f);
	glVertex3f(-0.43f, -0.55f, 0.0f);
	glVertex3f(-0.43f, -0.35f, 0.0f);

	glVertex3f(-0.43f, -0.60f, 0.0f);
	glVertex3f(-0.30f, -0.66f, 0.0f);
	glVertex3f(-0.30f, -0.66f, 0.0f);
	glVertex3f(-0.30f, -0.93f, 0.0f);
	glVertex3f(-0.30f, -0.93f, 0.0f);
	glVertex3f(-0.43f, -0.85f, 0.0f);
	glVertex3f(-0.43f, -0.85f, 0.0f);
	glVertex3f(-0.43f, -0.60f, 0.0f);
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


