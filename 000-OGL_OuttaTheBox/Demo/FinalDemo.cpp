
//header files
#include<Windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include"MyHeader.h"
#include"stdio.h"


#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"user32.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//global variables
BOOL gbActiveWindow = false;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

//texture: stone 
GLuint texture_wood = 0;
GLuint texture_ball1 = 0;
GLUquadric *quadric_sphere=NULL;

//variables
float cam = 3.5f;
float angle_cube = 2.0f;
float sphere = 1.5f;
float sphere_down = 0.6f;
bool flag = false;


//fullscreen
HWND ghwnd = NULL;
bool gbFullscreen = false;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
DWORD dwStyle;

FILE *gpFile = NULL;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmd, int iCmdShow)
{

	void display(void);
	int initialize(void);
	void ToggleFullscreen();

	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR str[255] = TEXT("My App");

	//variables
	BOOL gbDone = false;
	int iRet = 0;

	//Log File 
	if (fopen_s(&gpFile, "log.txt", "w") != 0) {
		MessageBox(NULL, TEXT("Log File Cannot be created\n"), TEXT("ACK"), MB_OK);
		exit(0);
	}
	else {
		fprintf(gpFile, "Log file successfully created\n");
	}

	//initialize of WndclassEx variables
	wndclass.cbSize = sizeof(wndclass);
	//CS_OWNDC= To tell the OS that not to discard him
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = NULL;
	wndclass.cbWndExtra = NULL;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = str;
	wndclass.lpszMenuName = NULL;

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		str, TEXT("TRIANGLE_WITH_TEXTURE"),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		100, 100, WIN_WIDTH, WIN_HEIGHT,
		NULL, NULL, hInstance, NULL);

	ghwnd = hwnd;
	iRet = initialize();
	if (iRet == -1) {
		fprintf(gpFile, "CreatePixelFormat() failed\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -2) {
		fprintf(gpFile, "SetPixelFormat() failed\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -3)
	{
		fprintf(gpFile, "wglCreateContext() failed\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -4) {
		fprintf(gpFile, "wglMakeCurrent() failed\n");
		DestroyWindow(hwnd);
	}
	else {
		fprintf(gpFile, "Success in initialization\n");
	}

	ShowWindow(hwnd, WS_MAXIMIZE);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);
	fprintf(gpFile, "Toggle function\n");
	ToggleFullscreen();

	//game loop
	while (gbDone == false) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (WM_QUIT == msg.message) {
				gbDone = true;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {
			if (gbActiveWindow == false) {
				//here call update
			}
			display();
		}
	}
	return ((int)msg.wParam);

}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

	void ToggleFullscreen(void);
	void resize(int, int);
	void unInitialize(void);

	//code
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

	case WM_KEYDOWN:
		switch (wParam) {
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
		case 0x46:
			
			break;
		case VK_SPACE:
			flag = true;
			break;
		}
		break;

	case WM_ERASEBKGND:
		return (0);

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		unInitialize();
		PostQuitMessage(0);
		break;
	}

	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

int initialize(void)
{
	void resize(int, int);
	BOOL loadTexture(GLuint*, TCHAR[]);

	int iPixelFormatIndex = 0;
	PIXELFORMATDESCRIPTOR pfd;

	memset((void *)&pfd, NULL, sizeof(PIXELFORMATDESCRIPTOR));
	//initialize variables of pixelformatdescriptor
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	ghdc = GetDC(ghwnd);
	//tyachya kade multiple pixelformat astat.. to jo match hoil tyacha index return karto
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0) {
		return (-1);
	}
	else {
		fprintf(gpFile, "ChoosePixelFormat function\n");
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		return (-2);
	}
	else {
		fprintf(gpFile, "SetPixelFormat function\n");
	}

	ghrc = wglCreateContext(ghdc);

	if (ghrc == NULL) {
		return (-3);
	}
	else {
		fprintf(gpFile, "WglCreateContext function\n");
	}

	if (wglMakeCurrent(ghdc, ghrc) == FALSE) {
		return (-4);
	}
	else {
		fprintf(gpFile, "wglMakeCurrent function");
	}

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//test for enabling texture
	glEnable(GL_TEXTURE_2D);

	//call loadTexture
	if (loadTexture(&texture_wood, MAKEINTRESOURCE(ID_WOOD))== FALSE) {
		return -5;
	}
	else {
		fprintf(gpFile,"Successfully loaded texture :wood\n");
	}
	

	if (loadTexture(&texture_ball1, MAKEINTRESOURCE(ID_BALL1)) == FALSE) {
		return(-6);
	}
	else {
		fprintf(gpFile, "Successfully loaded texture :Ball\n");
	}
	quadric_sphere = gluNewQuadric();
	gluQuadricTexture(quadric_sphere, GL_TRUE);

	resize(WIN_WIDTH, WIN_HEIGHT);


	return 0;
}

