#include<windows.h>
#include<stdio.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include"ResHead.h"

#define _USE_MATH_DEFINES 1
#include<math.h>
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"winmm.lib")

//Color MAcros
//Saffron 255 153 51
#define SAFFRON_R 1.0f
#define SAFFRON_G 0.600f
#define SAFFRON_B 0.196f
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

//Animation Parameter
bool bI1Reached = false, bNReached = false, bDReached = false,bI2Reached = false, bAReached = false, bPlaneReached = false;
static float fTransSpeed = 0.0003f;

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
    TCHAR szApp[] = TEXT("DI_PERS");
    bool bDone = false;
    int iRet = 0;

    if(fopen_s(&fptr,"DILog.txt","w") != 0) {
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
	wndclass.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(MyIcon));
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MyIcon));
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
	void drawRefLines(void);
	void drawPlane1(void);
	void drawPlane2(void);
	void drawPlane3(void);

	static bool bPlaySong = false;


    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    gluLookAt(0,0,3,0,0,0,0,1,0);

	
	glLineWidth(9.0f);
	if (!bPlaySong) {
		PlaySound(MAKEINTRESOURCE(MyWav), NULL, SND_ASYNC | SND_NODEFAULT | SND_RESOURCE);
	}
	drawI1();
	
	if(bAReached == true) {
		drawN();
	}

	if(bI2Reached == true) {
		drawD();
	}

	if(bNReached ==true) {
		drawI2();
	}

	if(bI1Reached == true) {
		drawA();
	}

	if(bDReached == true) {
		//drawRefLines();

		drawPlane1();
		drawPlane2();
		drawPlane3();
	}
	

    SwapBuffers(ghdc);
	bPlaySong = true;
}

void drawI1() {

	static GLfloat bXTrans = -0.8f;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glTranslatef(bXTrans,0.0f,-3.0f);
    glBegin(GL_LINES);
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(-1.5f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(-1.5f,-0.5f,0.0f);
    glEnd();

	bXTrans += fTransSpeed;
	//fprintf(fptr,"Val = %f\n",bXTrans);
	if(bXTrans >= 0.0f){
		bXTrans = 0.00f;
		bI1Reached = true;
	}

}

void drawN(){

	static GLfloat bYTrans = 2.0f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f,bYTrans,-3.0f);

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

	bYTrans -= fTransSpeed;
	if(bYTrans <= 0.0f) {
		bYTrans = 0.0f;
		bNReached = true;
	}
	
}
	
void drawD(){
    //D
    glPointSize(6.4f);
 
	
	GLfloat G = GREEN_G, R = GREEN_R, B = GREEN_B;
	static GLfloat A = 0.0f;
	//static int itr =0;
    glBegin(GL_POINTS);
    for(GLfloat angle = 3*M_PI/2; angle <= 5*M_PI/2 ; angle += 0.01f) {

		glColor4f(R,G,B,A);
        glVertex3f(-0.1+cosf(angle)*0.5f,sinf(angle)*0.5f,0.0f);

		//calcus for increment G = SAFFRON_G - GREEN_G /itr tya alelya avlue na increase kel only!!
		G += 0.00021f;
		R += 0.0029f;
		B += 0.00054f;

    }

    glEnd();
	

	static GLfloat Sr=0.0f, Sg = 0.0f, Sb = 0.0f;
	static GLfloat Gr=0.0f, Gg = 0.0f, Gb = 0.0f;
	static int iCntStat = 0;

	glBegin(GL_LINES);

 	glColor3f(Sr,Sg,Sb);
	glVertex3f(-0.21f,0.5f,0.0f);
	glVertex3f(-0.1f,0.5f,0.0f);

    glVertex3f(-0.2f,0.5f,0.0f);

    glColor3f(Gr,Gg,Gb);
    glVertex3f(-0.2f,-0.5f,0.0f);

	glVertex3f(-0.21f,-0.5f,0.0f);
	glVertex3f(-0.1f,-0.5f,0.0f);

    glEnd();

	if(iCntStat >= 10) {
	//fprintf(fptr,"Here Val SR =%d\n",iCntStat);

		Sr += 0.001f;
		Sg += 0.0006f;
		Sb += 0.0002f; 
		if(Sr >= SAFFRON_R) {
			//fprintf(fptr,"Here Val itr =%d\n",itr);
			bDReached = true;
			Sr = SAFFRON_R;
		}
		if(Sg >= SAFFRON_G) {
			Sg = SAFFRON_G;
		}
		if(Sb >= SAFFRON_B) {
			Sb = SAFFRON_B;
		}

		Gr += 0.00007f;
		Gg += 0.0006f;
		Gb += 0.000027f;

		if(Gr >= GREEN_R){
			Gr = GREEN_R;
		}
		if(Gg >= GREEN_G){
			Gg = GREEN_G;
		}
		if(Gb >= GREEN_B){
			Gb = GREEN_B;
		}
		
		A += 0.00057f;
		if(A >= 1.0f){
			A = 1.0f;
		}
		iCntStat =0;
	}
	else {
		iCntStat++;
	}
}

