#include<windows.h>
#include<stdio.h>
#include<gl/GL.h>
#include<gl/GLU.h>

#include<time.h>
#include<iostream>
#include<vector>
#include"Bubble.h"
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
bool gbActiveWindow = false;
DWORD dwStyle;
HWND ghwnd = NULL;
HGLRC ghrc = NULL;
HDC ghdc = NULL;
WINDOWPLACEMENT wpPrev;
FILE *fptr = NULL;

int iFrameRate, i=0;
int giStack = 50, giSlice = 50;
GLfloat gfRad = 10.0f;

vector<Bubble> Bubbles;

//for Light
bool bLights = false;

GLfloat lightAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightPosition[] = { 50.0f, 70.0f, 50.0f, 1.0f }; 

/* GLfloat materialAmbient[] = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat materialDiffuse[] = { 0.588235f, 0.670588f, 0.729412f, 1.0f };
GLfloat materialSpecular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
GLfloat materialShininess[] = { 0.75f * 128.0f }; */

GLfloat materialAmbient[] = { 0.878431f, 0.937254f, 0.956862f, 1.0f };
GLfloat materialDiffuse[] = { 0.337254f, 0.674509f, 0.764705f, 1.0f };
GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialShininess[] = { 0.75f * 128.0f };

/*
GLfloat materialAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
GLfloat materialDiffuse[] = { 0.1f, 0.1f, 0.1f, 1.0f };
GLfloat materialSpecular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat materialShininess[] = { 77.0f };
*/

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

    //var & func
    int initialize(void);
    void ToggleFullScreen(void);
    void display(void);
    void InitVector(void);

    MSG msg;
    TCHAR szAppName[] = TEXT("3D_ORTHO");
    HWND hwnd;
    WNDCLASSEX wndclass;
    bool bDone = false;
    int iRet = 0;

    if(fopen_s(&fptr, "_ScreenSaverLog.txt","w") != 0) {
        MessageBox(NULL, TEXT("Cannot Create Log!!"), TEXT("ErrMsg"), MB_OK);
        exit(0);
    }
    else {
        fprintf(fptr, "Log Created Successfully!!..\n\n");
    }

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;
    wndclass.lpfnWndProc = MyCallBack;
    wndclass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hInstance = hInstance;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassEx(&wndclass);

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                szAppName,
                TEXT("ScreenSaver"),
                WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                100, 100,
                WIN_WIDTH, WIN_HEIGHT,
                NULL,
                NULL,
                hInstance,
                NULL);

    ghwnd = hwnd;

    iRet = initialize();

    if(iRet == -1) {
        fprintf(fptr, "ChoosePixelFormat Failed!!\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -2) {
        fprintf(fptr, "SetPixelFormat Failed!!\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -3) {
        fprintf(fptr, "wglCreateContect Failed!!\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -4) {
        fprintf(fptr, "wglMakeCurrent Failed!!\n");
        DestroyWindow(hwnd);
    }
    else {
        fprintf(fptr, "Initialization Successful!!!\n");
    }
    
    ToggleFullScreen();
    ShowWindow(hwnd, iCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    //Create Bubbles
    InitVector();

    while(!bDone){
        if(PeekMessage(&msg, 0, 0, NULL, PM_REMOVE)) {
            if(msg.message == WM_QUIT) {
                bDone = true;
            }
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else {
            if(gbActiveWindow) {
                display();
            }
        }
    }

    return((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

    static bool bIsMax = false;
    void ToggleFullScreen(void);
    void resize(int, int);
    void uninitialize(void);

    switch(iMsg) {
        case WM_SETFOCUS:
            gbActiveWindow = true;
            break;

        case WM_KILLFOCUS:
            gbActiveWindow = false;
            break;

                
        case WM_KEYDOWN:
            switch(wParam) {
                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;
                
                case 'F':
                case 'f':
                    ToggleFullScreen();
                    break;

                case 's':
                case 'S':
                    if(!bIsMax){
                        ShowWindow(hwnd, SW_MAXIMIZE);
                        bIsMax = true;
                    }
                    else {
                        ShowWindow(hwnd, SW_NORMAL);
                        bIsMax = false;
                    }
                    break;
                
                case 'L':
                case 'l':
                    if(!bLights) {
                        bLights = true;
                        glEnable(GL_LIGHTING);
                    }
                    else {
                        bLights = false;
                        glDisable(GL_LIGHTING);
                    }
                    break;
            }
            break;

        case WM_SIZE:
            resize(LOWORD(lParam), HIWORD(lParam));
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

void ToggleFullScreen(void) {

    MONITORINFO mi;
    if(!gbFullScreen) {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if(dwStyle & WS_OVERLAPPEDWINDOW) {
            mi = {sizeof(MONITORINFO)};

            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi)){

                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

                SetWindowPos(ghwnd,
					    HWND_TOP,
					    mi.rcMonitor.left,
					    mi.rcMonitor.top,
					    mi.rcMonitor.right - mi.rcMonitor.left,
					    mi.rcMonitor.bottom - mi.rcMonitor.top,
					    SWP_FRAMECHANGED | SWP_NOZORDER);

            }
            ShowCursor(FALSE);
            gbFullScreen = true;
        }
    }
    else {

        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

        SetWindowPlacement(ghwnd, &wpPrev);

        SetWindowPos(ghwnd,
                HWND_TOP,
                0, 0, 0, 0,
                SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOOWNERZORDER);

        ShowCursor(TRUE);
        gbFullScreen = false;
    }
}

int initialize(void) {

    void resize(int, int);

    int iPixelFormatIndex = 0;
    PIXELFORMATDESCRIPTOR pfd;

    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 32;
    pfd.cRedBits = 8;
    pfd.cBlueBits = 8;
    pfd.cGreenBits = 8;
    pfd.cAlphaBits = 8;

    ghdc = GetDC(ghwnd);

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0){
        return(-1);
    }
    else {
        fprintf(fptr, "ChoosePixelFormat Successful!!..(Index = %d)\n",iPixelFormatIndex);
    }

    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) {
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

    if(wglMakeCurrent(ghdc, ghrc) == FALSE){
        return(-4);
    }
    else {
        fprintf(fptr,"wglMakeCurrent Successful!!..\n");
    }

    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0,  GL_POSITION, lightPosition);
    glEnable(GL_LIGHT0);

    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);


    resize(WIN_WIDTH, WIN_HEIGHT);

    srand(time(NULL));

    return(0);
}

void resize(int width, int height) {

    if(height == 0) {
        height = 1;
    }
    

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if(width < height) {
        glOrtho(0.0f,
            100.0f,
            0.0f,
            100.0f * ((GLfloat)height/(GLfloat)width),
            -100.0f,
            100.0f);
    }
    else {
        glOrtho(0.0f,
            100.0f * ((GLfloat)width/(GLfloat)height),
            0.0f,
            100.0f,
            -100.0f,
            100.0f);
    }

}

void display(void) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    for(i = 0; i < Bubbles.size(); i++) {
        Bubbles[i].Show();
    }

    if(iFrameRate == 60) {

        for(i = 0; i < Bubbles.size(); i++) {
            //Bubbles.at(i).CalculateTrans();
        }
        iFrameRate = 0;
    }
    iFrameRate++;
    SwapBuffers(ghdc);
}

Bubble::Bubble(GLfloat x_, GLfloat y_) {

    fxTrans = x_;
    fyTrans = y_;

    sphere = gluNewQuadric();

}

Bubble::~Bubble() {

    gluDeleteQuadric(sphere);
    sphere = NULL;
}

void Bubble::Show() {

    glTranslatef(this->fxTrans, this->fyTrans, 0.0f);

    gluSphere(this->sphere, gfRad, giStack, giSlice);

}

void Bubble::CalculateTrans(void) {

    
    if(this->fxTrans >= 80.0f || this->fxTrans <= 10.0f) {
        this->fxTrans -= 1.0f;
    }
    else {
        this->fxTrans += 1.0f;
    }
    
    if(this->fyTrans >= 60.0f || this->fyTrans <= 10.0f) {
        this->fyTrans -= 1.0f;
    }
    else {
        this->fyTrans += 1.0f;
    }
}


int Random(int min, int max) {

    return(min +( rand() % (max - min + 1)));
}

void InitVector(void) {

    for(i = 0; i < 25; i++) {
        GLfloat x = Random(20, 80);
        GLfloat y = Random(20, 60);
        fprintf(fptr, "%f\t%f\n",x,y);
        Bubbles.push_back(Bubble(x, y));
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