#include<stdio.h>
#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>


#include"ResHeader.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600


#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

//Global vars
bool gbIsFullScreen = false;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
DWORD dwStyle = NULL;
bool gbActivateWindow = false;
FILE *fptr = NULL;
GLfloat fRPyAngle = 0.0f;
GLfloat fRCuAngle = 0.0f;

//Texture Var
GLuint iTexKundali;
GLuint iTexStone;

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

	int initialize(void);
	void display(void);
	void update(void);

	WNDCLASSEX wndclass;
	HWND hwnd;
	TCHAR szApp[] = TEXT("GL_PERSP");
	MSG msg;
	bool bDone = false;
	int iRet = 0;
	if (fopen_s(&fptr, "_TexCubeLog.txt", "w") != 0) {

		MessageBox(NULL, TEXT("Cannot Create Log!!.."), TEXT("ErrMsg"), MB_OKCANCEL);
		exit(0);
	}
	else {
		fprintf(fptr, "Lof Created SuccessFully!!..\n\n");
	}


	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpszClassName = szApp;
	wndclass.lpszMenuName = NULL;
	wndclass.lpfnWndProc = MyCallBack;
	wndclass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szApp,
		TEXT("Textured Cube"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
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
		fprintf(fptr, "ChoosePixelFormatFailed!!..\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -2) {
		fprintf(fptr, "SetPixelFormatFailed!!..\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -3) {
		fprintf(fptr, "wglCreateContextFailed!!..\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -4) {
		fprintf(fptr, "wglMakeCurrentFailed!!..\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -5) {
		fprintf(fptr, "loadTextureFailed!!..\n");
		DestroyWindow(hwnd);
	}
	else {
		fprintf(fptr, "Initialization Successful!!..\n");
	}

	ShowWindow(hwnd, iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	while (!bDone) {

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

			if (gbActivateWindow) {
				display();
				update();
			}
		}
	}

	return((int)msg.wParam);
}


LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

	void uninitialize(void);
	void resize(int, int);
	void ToggleFullScreen(void);

	static bool bIsMax = false;

	switch (iMsg) {

	case WM_SETFOCUS:
		gbActivateWindow = true;
		break;

	case WM_KILLFOCUS:
		gbActivateWindow = false;
		break;

	case WM_ERASEBKGND:
		return(0);

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_KEYDOWN:
		switch (wParam) {
		case VK_ESCAPE:
			DestroyWindow(hwnd);
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

		case 'f':
		case 'F':
			ToggleFullScreen();
			break;
		}
		break;

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


void ToggleFullScreen(void) {

	//Var
	MONITORINFO mi;

	if (!gbIsFullScreen) {

		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW) {

			mi = { sizeof(MONITORINFO) };

			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi)) {

				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

				SetWindowPos(ghwnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_FRAMECHANGED | SWP_NOZORDER);
			}
		}
		ShowCursor(FALSE);
		gbIsFullScreen = true;
	}
	else {

		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowPos(ghwnd,
			HWND_TOP,
			0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbIsFullScreen = false;
	}
}
int initialize(void) {

	void resize(int, int);
	BOOL loadTexture(GLuint*, TCHAR[]);

	//var
	int iPixelFormatIndex = 0;
	PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR) };

	memset((void*)&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cBlueBits = 8;
	pfd.cGreenBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	ghdc = GetDC(ghwnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0) {
		return(-1);
	}
	else {
		fprintf(fptr, "ChoosePixelFormat Successful !!..(Indx = %d)\n", iPixelFormatIndex);
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) {
		return(-2);
	}
	else {
		fprintf(fptr, "SetPixelFormat Successful!!..\n");
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL) {
		return(-3);
	}
	else {
		fprintf(fptr, "wglCreateContext Successful!!..\n");
	}

	if (wglMakeCurrent(ghdc, ghrc) == FALSE) {
		return(-4);
	}
	else {
		fprintf(fptr, "wglMakeCurrent Successful!!..\n");
	}

	glShadeModel(GL_SMOOTH);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_TEXTURE_2D);

	if (loadTexture(&iTexKundali, MAKEINTRESOURCE(ID_KUNDALI)) == FALSE) {
		return(-5);
	}
	else {
		fprintf(fptr, "Texture Kundali Loaded Successfully!!..=\n");
	}

	if (loadTexture(&iTexStone, MAKEINTRESOURCE(ID_STONE)) == FALSE) {
		return(-5);
	}
	else {
		fprintf(fptr, "Texture Pyramid Loaded Successfully!!..=\n");
	}

	resize(WIN_WIDTH, WIN_HEIGHT);
	return(0);

}

BOOL loadTexture(GLuint *texture, TCHAR imageResourceID[]) {

	//Var Decl
	HBITMAP hBitmap = NULL;
	BITMAP bmp;
	BOOL bStatus = FALSE;

	//Code

	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL),
		imageResourceID,
		IMAGE_BITMAP,
		0, 0,
		LR_CREATEDIBSECTION);


	if (hBitmap) {
		bStatus = TRUE;

		GetObject(hBitmap, sizeof(BITMAP), &bmp);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		glGenTextures(1, texture);

		glBindTexture(GL_TEXTURE_2D, *texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


		gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
			bmp.bmWidth, bmp.bmHeight,
			GL_BGR_EXT, GL_UNSIGNED_BYTE,
			bmp.bmBits);

		DeleteObject(hBitmap);
	}

	return(bStatus);
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

	void drawCube(void);
	void drawPyramid(void);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(1.3f, 0.0f, -5.0f);
	glScalef(0.8f, 0.8f, 0.8f);
	glRotatef(fRCuAngle, 1.0f, 1.0f, 1.0f);

	drawCube();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.3f, 0.0f, -5.0f);
	glRotatef(fRCuAngle, 0.0f, 1.0f, 0.0f);

	drawPyramid();

	SwapBuffers(ghdc);

}

void drawCube(void) {

	glBindTexture(GL_TEXTURE_2D, iTexKundali);

	glBegin(GL_QUADS);

	//Top Face
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);

	//Bottom Face
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	//Front face
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	//Back Face
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	//Right Face
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.01, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	//Left Face
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	glEnd();
}


void drawPyramid(void) {

	glBindTexture(GL_TEXTURE_2D, iTexStone);

	glBegin(GL_TRIANGLES);

	//Frontal Face
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);	// Appex
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);	//Left
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);	//Right

	//Right Face
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);	//Appex
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);	//Left
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);	//Right

	//Back Face
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);		//Appex
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);		//Left
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);	//Right

	//Left Face
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);	//Appex
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);	//Left
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);	//Right

	glEnd();
}


void update(void) {

	fRCuAngle += 0.009f;
	if (fRCuAngle >= 360.0f) {
		fRCuAngle = 0.0f;
	}

	fRPyAngle += 0.009f;
	if (fRPyAngle >= 360.0f) {
		fRPyAngle = 0.0f;
	}
}

void uninitialize(void) {

	if (gbIsFullScreen == true) {

		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowPos(ghwnd,
			HWND_TOP,
			0, 0, 0, 0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbIsFullScreen = false;

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

	//glDeleteTextures(1, iTexStone);

	if (fptr) {
		fprintf(fptr, "\nFile Closed Successfully..\n");
		fptr = NULL;
	}

}