void ToggleFullscreen(void)
{
	MONITORINFO mi;

	if (gbFullscreen == false) {
		//style gya
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		//style aahe ki nahi check kara
		if (dwStyle&WS_OVERLAPPEDWINDOW) {
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghwnd, &wpPrev) &&
				GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				//WS_OVERLAPPEDWINDOW he style kadun takaychi
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
		gbFullscreen = true;

	}
	else {
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle&WS_OVERLAPPEDWINDOW);
		//purvi hoti ti set kar style
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbFullscreen = false;
	}
}

void resize(int width, int height)
{
	if (height == 0)
		height = 1;

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 1.0f, 100.0f);

}

BOOL loadTexture(GLuint *texture, TCHAR ImageResourceId[]) {

	//HI CHUK HOTI NALAYK MANSA!!! Ithe Tu Literally texture_wood la zero na assign kartoy ani expect kartoys ki changes
	//whave!!
	//texture_wood = *texture;

	HBITMAP hBitmap = NULL;
	BITMAP bmp;
	BOOL bStatus = FALSE;

	//code
	//HANDLE LoadImage(HMODULE, LPCTSTR, UINT, int, int,UINT);
	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL),
		ImageResourceId, IMAGE_BITMAP,
		0, 0, LR_CREATEDIBSECTION);

	if (hBitmap) {
		bStatus = TRUE;
		//Bitmap cha data milavna
		GetObject(hBitmap, sizeof(bmp), &bmp);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		glGenTextures(1, texture);

		glBindTexture(GL_TEXTURE_2D, *texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_TEXTURE_WRAP_S);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_TEXTURE_WRAP_T);

		//sphere texture
/*		glBindTexture(GL_TEXTURE_2D, *texture);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);*/
		
	
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight,
			GL_BGR_EXT, GL_UNSIGNED_BYTE, bmp.bmBits);

		DeleteObject(hBitmap);

	}
	return (bStatus);
}


void display()
{

	void update(float *);
	void Board();
	void Ball();
	void Wall();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	
	
	/*glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(3.0f, 0.0f, 0.0f);
	glVertex3f(-3.0f, 0.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 2.0f, 0.0f);
	glVertex3f(0.0f, -2.0f, 0.0f);
	glEnd();

	//glRotatef(angle_cube, 0.0f, 1.0f, 0.0f);
	*/
	//Board
	glLoadIdentity();
	glTranslatef(0.0f, -0.6f, -cam);
	Board();//middle

	//left
	glLoadIdentity();
	glTranslatef(-2.1f, -0.6f, -cam);
	Board();

	//right
	glLoadIdentity();
	glTranslatef(2.1f, -0.6f, -cam);
	Board();

	//Sphere
	Ball();

	//Wall
	glLoadIdentity();
	glTranslatef(0.0f, -0.0f, -5.0f);
	//Wall();

	SwapBuffers(ghdc);
	update(&angle_cube);

}

void Wall() {

	//back wall
	glBegin(GL_POLYGON);
	glVertex3f(4.7f, 2.3f, -11.0f);//top -r
	glVertex3f(-4.7f, 2.3f, -11.0f);//top -l
	glVertex3f(-4.7f, -1.5f, -11.0f);//bottom -l
	glVertex3f(4.7f, -1.5f, -11.0f);//bottom -r
	glEnd();
	
	//right
	glBegin(GL_POLYGON);
	glVertex3f(-4.4f, 2.3f, -11.0f);//top -r
	glVertex3f(-4.4f, 2.3f, 0.0f);//top -l
	glVertex3f(-4.4f, -1.5f, 0.0f);//top -r
	glVertex3f(-4.4f, -1.5f, -11.0f);//top -r
	glEnd();

	//left
	glBegin(GL_POLYGON);
	glVertex3f(4.4f, 2.3f, -11.0f);//top -r
	glVertex3f(4.4f, 2.3f, 0.0f);//top -l
	glVertex3f(4.4f, -1.5f, 0.0f);//top -r
	glVertex3f(4.4f, -1.5f, -11.0f);//top -r
	glEnd();

	//down
	glBegin(GL_POLYGON);
	glVertex3f(4.4f, -1.52f, -11.0f);//bottom -r
	glVertex3f(-4.4f, -1.52f, -11.0f);//bottom -l
	glVertex3f(-4.4f, -1.91f, 0.0f);//top -r
	glVertex3f(4.4f, -1.91f, 0.0f);//top -r
	glEnd();

}

