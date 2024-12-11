#include<windows.h>
#include<stdio.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include<iostream>
#include<vector>
using namespace std;

#define _USE_MATH_DEFINES 1
#include<math.h>
#include"PosVector.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"user32.lib")

//Global Vars
HWND ghwnd = NULL;
HGLRC ghrc = NULL;
HDC ghdc = NULL;
bool gbActiveWindow = false;
bool gbFullScreen = false;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
FILE *fptr;
GLfloat fAngle = 0.0f;

//Vector Variable
vector<PosVector> Data;

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

    //local Vars & functions
    int initialize(void);
    void display(void);
    void update(void);
    void ToggleFullScreen(void);

    WNDCLASSEX wndclass;
    HWND hwnd;
    TCHAR szAppName[] = TEXT("OGL_FFP");
    MSG msg;
    bool bDone = false;
    int iRet = 0;

    if(fopen_s(&fptr,"SphereLog.txt","w") != 0){
        MessageBox(NULL, TEXT("Cannot Create Log!!"), TEXT("ErrMsg"), MB_OK);
        exit(0);
    }
    else {
        fprintf(fptr,"Log File Created Successfully!!..\n\n");
    }

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc= MyCallBack;
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hInstance = hInstance;
    wndclass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    RegisterClassEx(&wndclass);

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
            szAppName,
            TEXT("Sphere"),
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
        fprintf(fptr,"ChoosePixelFormat Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -2) {
        fprintf(fptr,"SetPixelFormat Failed!!..\n");
        DestroyWindow(hwnd);
    }
        else if(iRet == -3) {
        fprintf(fptr,"wglCreateContext Failed!!..\n");
        DestroyWindow(hwnd);
    }
        else if(iRet == -4) {
        fprintf(fptr,"wglMakeCurrent Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else {
        fprintf(fptr,"Initialization Successful!!..\n\n");
    }

    ToggleFullScreen();
    ShowWindow(hwnd, iCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    while(!bDone) {

        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if(msg.message == WM_QUIT ){
                bDone = true;
            }
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else {
            if(gbActiveWindow == true) {
                display();
                update();
            }
        }
    }

    return((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

    //func
    void resize(int, int);
    void uninitialize(void);
    void ToggleFullScreen(void);

    switch(iMsg) {
        case WM_SETFOCUS:
            gbActiveWindow = true;
            break;

        case WM_KILLFOCUS:
            gbActiveWindow = false;
            break;

        case WM_ERASEBKGND:
            return(0);

        case WM_KEYDOWN:
            switch(wParam) {
                case 'f':
                case 'F':
                    ToggleFullScreen();
                    break;

                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;
            }
            break;

            case WM_SIZE:
                resize(LOWORD(lParam), HIWORD(lParam));
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
    MONITORINFO mi;

    if(!gbFullScreen) {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        if(dwStyle &WS_OVERLAPPEDWINDOW) {
            mi= {sizeof(MONITORINFO)};

            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi)) {

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
                    SWP_NOMOVE | SWP_FRAMECHANGED | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOSIZE);
        ShowCursor(TRUE);
        gbFullScreen = false;
    }
}

int initialize(void) {

    void resize(int,int);
    void createSphereData(void);
    
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;

    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cBlueBits = 8;
    pfd.cGreenBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;

    ghdc = GetDC(ghwnd);

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0) {
        return(-1);
    }
    else {
        fprintf(fptr, "ChoosePixelFormat Successful!!..\t{Index : %d}\n",iPixelFormatIndex);
    }
    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) {
        return(-2);
    }
    else {
        fprintf(fptr, "SetPixelFormat Successful!!..\n");
    }
    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL) {
        return(-3);
    }
    else {
        fprintf(fptr, "wglCreateContext Successful!!..\n");
    }
    if(wglMakeCurrent(ghdc, ghrc) == FALSE) {
        return(-4);
    }
    else {
        fprintf(fptr, "wglMakeCurrent Successful!!..\n");
    }

    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    resize(WIN_WIDTH, WIN_HEIGHT);

    //call to make array!.
    createSphereData();
    
    return(0);
}

void createSphereData(void) {

    float mapValues(float, float, float, float, float);
    int i, j, limit = 100;
    float lat, lon;
    float r = 2.0f;
    
    for(i=0; i<limit; i++) {
        lat = mapValues(i, 0, limit, -M_PI, M_PI);
        for(j=0; j < limit; j++) {
            lon = mapValues(j, 0, limit, -2* M_PI, 2*M_PI);
            float x = r * sin(lat) * cos(lon);
            float y = r * sin(lat) * sin(lon);
            float z = r * cos(lat);
            fprintf(fptr, "X:%f, Y:%f, Z:%f\n",x,y, z);
            Data.push_back(PosVector(x, y, z));
        }
    }
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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int i, j;
    static bool bFlag = false;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -8.0f);
    glPointSize(3.0f);
    glColor3f(1.0f, 0.0f, 0.0f);  
    /* glBegin(GL_POINTS);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glEnd(); */

    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(fAngle, 1.0f, 1.0f, 1.0f);
    for(i=0; i<100; i++) {
        for(j=0; j<100;j++) {
            PosVector p = Data.at((i*100)+j);
            glBegin(GL_POINTS);
                glVertex3f(p.x, p.y, p.z);
            glEnd();
        }
    }

    SwapBuffers(ghdc);
}

void update(void) {
    fAngle += 0.2f;
    if(fAngle >= 360.0f) {
        fAngle = 0.0f;
    }
}

float mapValues(float index, float _start, float _end, float _mapStart, float _mapEnd) {

    return(_mapStart + (_mapEnd - _mapStart) * ( (index - _start) / (_end - _start)));
}

PosVector::PosVector(float _x, float _y, float _z) {
    x = _x;
    y = _y;
    z = _z;
}

PosVector::~PosVector() {

}

void uninitialize(void) {
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