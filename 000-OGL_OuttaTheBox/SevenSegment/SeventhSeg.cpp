#include<stdio.h>	//header for file
#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")

#define _USE_MATH_DEFINES 1
#include<math.h>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600


//Variables & Functions
bool gbFullScreen = false;
DWORD dwStyle;
HWND ghwnd=NULL;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
HDC ghdc = NULL;
HGLRC ghrc = NULL;
bool gbActivateWindow = false;
FILE *fptr = NULL;

//global Variables for display
GLfloat fR=0.0f,fG=0.0f,fB=0.0f, fA=0.0f;
int iDigitArr[] = {0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B};
int size = sizeof(iDigitArr)/sizeof(iDigitArr[0]);

void SevenSeg(int iDigShow);
void SetRGBA(int,int);

LRESULT CALLBACK MyCallBack(HWND,UINT,WPARAM,LPARAM);
void resize(int,int);
void ToggleFullScreen(void);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

	//functuions & Variables
	int initialize(void);
	void display(void);

	WNDCLASSEX wndclass;
	MSG msg;
	HWND hwnd;
	TCHAR szAppName[] = TEXT("OGL_Orth");
	bool bDone = false;
	int iRet = 0;

	if(fopen_s(&fptr,"SSLog.txt","w")!=0){
		MessageBox(NULL,TEXT("Cannot create Log File!!.."),TEXT("ErrMsg"),MB_OK);
		exit(0);
	}

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.lpfnWndProc = MyCallBack;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = CreateSolidBrush(RGB(0,0,0));
	wndclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("Seven Segment Display"),
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

	if(iRet == -1) {
		fprintf(fptr,"ChoosePixelFormat Failed !!..\n");
		DestroyWindow(hwnd);
	}
	else if(iRet == -2) {
		fprintf(fptr,"SetPixelFormat Failed !!..\n");
		DestroyWindow(hwnd);	
	}
	else if(iRet == -3) {
		fprintf(fptr,"wglCreateContext Failed !!..\n");
		DestroyWindow(hwnd);	
	}
	else if(iRet == -4) {
		fprintf(fptr,"wglMakeCurrent Failed !!..\n");
		DestroyWindow(hwnd);	
	}
	else {
		fprintf(fptr,"initialize Successful!!..\n");
	}

	ShowWindow(hwnd,iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//Msg LOOP
	while(bDone == false) {

		if(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {

			if(msg.message == WM_QUIT) {
				bDone = true;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {

			if(gbActivateWindow == true) {
				//Call To Update
			}
			display();
		}
	}

	return((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd,UINT iMsg, WPARAM wParam,LPARAM lParam) {

	//funcction & Variables
	void ToggleFullScreen(void);
	void uninitialize(void);
	void resize(int,int);

	static bool bIsMax = false;

	switch(iMsg) {

		case WM_SETFOCUS:
			gbActivateWindow = true;
			break;

		case WM_KILLFOCUS:
			gbActivateWindow = false;
			break;

		case WM_SIZE:
			resize(LOWORD(lParam),HIWORD(lParam));
			break;

		case WM_KEYDOWN:
			switch(wParam){

				case VK_ESCAPE:
					DestroyWindow(hwnd);
					break;
				case 'F':
				case 'f':
					ToggleFullScreen();
					break;

				case 'S':
				case 's':
					if(!bIsMax){
						ShowWindow(hwnd,SW_MAXIMIZE);
						bIsMax = true;
					}
					else {
						ShowWindow(hwnd,SW_SHOWNORMAL);
						bIsMax = false;
					}
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
	return(DefWindowProc(hwnd,iMsg,wParam,lParam));
}

void ToggleFullScreen(void){

	//var
	MONITORINFO mi;

	if(!gbFullScreen){

		dwStyle = GetWindowLong(ghwnd,GWL_STYLE);

		if(dwStyle & WS_OVERLAPPEDWINDOW) {
			mi = {sizeof(MONITORINFO)};

			if(GetWindowPlacement(ghwnd,&wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd,MONITORINFOF_PRIMARY),&mi)){

				SetWindowLong(ghwnd,GWL_STYLE,dwStyle & ~WS_OVERLAPPEDWINDOW);

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
		gbFullScreen = true;
	}
	else {

		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowPos(ghwnd,
			HWND_TOP,
			0,0,0,0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbFullScreen = false;
	}
}

int initialize(void){

	//param 
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;


	ghdc = GetDC(ghwnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc,&pfd);
	if(iPixelFormatIndex == 0){
		return(-1);
	}
	else {
		fprintf(fptr,"ChoosePixelFormat Successful !!..(Indx = %d)\n",iPixelFormatIndex);
	}

	if(SetPixelFormat(ghdc,iPixelFormatIndex,&pfd) == FALSE){
		return(-2);
	}
	else {
		fprintf(fptr,"SetPixelFormat Successful!!..\n");
	}

	ghrc = wglCreateContext(ghdc);
	if(ghrc == NULL){
		return(-3);
	}
	else {
		fprintf(fptr,"wglCreateContext Successful!!..\n");
	}

	if(wglMakeCurrent(ghdc,ghrc) == NULL) {
		return(-4);
	}
	else {
		fprintf(fptr,"wglMakeCurrent Successful!!..\n");
	}

	glClearColor(0.0f,0.0f,0.0f,1.0f);

	resize(WIN_WIDTH,WIN_HEIGHT);

	return(0);
}

void resize(int width, int height) {

	if(height == 0){
		height = 1;
	}

	glViewport(0,0,(GLsizei)width,(GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

    gluOrtho2D(0.0f,(GLfloat)WIN_WIDTH,0.0f,(GLfloat)WIN_HEIGHT);

}

void display(void){

static int index = 0, iChngCnt = 0;
	
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glTranslatef(400.0f,300.0f,0.0f);

	SevenSeg(iDigitArr[index]);
	if(iChngCnt == 1000) {
		index = (index + 1) % size;
		//fprintf(fptr,"%d  %d  %d\n",index,iDigitArr[index],count );
		iChngCnt = 0;
	}
	//fprintf(fptr,"%d\n",index);

	iChngCnt++;
	SwapBuffers(ghdc);

}

void SevenSeg(int iDigShow) {

	//A
	glBegin(GL_POLYGON);
	SetRGBA(iDigShow, 6);
	glColor4f(fR,fG,fB,fA);
	glVertex2f(450.0f,420.0f);
	glVertex2f(350.0f,420.0f);
	glVertex2f(350.0f,430.0f);
	glVertex2f(450.0f,430.0f);
	glEnd();

	//B
	glBegin(GL_POLYGON);
	SetRGBA(iDigShow, 5);
	glColor4f(fR,fG,fB,fA);
	glVertex2f(470.0f,310.0f);
	glVertex2f(460.0f,310.0f);
	glVertex2f(460.0f,410.0f);
	glVertex2f(470.0f,410.0f);
	glEnd();

	//C
	glBegin(GL_POLYGON);
	SetRGBA(iDigShow, 4);
	glColor4f(fR,fG,fB,fA);
	glVertex2f(470.0f,180.0f);
	glVertex2f(460.0f,180.0f);
	glVertex2f(460.0f,280.0f);
	glVertex2f(470.0f,280.0f);
	glEnd();

	//D
	glBegin(GL_POLYGON);
	SetRGBA(iDigShow, 3);
	glColor4f(fR,fG,fB,fA);
	glVertex2f(450.0f,160.0f);
	glVertex2f(350.0f,160.0f);
	glVertex2f(350.0f,170.0f);
	glVertex2f(450.0f,170.0f);
	glEnd();

	//E
	glBegin(GL_POLYGON);
	SetRGBA(iDigShow, 2);
	glColor4f(fR,fG,fB,fA);
	glVertex2f(340.0f,180.0f);
	glVertex2f(330.0f,180.0f);
	glVertex2f(330.0f,280.0f);
	glVertex2f(340.0f,280.0f);
	glEnd();
	
	//F
	glBegin(GL_POLYGON);
	SetRGBA(iDigShow, 1);
	glColor4f(fR,fG,fB,fA);
	glVertex2f(340.0f,310.0f);
	glVertex2f(330.0f,310.0f);
	glVertex2f(330.0f,410.0f);
	glVertex2f(340.0f,410.0f);
	glEnd();	


	//G
	glBegin(GL_POLYGON);
	SetRGBA(iDigShow, 0);
	glColor4f(fR,fG,fB,fA);
	glVertex2f(450.0f,290.0f);
	glVertex2f(350.0f,290.0f);
	glVertex2f(350.0f,300.0f);
	glVertex2f(450.0f,300.0f);
	glEnd();

}

void SetRGBA(int iDigShow,int iShift) {

	fR = 1.0f;
	fG = 1.0f;
	fB = 1.0f;
	fA = 0.15+1.0f * ((iDigShow >> iShift) & 1);

}

void uninitialize(void){

	if(gbFullScreen == true){

		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowPos(ghwnd,
			HWND_TOP,
			0,0,0,0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		gbFullScreen = false;

	}

	if(wglGetCurrentContext() == ghrc){
		wglMakeCurrent(NULL,NULL);
	}

	if(ghrc){
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if(ghdc){
		ReleaseDC(ghwnd,ghdc);
		ghdc = NULL;
	}

	if(fptr){
		fprintf(fptr,"\nFile Closed Successfully..\n");
		fptr = NULL;
	}
}