void Board() {
	
	glBindTexture(GL_TEXTURE_2D, texture_wood);
	//front
	glBegin(GL_POLYGON);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(0.9f, -0.3f, 0.0f);//top -r
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-0.9f, -0.3f, 0.0f);//top -l
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-0.9f, -0.5f, 0.0f);//bottom -l
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(0.9f, -0.5f, 0.0f);//bottom -r
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture_wood);
	//right
	glBegin(GL_POLYGON);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(0.9f, -0.3f, -12.0f);//top -r
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(0.9f, -0.3f, 0.0f);//top -l
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(0.9f, -0.5f, 0.0f);//bottom -l
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(0.9f, -0.5f, -12.0f);//bottom -r
	glEnd();
	glBindTexture(GL_TEXTURE_2D, texture_wood);
	//left
	glBegin(GL_POLYGON);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-0.9f, -0.3f, -12.0f);//top -r
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-0.9f, -0.3f, 0.0f);//top -l
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-0.9f, -0.5f, 0.0f);//bottom -l
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-0.9f, -0.5f, -12.0f);//bottom -r
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture_wood);
	//back
	glBegin(GL_POLYGON);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(0.9f, -0.3f, -12.0f);//top -r
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-0.9f, -0.3f, -12.0f);//top -l
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-0.9f, -0.5f, -12.0f);//bottom -l
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(0.9f, -0.5f, -12.0f);//bottom -r
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture_wood);
	//top
	glBegin(GL_POLYGON);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(0.9f, -0.3f, -12.0f);//top -r
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-0.9f, -0.3f, -12.0f);//top -l
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-0.9f, -0.3f, 0.0f);//bottom -l
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(0.9f, -0.3f, 0.0f);//bottom -r
	glEnd();
	
	if (flag == true) {
		if (cam >= 0.2f) {
			cam = cam - 0.009f;
		}
	}
	
}

void Ball() {

	static float angle_sphere = 0.0f;
	void sphere_rotate();
	glLoadIdentity();
	//glRotatef(angle_sphere, 0.0f, 1.0f, 1.0f);
	
	if (flag == true) {
		sphere_rotate();
	}
	
	glBindTexture(GL_TEXTURE_2D, texture_ball1);
	gluSphere(quadric_sphere, 0.1f, 50, 50);
	
	angle_sphere = angle_sphere + 2.0f;
	if (angle_sphere >= 360.0f) {
		angle_sphere = 0.0f;
	}
}


void sphere_rotate() {
	if (sphere <= 4.0f) {
		//ball moving straight
		glTranslatef(0.0f, -0.2f, -sphere);
		sphere = sphere + 0.009f;
		fprintf(gpFile, "%f\n", sphere);
	}
	else {
		//ball falling down
		glTranslatef(0.0f, -sphere_down, -12.0f);
		if (sphere >= 4.0f  && sphere_down <= 1.9f) {
			sphere_down = sphere_down + 0.06f;
		}
		fprintf(gpFile, "sphere_down=%f\n", sphere_down);
	}
}

void update(float *ptr) {
	*ptr = *ptr + 0.9f;
	if (*ptr >= 360.0f) {
		*ptr = 0.0f;
	}
}

void unInitialize(void)
{
	//fullscreen ahe mi nahi check kara ani jar asel tr restore to normal size
	if (gbFullscreen == true) {
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle& ~WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}

	if (wglGetCurrentContext() == ghrc) {
		wglMakeCurrent(NULL, NULL);
	}

	if (ghrc == NULL) {
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if (ghdc == NULL) {
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (gpFile) {
		fprintf(gpFile, "Log file is closed\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}


