#include<stdio.h>
#include<windows.h>
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

//Global vars
HWND ghwnd = NULL;
HDC ghdc  =  NULL;
HGLRC ghrc = NULL;
bool gbActivateWindow = false;
FILE *fptr;

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstannce, LPSTR lpszICmdLine, int iCmdShow) {

    //vars & Funcs
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("OGL_PERSP");
    bool bDone = false;
    int iRet = 0;

    int initialize(void);
    void display(void);

    if(fopen_s(&fptr, "_FractalLog.txt", "w") != 0) {
        MessageBox(NULL, TEXT("Cannot Create File!!.."), TEXT("ErrMsg"), MB_OK);
        exit(0);
    }
    else{
        fprintf(fptr,"File Created Successfully!!..\n\n");
    }

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpfnWndProc = MyCallBack;
    wndclass.hbrBackground =  CreateSolidBrush(RGB(0, 0, 0));
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hIconSm  = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hInstance = hInstance;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassEx(&wndclass);

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
            szAppName,
            TEXT("Fractals"),
            WS_OVERLAPPEDWINDOW | 
            WS_CLIPCHILDREN | 
            WS_CLIPSIBLINGS | 
            WS_VISIBLE,
            100, 100,
            WIN_WIDTH,
            WIN_HEIGHT,
            NULL,
            NULL,
            hInstance,
            NULL);

    ghwnd = hwnd;

    iRet = initialize();
    if(iRet == -1) {
        fprintf(fptr, "ChoosePixelFormat Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -2) {
        fprintf(fptr, "SetPixelFormat Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -3) {
        fprintf(fptr, "wglCreateContext Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -4) {
        fprintf(fptr, "wgl MakeCurrent Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else {
        fprintf(fptr, "Initialization Successful!1..\n");
    }

    ShowWindow(hwnd, iCmdShow);
    SetFocus(hwnd);
    SetForegroundWindow(hwnd);

    //game Loop
    while(!bDone) {
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
            if(gbActivateWindow) {
                display();
            }
        }
    }

    return((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

    void uninitialize(void);
    void resize(int, int);

    switch(iMsg) {

        case WM_SETFOCUS:
            gbActivateWindow = true;
            break;
        
        case WM_KILLFOCUS:
            gbActivateWindow = false;
            break;

        case WM_KEYDOWN:
            switch(wParam) {
                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;
            }
            break;

        case WM_SIZE:
            resize(HIWORD(lParam), LOWORD(lParam));
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

int initialize(void) {

    void resize(int, int);

    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;

    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;

    ghdc = GetDC(ghwnd);

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0) {
        return(-1);
    }
    else {
        fprintf(fptr,"ChoosePixelFormatSuccessful!! (Indx=%d)\n",iPixelFormatIndex);
    }

    if(SetPixelFormat(ghdc,iPixelFormatIndex, &pfd) == FALSE) {
        return(-2);
    
    }
    else {
        fprintf(fptr,"SetPixelFormat Successful!!..\n");
    }

    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL) {
        return(-3);
    }
    else {
        fprintf(fptr,"wglCreateContext Successful!..\n");
    }

    if(wglMakeCurrent(ghdc, ghrc) == FALSE) {
        return(-4);
    }
    else {
        fprintf(fptr,"wglMakeCurrent Successful!!..\n");
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    resize(WIN_WIDTH, WIN_HEIGHT);

    return(0);
}

void resize(int width, int height) {

    if(height == 0) {
        height = 1;
    }

    glViewport(0, 0, (GLsizei)width,(GLsizei)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(0, width, height, 0);

}

void display(void) {

    void Branch(int);

    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(400, WIN_HEIGHT, 0.0f);

    glColor3f(0.0f, 1.0f, 0.0f);
    glLineWidth(3.0f);

    Branch(200);

    SwapBuffers(ghdc);
}

void Branch(int iLen) {
    
    glBegin(GL_LINES);
    glVertex2i(0, 0);
    glVertex2i(0, - iLen);
    glEnd();

    glTranslatef(0.0f, -iLen ,0.0f);
    
    if(iLen > 2) {

        glPushMatrix();
        glRotatef(45.0f, 0.0f, 0.0f,  1.0f);    
        Branch(iLen*0.67);
        glPopMatrix();
/* 
        glPushMatrix();
        glRotatef(30.0f, 0.0f, 0.0f,  1.0f);    
        Branch(iLen*0.67);
        glPopMatrix();

        glPushMatrix();
        glRotatef(-30.0f, 0.0f, 0.0f,  1.0f);    
        Branch(iLen*0.67);
        glPopMatrix(); */

        glPushMatrix();
        glRotatef(-45.0f, 0.0f, 0.0f,  1.0f);    
        Branch(iLen*0.67);
        glPopMatrix();
    }
}

void uninitialize(void) {

    if(wglGetCurrentContext() == ghrc) {
        wglMakeCurrent(NULL, NULL);
    }

    if(ghrc) {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    if(ghdc) {
        DeleteDC(ghdc);
        ghdc = NULL;
    }

    if(fptr) {
        fprintf(fptr, "File Closed Successfully!!..\n");
        fclose(fptr);
        fptr = NULL;
    }
}


