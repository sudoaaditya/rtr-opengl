#include<stdio.h>	//header for file
#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<math.h>

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

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

//matrices for identity translation & rotation
static GLfloat Imat[16], TMat[16], RMat[16];

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

	if(fopen_s(&fptr,"TSLog.txt","w")!=0){
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
		TEXT("Spinning Triangle"),
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
	int i,j;

	static bool bIsMax = false;

	switch(iMsg) {

		// to initialize Identity Matrix
		case WM_CREATE:
			for(i=0;i<16;i++) {

				if(i%5 == 0) {
					Imat[i] = 1.0f;
				}
				else {
					Imat[i] = 0.0f;
				}
				fprintf(fptr,"%0.2f \n",Imat[i]);
			}

			break;

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

	//Initialization of Translatiton Matrix >> we areb translating at Z-axis at -3 so
	for(int i=0; i<16; i++) {
		if(i%5 == 0){
			TMat[i] = 1.0f;
		}
		else if(i == 14)
			TMat[i] = -4.0f;
		else{
			TMat[i] = 0.0f;
		}
		fprintf(fptr,"%0.2f \n",TMat[i]);
	}


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

	void FillRotationMat(GLfloat);
	//Variables
	static GLfloat fRAngle = 0.0f;
	static int count = 1;

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	//LoadIdentity
	glLoadMatrixf(Imat);

	//Multipliy it with Translation Mat
	glMultMatrixf(TMat);

	//Change Rotation Matrix first
	FillRotationMat(fRAngle);
	glMultMatrixf(RMat);

	glBegin(GL_TRIANGLES);

	//appex with RED
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);

	//left bottom with GREEN
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);

	//right bottom with Blue
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(1.0f, -1.0f);

	glEnd();
	
	fRAngle += 0.0009f;
	if(fRAngle == 360.0f){
		fRAngle = 0.0f;
	}

	SwapBuffers(ghdc);
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

void FillRotationMat(GLfloat fAngle) {
	RMat[0]=cosf(fAngle);
	RMat[1]=0.0f;
	RMat[2]=-sinf(fAngle);
	RMat[3]=0.0f;
	RMat[4]=0.0f;
	RMat[5]=1.0f;
	RMat[6]=0.0f;
	RMat[7]=0.0f;
	RMat[8]=sinf(fAngle);
	RMat[9]=0.0f;
	RMat[10]=cosf(fAngle);
	RMat[11]=0.0f;
	RMat[12]=0.0f;
	RMat[13]=0.0f;
	RMat[14]=0.0f;
	RMat[15]=1.0f;
}