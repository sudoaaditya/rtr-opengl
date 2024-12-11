#include<stdio.h>	//header for file
#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

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

LRESULT CALLBACK MyCallBack(HWND,UINT,WPARAM,LPARAM);
void resize(int,int);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

	//functuions & Variables
	int initialize(void);
	void display(void);

	WNDCLASSEX wndclass;
	MSG msg;
	HWND hwnd;
	TCHAR szAppName[] = TEXT("OGL_Pers");
	bool bDone = false;
	int iRet = 0;

	if(fopen_s(&fptr,"DHLog.txt","w")!=0){
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
		TEXT("Deathly Hallows"),
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

	gluPerspective(45.0f,
		((GLfloat)width / (GLfloat)height),
		0.1f,
		100.0f);

}

void display(void){

	//Variables
	GLfloat fx1,fx2,fx3,fy1,fy2,fy3;
	GLfloat fRad,fPer,fArea,fxCord,fyCord;
	GLfloat fdAB,fdBC,fdAC;
	static bool bToWrite = false;

	glClear(GL_COLOR_BUFFER_BIT);

	//Draw Triangle
	fx1 = 0.0f;
	fy1 = 1.0f;
	fx2 = -1.0f;
	fy2 = -1.0f;
	fx3 = 1.0f;
	fy3 = -1.0f;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-3.0f);
	glLineWidth(2.0f);
	glBegin(GL_LINES);

	glVertex2f(fx1,fy1);
	glVertex2f(fx2,fy2);

	glVertex2f(fx2,fy2);
	glVertex2f(fx3,fy3);

	glVertex2f(fx3,fy3);
	glVertex2f(fx1,fy1);
	glEnd();

	//Calcus For InCircle
	//Distances of Edges.
	fdAB = sqrtf(((fx2-fx1)*(fx2-fx1))+((fy2-fy1)*(fy2-fy1)));
	fdBC = sqrtf(((fx3-fx2)*(fx3-fx2))+((fy3-fy2)*(fy3-fy2)));
	fdAC = sqrtf(((fx3-fx1)*(fx3-fx1))+((fy3-fy1)*(fy3-fy1)));

	if(!bToWrite){
		fprintf(fptr,"\nDistances\nfdAB = %f\tfdBC = %f\tfdAC = %f\n",fdAB,fdBC,fdAC);
	}

	fPer = ((fdAB + fdBC + fdAC) /2);

	fArea = sqrtf(fPer * (fPer - fdAB) * (fPer - fdBC) * (fPer - fdAC));

	fRad = (fArea / fPer);

	if(!bToWrite) {
		fprintf(fptr,"SemiPermeter = %f\nArea Of Triangles = %f\nRadius Of InCircle = %f\n",fPer,fArea,fRad);
	}

	//Origin Of InCircle
	fxCord = (GLfloat)(((fdBC*fx1) + (fx2*fdAC) + (fx3*fdAB)) / (fPer * 2));
	fyCord = (GLfloat)(((fdBC*fy1) + (fy2*fdAC) + (fy3*fdAB)) / (fPer * 2));

	if(!bToWrite) {
		fprintf(fptr,"Origin Of Incircle\nX Coord = %f\nY Coord = %f\n",fxCord,fyCord);
	}	

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-3.0f);
	glPointSize(2.0f);
	glBegin(GL_POINTS);
	for (GLfloat angle = 0.0f; angle < 2.0f*M_PI; angle += 0.001f){
		glVertex2f(fxCord + (sinf(angle)*fRad),fyCord + (cosf(angle)*fRad));
	}
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-3.0f);
	glLineWidth(2.0f);

	glBegin(GL_LINES);
	glVertex2f(0.0f,1.0f);
	glVertex2f(0.0f,-1.0f);
	glEnd();


	SwapBuffers(ghdc);

	bToWrite = true;
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