void drawI2() {

	static GLfloat bYTrans = -2.0f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f,bYTrans,-3.0f);

	//I
    glBegin(GL_LINES);
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(0.7f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(0.7f,-0.5f,0.0f);
    glEnd();

	bYTrans += fTransSpeed;
	if(bYTrans >= 0.0f) {
		bYTrans = 0.0f;
		bI2Reached = true;
	}

}

void drawA(){

	static GLfloat bXTrans = 0.8f, Alpaha = 0.0f;
	static int inCnt = 0;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glTranslatef(bXTrans,0.0f,-3.0f);	

	if(bPlaneReached == true) {
		glBegin(GL_LINES);
		glColor4f(SAFFRON_R,SAFFRON_G,SAFFRON_B,Alpaha);
		glVertex3f(1.16f,0.020f,0.0f);
		glVertex3f(1.44f,0.020f,0.0f);

		glColor4f(1.0f,1.0f,1.0f,Alpaha);
		glVertex3f(1.15f,0.0f,0.0f);
		glVertex3f(1.45f,0.0f,0.0f);

		glColor4f(GREEN_R,GREEN_G,GREEN_B,Alpaha);
		glVertex3f(1.15f,-0.020f,0.0f);	
		glVertex3f(1.45f,-0.020f,0.0f);

		glEnd();
	}	
	//A
	glBegin(GL_LINES);

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


	bXTrans -= fTransSpeed;
	if(bXTrans<= 0.0f){
		bXTrans = 0.0f;
		bAReached = true;
	}
	if(bPlaneReached == true) {
		if(inCnt == 10) {
			Alpaha += 0.0004f;
			if(Alpaha >= 1.0f) {
				Alpaha = 1.0f;
			}
			inCnt =0;
		}
		inCnt++;
	}
	
}

void drawRefLines(void){

	glLineWidth(1.0f);
	glBegin(GL_LINES);
	//X-Axis 1.10
	glColor3f(1.0f,0.0f,0.0f);
	glVertex3f(2.21f,0.0f,0.0f);
	glVertex3f(-2.21f,0.0f,0.0f);

	//Y-Axis 0.625
	glColor3f(0.0f,0.0f,1.0f);
	glVertex3f(0.0f,1.25f,0.0f);
	glVertex3f(0.0f,-1.25f,0.0f);
	glEnd();

	glPointSize(1.0f);
	glBegin(GL_POINTS);
	//3rd Quad
	for(GLfloat angle = M_PI;angle >= M_PI/2;angle -= 0.001f) {
		glVertex3f(-0.95f+cosf(angle)*1.25f,-1.25+sinf(angle)*1.25f,0.0f);

	}
	//2nd Quad
	for(GLfloat angle = M_PI;angle <= 3*M_PI/2;angle += 0.001f) {
		glVertex3f(-0.95+cosf(angle)*1.25f,1.25+sinf(angle)*1.25f,0.0f);

	}
	//4th Quad
	for(GLfloat angle = M_PI/2;angle >= 0;angle -= 0.001f) {
		glVertex3f(1.6f+cosf(angle)*1.25f,-1.25+sinf(angle)*1.25f,0.0f);

	}	
	//1th Quad
	for(GLfloat angle = 3*M_PI/2;angle <= 2*M_PI;angle += 0.001f) {
		glVertex3f(1.6f+cosf(angle)*1.25f,1.25+sinf(angle)*1.25f,0.0f);

	}

	glEnd();

}

