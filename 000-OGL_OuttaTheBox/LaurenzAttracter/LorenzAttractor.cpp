#include<stdio.h>
#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>

//for DS
#include<iostream>
#include<vector>
#include"Vector.h"
using namespace std;


#define _USE_MATH_DEFINES 1
#include<math.h>
#define WIN_WIDTH 800
#define WIN_HEIGHT 600


#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")

//Global Variables
bool gbFullScreen = false;
DWORD dwStyle = NULL;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
bool gbActivateWindow = false;
FILE *fptr = NULL;

// Variables
GLfloat fx = 0.01f;
GLfloat fy = 0.02f;
GLfloat fz = 0.0f;

GLfloat fAlpha = 10.0f;
GLfloat fRoh = 28.0f;
GLfloat fBeta = (8.0f/3.0f);
vector<Vector> v;

GLfloat fRotAngle =0.0f;
GLfloat fRVal = 0.0f;
GLfloat fGVal = 0.0f;
GLfloat fBVal = 0.0f;


LRESULT CALLBACK MyCallBack(HWND,UINT,WPARAM,LPARAM);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreevInstance,LPSTR lpszCmdLine, int iCmdShow) {

    //var & Fun
    int initialize(void);
    void display(void);
    void update(void);
    void ToggleFullScreen(void);

    
    //Var
    WNDCLASSEX wndclass;
    MSG msg;
    TCHAR sszApp[] = TEXT("OGL_PRESP");
    HWND hwnd;
    bool bDone = false;
    int iRet = 0;

    if(fopen_s(&fptr,"LALog.txt","w") != 0) {
        MessageBox(NULL,TEXT("Cannot Create Log!!..."),TEXT("ErrMsg"),MB_OK);
        exit(0);     
    }
    else {
        fprintf(fptr,"File Created Successfully!!..\n\n");   

    }
    

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpszClassName = sszApp;
    wndclass.lpszMenuName = NULL;
    wndclass.lpfnWndProc = MyCallBack;
    wndclass.hbrBackground = CreateSolidBrush(RGB(0,0,0));
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
    wndclass.hIconSm = LoadIcon(NULL,IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);

    RegisterClassEx(&wndclass);
   

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
        sszApp,
        TEXT("Loreanz Attracter"),
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

    if(iRet == -1){
        fprintf(fptr,"ChoosePixelFormat Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -2){
        fprintf(fptr,"SetPixelFormat Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -3){
        fprintf(fptr,"wglCreateContext Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -4){
        fprintf(fptr,"wglMakeCurrent Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else {
        fprintf(fptr,"Initialization Successful!!..\n");
    }

    ToggleFullScreen();
    ShowWindow(hwnd,iCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);


    //Msg LOOP
    while( !bDone ){

        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

            if(msg.message == WM_QUIT) {
                bDone = true;
            }
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

        }
        else {
            if(gbActivateWindow == true){
                update();
                display();
            }
        }
    }

    return((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

    //Func
    void uninitialize(void);
    void resize(int, int);
    void ToggleFullScreen(void);


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

        case WM_ERASEBKGND:
            return(0);

        case WM_KEYDOWN:
            switch(wParam){
                case VK_ESCAPE:
                DestroyWindow(hwnd);
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

int initialize(void) {

    void resize(int, int);
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;

    ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 32;    //1st
    pfd.cRedBits = 8;
    pfd.cBlueBits = 8;
    pfd.cGreenBits = 8;
    pfd.cAlphaBits = 8;

    ghdc = GetDC(ghwnd);

    iPixelFormatIndex = ChoosePixelFormat(ghdc,&pfd);
    if(iPixelFormatIndex == 0) {
        return(-1);
    }
    else {
        fprintf(fptr,"ChoosePixelFormat Successful!(Index = %d\n",iPixelFormatIndex);
    }

    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) {
        return(-2);
    }
    else {
        fprintf(fptr,"SetPixelFormat Successful!!\n");
    }

    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL) {
        return(-3);
    }
    else {
        fprintf(fptr,"wglCreateContext Successful!!\n");
    }

    if(wglMakeCurrent(ghdc, ghrc) == FALSE) {
        return(-4);
    }
    else {
        fprintf(fptr,"wglMakeCurrent Successful!!\n");
    }

    //Shade Model
    glShadeModel(GL_SMOOTH);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glClearDepth(1.0f); //2nd

    //4th ones
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_POINT_SMOOTH);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    resize(WIN_WIDTH, WIN_HEIGHT);


    return(0);
}

void resize(int width, int height) {

	if(height == 0){
		height = 1;
	}

    glViewport(0, 0, (GLsizei)width,(GLsizei)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(45.0f, 
        (GLfloat)width / (GLfloat)height,
        0.1f,
        200.0f);
    //gluOrtho2D(-width/2,width/2,-height/2,height/2);
    //gluOrtho2D(0,width,height,0);
}


void display(void) {

    static int iChngCnt =0;

    //3rd One
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(-2.0f, 0.0f, -150.0f);

    //glScalef(5.0f, 5.0f, 5.0f);

    glRotatef(fRotAngle, 1.0f, 1.0f, 1.0f);


    if(iChngCnt == 50) {  
        
        GLfloat fdt = 0.01f;

        GLfloat fdx = (fAlpha * (fy- fx)) * fdt;
        GLfloat fdy = (fx * (fRoh - fz) - fy) * fdt;
        GLfloat fdz = ( fx * fy - fBeta * fz) * fdt;

        fx = fx + fdx;
        fy = fy + fdy;
        fz = fz + fdz;

        v.push_back(Vector(fx, fy, fz));
        iChngCnt = 0;
    }
    iChngCnt++;
    //glLineWidth(2.0f);
    glPointSize(2.0f);
    glBegin(GL_POINTS);

    for(int i = 0; i< v.size(); i++) {

        //glColor3f(fRVal, fGVal, fBVal);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(v[i].x, v[i].y, v[i].z );

/*         if(iChngCnt == 100) {
            fRVal += 0.001f;
            if(fRVal >= 1.0f) {
                fRVal = 0.0f;
            }
            fGVal += 0.01f;
            if(fGVal >= 1.0f) {
                fGVal = 0.0f;
            }
            fBVal += 0.0001f;
            if(fBVal >= 1.0f) {
                fBVal = 0.0f;
           }
           iChngCnt = 0;
        }
        iChngCnt++;
 */    }
    glEnd();
    SwapBuffers(ghdc);

}

void update(void) {

    fRotAngle += 0.008f;
    if(fRotAngle >= 360.0f) {
        fRotAngle = 0.0f;
    }


}

void uninitialize(void) {

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
