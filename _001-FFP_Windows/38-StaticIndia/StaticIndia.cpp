#include<windows.h>
#include<stdio.h>
#include<gl/GL.h>
#include<gl/GLU.h>

#define _USE_MATH_DEFINES 1
#include<math.h>
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")


//Color MAcros
//Saffron 255 153 51
#define SAFFRON_R 1.0f
#define SAFFRON_G 0.600f
#define SAFFRON_B 0.20f
//White 255 255 255
#define WHITE_R 1.0f
#define WHITE_G 1.0f
#define WHITE_B 1.0f
//Green 18 136 7
#define GREEN_R 0.070f
#define GREEN_G 0.533f
#define GREEN_B 0.027f
//Blue 0 0 136
#define BLUE_R 0.0f
#define BLUE_G 0.0f
#define BLUE_B 0.533f


//global variables
HWND ghwnd = NULL;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
bool gbFullScreen = false;
DWORD dwStyle = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
bool gbActiveWindow = false;
FILE *fptr = NULL;

LRESULT CALLBACK MyCallBack(HWND,UINT,WPARAM,LPARAM);

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

    //func & 
    int initialize(void);
    void display(void);
    void ToggleFullScreen(void);

    //variables
    WNDCLASSEX wndclass;
    MSG msg;
    HWND hwnd;
    TCHAR szApp[] = TEXT("GL_PERSPECTIVE");
    bool bDone = false;
    int iRet = 0;

    if(fopen_s(&fptr,"SIndiaaLog.txt","w") != 0) {
        MessageBox(NULL,TEXT("Cannot Create Log!!.."),TEXT("ErrMsg"),MB_OKCANCEL);
        exit(0);
    }
    else {
        fprintf(fptr,"Log Created Successfully!!..\n\n");
    }

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpszClassName = szApp;
	wndclass.lpszMenuName = NULL;
	wndclass.lpfnWndProc = MyCallBack;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = CreateSolidBrush(RGB(0,0,0));
	wndclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szApp,
		TEXT("!...  INDIA  ...!"),
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

	ToggleFullScreen();
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

			if(gbActiveWindow == true) {
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
			gbActiveWindow = true;
			break;

		case WM_KILLFOCUS:
			gbActiveWindow = false;
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
    void resize(int,int);
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

	gluPerspective(45.0f,
		((GLfloat)width / (GLfloat)height),
		0.1f,
		100.0f);

}

void display(void) {

	//func
	void drawI1(void);
	void drawN(void);
	void drawD(void);
	void drawI2(void);
	void drawA(void);

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0,0,3,0,0,0,0,1,0);

    
	glLineWidth(9.0f);

	drawI1();
	drawN();
	drawD();
	drawI2();
	drawA();

    SwapBuffers(ghdc);
}

void drawI1() {

    //I
    glBegin(GL_LINES);
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(-1.5f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(-1.5f,-0.5f,0.0f);
    glEnd();


}

void drawN(){

    //N
    glBegin(GL_LINES);
    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(-1.1f,-0.5f,0.0f);

    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(-1.1f,0.5f,0.0f);

    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(-1.1f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(-0.6f,-0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(-0.6f,-0.5f,0.0f);

    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(-0.6f,0.5f,0.0f);
    glEnd();
}
void drawD(){
    //D
    glPointSize(6.4f);
	
	GLfloat G = GREEN_G, R = GREEN_R, B = GREEN_B;
	//int itr =0;
    glBegin(GL_POINTS);
    for(GLfloat angle = 3*M_PI/2; angle <= 5*M_PI/2 ; angle += 0.01f) {

		glColor3f(R,G,B);
        glVertex3f(-0.1+cosf(angle)*0.5f,sinf(angle)*0.5f,0.0f);

		//calcus for increment G = SAFFRON_G - GREEN_G /itr tya alelya avlue na increase kel only!!
		G += 0.00021f;
		if(G == SAFFRON_G){
			G = SAFFRON_G;
		}
		R += 0.0029f;
		if( R == SAFFRON_R){
			R = SAFFRON_R;
		}
		B += 0.00054f;
		if(B == SAFFRON_B) {
			B = SAFFRON_B;
		}
		//itr++;
    }

    glEnd();
	
	
	glBegin(GL_LINES);
	glVertex3f(-0.21f,0.5f,0.0f);
	glVertex3f(-0.1f,0.5f,0.0f);

    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(-0.2f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(-0.2f,-0.5f,0.0f);

	glVertex3f(-0.21f,-0.5f,0.0f);
	glVertex3f(-0.1f,-0.5f,0.0f);
    glEnd();
}

void drawI2() {

	//I
    glBegin(GL_LINES);
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(0.7f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(0.7f,-0.5f,0.0f);
    glEnd();

}

void drawA(){
	//A
	glBegin(GL_LINES);

	//MID
	//Orange Strip
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
	glVertex3f(1.15f,0.025f,0.0f);
	glVertex3f(1.45f,0.025f,0.0f);
	//WHITE Strip
	glColor3f(1.0f,1.0f,1.0f);
	glVertex3f(1.15f,0.0f,0.0f);
	glVertex3f(1.45f,0.0f,0.0f);
	//Green Strip
    glColor3f(GREEN_R,GREEN_G,GREEN_B);
	glVertex3f(1.15f,-0.02f,0.0f);
	glVertex3f(1.45f,-0.02f,0.0f);

	//1st inCline
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
	glVertex3f(1.3f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
	glVertex3f(1.0f,-0.5f,0.0f);

	//2nd One
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
	glVertex3f(1.3f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
	glVertex3f(1.6f,-0.5f,0.0f);
	glEnd();

	glBegin(GL_POINTS);
	glColor3f(BLUE_R,BLUE_G,BLUE_B);
	glVertex3f(1.3f,0.0f,0.0f);
	glEnd();
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
