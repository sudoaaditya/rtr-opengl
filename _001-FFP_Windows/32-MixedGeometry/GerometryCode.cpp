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
#define LineToDrawCircle 1000

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

	if (fopen_s(&fptr, "GeometryLog.txt", "w") != 0) {
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
		TEXT("<<FreeGeometry>>"),
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

	GLfloat fx1, fx2, fx3, fy1, fy2, fy3;
	GLfloat fArea, fRad, fPer;
	GLfloat fdAB, fdBC, fdAC;
	GLfloat fxCord, fyCord;
	static GLfloat fAngle = 0.0f;
	static bool bToWrite = false;

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -1.0f);
	glLineWidth(2.0f);
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
	float gfCoord = 0.05f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLineWidth(0.5f);
	glTranslatef(0.0f, 0.0f, -2.0f);
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f);

	for (; gfCoord <= 2.0f; gfCoord += 0.06f) {

		//Horizontal Lines
		glVertex3f(2.0f, gfCoord, 0.0f);
		glVertex3f(-2.0f, gfCoord, 0.0f);

		glVertex3f(2.0f, -gfCoord, 0.0f);
		glVertex3f(-2.0f, -gfCoord, 0.0f);

		//Vertical lines
		glVertex3f(-gfCoord, 2.0f, 0.0f);
		glVertex3f(-gfCoord, -2.0f, 0.0f);

		glVertex3f(gfCoord, 2.0f, 0.0f);
		glVertex3f(gfCoord, -2.0f, 0.0f);

	}
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -4.0f);
	glLineWidth(2.0f);
	//coord of triangles
	fx1 = 0.0f;
	fy1 = 1.0f;
	fx2 = -1.0f;
	fy2 = -1.0f;
	fx3 = 1.0f;
	fy3 = -1.0f;

	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(fx1, fy1, 0.0f);
	glVertex3f(fx2, fy2, 0.0f);

	glVertex3f(fx2, fy2, 0.0f);
	glVertex3f(fx3, fy3, 0.0f);

	glVertex3f(fx3, fy3, 0.0f);
	glVertex3f(fx1, fy1, 0.0f);

	glEnd();

	//diatances of each side of triangle by distance formula
	fdAB = sqrtf(((fx2 - fx1)*(fx2 - fx1)) + ((fy2 - fy1)*(fy2 - fy1)));
	fdBC = sqrtf(((fx3 - fx2)*(fx3 - fx2)) + ((fy3 - fy2)*(fy3 - fy2)));
	fdAC = sqrtf(((fx3 - fx1)*(fx3 - fx1)) + ((fy3 - fy1)*(fy3 - fy1)));

	//perimeter of triangle >> A+B+C and we need half of it for area 
	fPer = ((fdAB + fdAB + fdBC) / 2);

	//are of T = sqrt(P(P-A)(P-B)(P-C))
	fArea = sqrtf(fPer*(fPer - fdAB)*(fPer - fdBC)*(fPer - fdAC));

	//Radius of inCircle = AreaOf T/Perimete Of T
	fRad = (fArea / fPer);

	if (!bToWrite) {
		fprintf(fptr, "fdAB = %f\nfdBC = %f\nfdAC = %f\nfPer =  %f\nfArea = %f\nfRad = %f\n", fdAB, fdBC, fdAC, fPer, fArea, fRad);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -4.0f);

	fxCord = (GLfloat)(((fdBC*fx1) + (fx2*fdAC) + (fx3*fdAB)) / (fPer * 2));
	fyCord = (GLfloat)(((fdBC*fy1) + (fy2*fdAC) + (fy3*fdAB)) / (fPer * 2));

	if (!bToWrite) {
		fprintf(fptr, "\n\nfx1 = %f\tfy1 = %f\nfx2 = %f\tfy2 = %f\nfx3 = %f\tfy3 = %f\n\nfxCoord = %f\tfyCord = %f\n", fx1, fy1, fx2, fy2, fx3, fy3, fxCord, fyCord);
	}

	glPointSize(2.0f);
	glBegin(GL_POINTS);
	glColor3f(1.0f, 1.0f, 0.0f);
	for (GLfloat i = 0; i < 2.0f*M_PI; i += 0.001f) {
		glVertex2f(fxCord + cosf(i)*fRad, fyCord + sinf(i)*fRad);
	}
	glEnd();

	//Rectangle 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -4.0f);
	
	glBegin(GL_LINE_LOOP);

	glVertex3f(fx3, fy1, 0.0f);
	glVertex3f(fx2, fy1, 0.0f);

	glVertex3f(fx2, fy2, 0.0f);
	glVertex3f(fx3, fy3, 0.0f);

	glEnd();

	//Circle
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -4.0f);
	glRotatef(fAngle, 0.0f, 1.0f, 0.0f);
	glBegin(GL_LINE_LOOP);
	for (GLfloat angle = 0.0f; angle < 2.0f*M_PI; angle += 0.001f) {
		glVertex2f(1.42f*cosf(angle), 1.42f*sinf(angle));
	}
	glEnd();

	fAngle += 1.0f;
	if (fAngle == 360.0f) {
		fAngle = 0.0f;
	}
	SwapBuffers(ghdc);
	bToWrite = true;
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