void drawPlane1(void) {

	void drawPlane2(void);

	static GLfloat fxTrans=-4.0f;

	//drawPlane2();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(fxTrans,0.0f,-5.0f);

	glBegin(GL_TRIANGLES);
	//Powder Blue 186 226  238
	glColor3f(0.729f,0.886f,0.933f);
	glVertex3f(-0.02f,0.0f,0.0f);
	glVertex3f(-0.3f,0.07f,0.0f);
	glVertex3f(-0.3f,-0.07f,0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-0.3f,0.07f,0.0f);
	glVertex3f(-0.6f,0.09f,0.0f);
	glVertex3f(-0.9f,0.06f,0.0f);
	glVertex3f(-0.9f,-0.06f,0.0f);
	glVertex3f(-0.6f,-0.09f,0.0f);
	glVertex3f(-0.3f,-0.07f,0.0f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(-0.50,0.08,0.0f);
	glVertex3f(-0.7f,0.3f,0.0f);
	glVertex3f(-0.7f,0.075f,0.0f);

	glVertex3f(-0.50,-0.08,0.0f);
	glVertex3f(-0.7f,-0.3f,0.0f);
	glVertex3f(-0.7f,-0.075f,0.0f);

	glVertex3f(-0.8f,0.068f,0.0f);
	glVertex3f(-0.9f,0.2f,0.0f);
	glVertex3f(-0.9f,0.06f,0.0f);

	glVertex3f(-0.8f,-0.068f,0.0f);
	glVertex3f(-0.9f,-0.2f,0.0f);
	glVertex3f(-0.9f,-0.06f,0.0f);
	glEnd();

	glLineWidth(1.5f);
	glBegin(GL_LINES);
	glColor3f(0.0f,0.0f,0.0f);
	//I
	glVertex3f(-0.64f,0.05f,0.0f);
	glVertex3f(-0.64f,-0.05f,0.0f);

	//A
	glVertex3f(-0.61f,0.05f,0.0f);		
	glVertex3f(-0.63f,-0.05f,0.0f);

	glVertex3f(-0.62f,0.0f,0.0f);
	glVertex3f(-0.60,0.0f,0.0f);

	glVertex3f(-0.61f,0.05f,0.0f);	
	glVertex3f(-0.59f,-0.05f,0.0f);

	//F	
	glVertex3f(-0.58f,0.05f,0.0f);
	glVertex3f(-0.58f,-0.05f,0.0f);
	
	glVertex3f(-0.58f,0.05f,0.0f);
	glVertex3f(-0.55f,0.05f,0.0f);

	glVertex3f(-0.58f,0.01f,0.0f);
	glVertex3f(-0.56f,0.01f,0.0f);

	glEnd();

	glLineWidth(9.0f);
	glBegin(GL_LINES);
	glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
	glVertex3f(-0.9f,0.040f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3f(-1.3f,0.040f,0.0f);

	glColor3f(1.0f,1.0f,1.0f);
	glVertex3f(-0.9f,0.0f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3f(-1.3f,0.0f,0.0f);

	glColor3f(GREEN_R,GREEN_G,GREEN_B);
	glVertex3f(-0.9f,-0.040f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);	
	glVertex3f(-1.3f,-0.040f,0.0f);

	glEnd();


	fxTrans += 0.0001f;


}

void drawPlane2(void) {

	static GLfloat fxTrans=-4.0f,fyTrans = 2.3f, angle = -90.0f, fIxTrans = 3.8f, fIyTrans =0.0f, Iangle = 0.0f ;
	static bool bInvTrans = false;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(!bInvTrans) {
		glTranslatef(fxTrans, fyTrans,-5.0f);
		glRotatef(angle,0.0f,0.0f,1.0f);
	}
	else {
		glTranslatef(fIxTrans, fIyTrans,-5.0f);
		glRotatef(Iangle,0.0f,0.0f,1.0f);
	}

	glBegin(GL_TRIANGLES);
	//Powder Blue 186 226  238
	glColor3f(0.729f,0.886f,0.933f);
	glVertex3f(-0.02f,0.0f,0.0f);
	glVertex3f(-0.3f,0.07f,0.0f);
	glVertex3f(-0.3f,-0.07f,0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-0.3f,0.07f,0.0f);
	glVertex3f(-0.6f,0.09f,0.0f);
	glVertex3f(-0.9f,0.06f,0.0f);
	glVertex3f(-0.9f,-0.06f,0.0f);
	glVertex3f(-0.6f,-0.09f,0.0f);
	glVertex3f(-0.3f,-0.07f,0.0f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(-0.50,0.08,0.0f);
	glVertex3f(-0.7f,0.3f,0.0f);
	glVertex3f(-0.7f,0.075f,0.0f);

	glVertex3f(-0.50,-0.08,0.0f);
	glVertex3f(-0.7f,-0.3f,0.0f);
	glVertex3f(-0.7f,-0.075f,0.0f);

	glVertex3f(-0.8f,0.068f,0.0f);
	glVertex3f(-0.9f,0.2f,0.0f);
	glVertex3f(-0.9f,0.06f,0.0f);

	glVertex3f(-0.8f,-0.068f,0.0f);
	glVertex3f(-0.9f,-0.2f,0.0f);
	glVertex3f(-0.9f,-0.06f,0.0f);
	glEnd();

	glLineWidth(1.5f);
	glBegin(GL_LINES);
	glColor3f(0.0f,0.0f,0.0f);
	//I
	glVertex3f(-0.64f,0.05f,0.0f);
	glVertex3f(-0.64f,-0.05f,0.0f);

	//A
	glVertex3f(-0.61f,0.05f,0.0f);		
	glVertex3f(-0.63f,-0.05f,0.0f);

	glVertex3f(-0.62f,0.0f,0.0f);
	glVertex3f(-0.60,0.0f,0.0f);

	glVertex3f(-0.61f,0.05f,0.0f);	
	glVertex3f(-0.59f,-0.05f,0.0f);

	//F	
	glVertex3f(-0.58f,0.05f,0.0f);
	glVertex3f(-0.58f,-0.05f,0.0f);
	
	glVertex3f(-0.58f,0.05f,0.0f);
	glVertex3f(-0.55f,0.05f,0.0f);

	glVertex3f(-0.58f,0.01f,0.0f);
	glVertex3f(-0.56f,0.01f,0.0f);

	glEnd();

	glLineWidth(9.0f);
	glBegin(GL_LINES);
	glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
	glVertex3f(-0.9f,0.040f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3f(-1.3f,0.040f,0.0f);

	glColor3f(1.0f,1.0f,1.0f);
	glVertex3f(-0.9f,0.0f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3f(-1.3f,0.0f,0.0f);

	glColor3f(GREEN_R,GREEN_G,GREEN_B);
	glVertex3f(-0.9f,-0.040f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);	
	glVertex3f(-1.3f,-0.040f,0.0f);

	glEnd();

	if(!bInvTrans) {
		fxTrans += 0.0001f;
		fyTrans -= 0.0001f;
		if(fxTrans >= 3.0f){
			bPlaneReached = true;
		}
		if(fxTrans >= 3.8f){
			fxTrans = 3.8f;
			bInvTrans = true;
		}
		if(fyTrans <= 0.0f){
			fyTrans = 0.0f;
		}
		angle += 0.004f;
		if(angle >= 0.0f){
			angle = 0.0f;
		}
	}
	else {
		fIxTrans += 0.0001f;
		fIyTrans -= 0.0001f;

		Iangle -= 0.004f;

	}


}

void drawPlane3(void) {

	static GLfloat fxTrans=-4.0f,fyTrans = -2.3f, angle = 90.0f, fIxTrans = 3.8f, fIyTrans =0.0f, Iangle = 0.0f ;
	static bool bInvTrans = false;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glTranslatef(-3.5f,1.8f,-5.0f);
	//glTranslatef(0.0f,0.0f,-5.0f);
	if(!bInvTrans) {
		glTranslatef(fxTrans, fyTrans,-5.0f);
		glRotatef(angle,0.0f,0.0f,1.0f);
	}
	else {
		glTranslatef(fIxTrans, fIyTrans,-5.0f);
		glRotatef(Iangle,0.0f,0.0f,1.0f);
	}	
	//glTranslatef(-1.5f+cosf(angle)*1.25f,1.50f+sinf(angle)*1.25f,-5.0f);
	// P-Y,p-X
	//glTranslatef(-1.8f, -2.64f,-5.0f);

	glBegin(GL_TRIANGLES);
	//Powder Blue 186 226  238
	glColor3f(0.729f,0.886f,0.933f);
	glVertex3f(-0.02f,0.0f,0.0f);
	glVertex3f(-0.3f,0.07f,0.0f);
	glVertex3f(-0.3f,-0.07f,0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-0.3f,0.07f,0.0f);
	glVertex3f(-0.6f,0.09f,0.0f);
	glVertex3f(-0.9f,0.06f,0.0f);
	glVertex3f(-0.9f,-0.06f,0.0f);
	glVertex3f(-0.6f,-0.09f,0.0f);
	glVertex3f(-0.3f,-0.07f,0.0f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(-0.50,0.08,0.0f);
	glVertex3f(-0.7f,0.3f,0.0f);
	glVertex3f(-0.7f,0.075f,0.0f);

	glVertex3f(-0.50,-0.08,0.0f);
	glVertex3f(-0.7f,-0.3f,0.0f);
	glVertex3f(-0.7f,-0.075f,0.0f);

	glVertex3f(-0.8f,0.068f,0.0f);
	glVertex3f(-0.9f,0.2f,0.0f);
	glVertex3f(-0.9f,0.06f,0.0f);

	glVertex3f(-0.8f,-0.068f,0.0f);
	glVertex3f(-0.9f,-0.2f,0.0f);
	glVertex3f(-0.9f,-0.06f,0.0f);
	glEnd();

	glLineWidth(1.5f);
	glBegin(GL_LINES);
	glColor3f(0.0f,0.0f,0.0f);
	//I
	glVertex3f(-0.64f,0.05f,0.0f);
	glVertex3f(-0.64f,-0.05f,0.0f);

	//A
	glVertex3f(-0.61f,0.05f,0.0f);		
	glVertex3f(-0.63f,-0.05f,0.0f);

	glVertex3f(-0.62f,0.0f,0.0f);
	glVertex3f(-0.60,0.0f,0.0f);

	glVertex3f(-0.61f,0.05f,0.0f);	
	glVertex3f(-0.59f,-0.05f,0.0f);

	//F	
	glVertex3f(-0.58f,0.05f,0.0f);
	glVertex3f(-0.58f,-0.05f,0.0f);
	
	glVertex3f(-0.58f,0.05f,0.0f);
	glVertex3f(-0.55f,0.05f,0.0f);

	glVertex3f(-0.58f,0.01f,0.0f);
	glVertex3f(-0.56f,0.01f,0.0f);

	glEnd();

	glLineWidth(9.0f);
	glBegin(GL_LINES);
	glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
	glVertex3f(-0.9f,0.040f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3f(-1.3f,0.040f,0.0f);

	glColor3f(1.0f,1.0f,1.0f);
	glVertex3f(-0.9f,0.0f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3f(-1.3f,0.0f,0.0f);

	glColor3f(GREEN_R,GREEN_G,GREEN_B);
	glVertex3f(-0.9f,-0.040f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);	
	glVertex3f(-1.3f,-0.040f,0.0f);

	glEnd();

	if(!bInvTrans) {
		fxTrans += 0.0001f;
		fyTrans += 0.0001f;
		if(fxTrans >= 3.8f){
			fxTrans = 3.8f;
			bInvTrans = true;
		}
		if(fyTrans >= 0.0f){
			fyTrans = 0.0f;
		}
		angle -= 0.004f;
		if(angle <= 0.0f){
			angle = 0.0f;
		}
	}
	else {
		fIxTrans += 0.0001f;
		fIyTrans += 0.0001f;

		Iangle += 0.004f;
	}
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
