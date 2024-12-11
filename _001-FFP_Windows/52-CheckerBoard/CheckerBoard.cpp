#include<windows.h>
#include<stdio.h>
#include<gl/GL.h>
#include<gl/GLU.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define CHECKER_IMAGE_WIDTH 64
#define CHECKER_IMAGE_HEIGHT 64


//glob Vars
//FullScren
bool gbFullScreen = false;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
HWND ghwnd = NULL;

//Rendering
bool gbActiveWindow = false;
HGLRC ghrc = NULL;
HDC ghdc = NULL;
FILE *fptr = NULL;

//tex var
GLubyte CheckerImage[CHECKER_IMAGE_WIDTH][CHECKER_IMAGE_HEIGHT][4];
GLuint iTexChecker;
GLfloat fRotAngle = 0.0f;




LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

    //vars & Fun
    int initialize(void);
    void ToggleFullScreen(void);
    void display();

    WNDCLASSEX wndclass;
    HWND hwnd;
    TCHAR szAppNam[] = TEXT("OGL_PREPS");
    MSG msg;
    bool bDone = false;
    int iRet = 0;

    //file Opening
    if(fopen_s(&fptr, "_CheckerLog.txt", "w") != 0) {
        MessageBox(NULL, TEXT("Cnnot Create Log!!.."), TEXT("ErrMsg"), MB_OKCANCEL);
        exit(0);
    }
    else {
        fprintf(fptr, "File Created Successfully!..\n\n");
    }

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpszClassName = szAppNam;
    wndclass.lpszMenuName = NULL;
    wndclass.lpfnWndProc = MyCallBack;
    wndclass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hInstance = hInstance;
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassEx(&wndclass);

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
        szAppNam,
        TEXT("CheckerBoard"),
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
        fprintf(fptr, "ChoosePixelFormat Falied!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -2) {
        fprintf(fptr, "SetPixelFormat Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -3) {
        fprintf(fptr, "wglCreateCotext Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -4) {
        fprintf(fptr, "wglMakeCurrent Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else {
        fprintf(fptr, "Initialization Succesful!!..\n");
    }

    ToggleFullScreen();
    ShowWindow(hwnd, iCmdShow);
    SetFocus(hwnd);


    //Game Loop
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
            if(gbActiveWindow) {
                display();
            }
        }
    }

    return((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

    //func
    void ToggleFullScreen(void);
    void uninitialize(void);
    void resize(int, int);

    switch(iMsg) {
        
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
            switch(wParam) {
                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;

                case'F':
                case 'f':
                    ToggleFullScreen();
                    break;

            }
            break;

        
        case WM_ERASEBKGND:
            return(0);

        case WM_CLOSE:
            //uninitialize();
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
            mi = { sizeof(MONITORINFO) };

            if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi)) {

                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

                SetWindowPos(ghwnd,
                    HWND_TOP,
                    mi.rcMonitor.left,
                    mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_FRAMECHANGED | SWP_NOZORDER);
            }
        }
        gbFullScreen = true;
        ShowCursor(FALSE);
    }

    else {

            SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

            SetWindowPlacement(ghwnd, &wpPrev);

            SetWindowPos(ghwnd, 
                HWND_TOP,
                0, 0, 0, 0,
                SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOOWNERZORDER);

            ShowCursor(TRUE);
            gbFullScreen = false;
    }
}

int initialize(void) {



    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;

    void resize(int, int);
    void loadTexture(void);

    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;


    ghdc = GetDC(ghwnd);

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0){
        return(-1);
    }
    else {
        fprintf(fptr, "ChoosePixelFormat Successful!!.\n");
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
        fprintf(fptr, "wglMakeCurent Successful!!../n");
    }

    glShadeModel(GL_SMOOTH);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_TEXTURE_2D);
   
    loadTexture();

    resize(WIN_WIDTH, WIN_HEIGHT);

    return(0);
}

void loadTexture(void) {

    //Vars
    void MakeCheckImage(void);

    MakeCheckImage();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &iTexChecker);

    glBindTexture(GL_TEXTURE_2D, iTexChecker);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0,
        GL_RGBA, 
        CHECKER_IMAGE_WIDTH,
        CHECKER_IMAGE_HEIGHT,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        CheckerImage);

    glTexEnvf(GL_TEXTURE_ENV,
        GL_TEXTURE_ENV_MODE,
        GL_REPLACE);

}

void resize(int width, int height) {

    if(height == 0) {
        height = 1;
    }

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f,
        (GLfloat)width / (GLfloat)height,
        0.1f,
        100.0f);

}

void display(void) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -3.0f);

    glRotatef(fRotAngle, 0.0f, 1.0f, 0.0f);
    glBindTexture(GL_TEXTURE_2D, iTexChecker);

    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-2.0f, -1.0f, 0.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-2.0f, 1.0f, 0.0f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(0.0f, -1.0f, 0.0f);


    //Quad 2

    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 0.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 0.0f);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(2.41421f, 1.0f, -1.41421f);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(2.41421f, -1.0f, -1.41421f);

    glEnd();

    fRotAngle += 0.04f;
    if(fRotAngle >= 360.0f) {
        fRotAngle = 0.0f;
    }

    SwapBuffers(ghdc);
}

void uninitialize(void) {

    if(gbFullScreen) {
        
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

        SetWindowPlacement(ghwnd, &wpPrev);

        SetWindowPos(ghwnd, 
            HWND_TOP,
            0, 0, 0, 0,
            SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOSIZE | SWP_NOOWNERZORDER);

        ShowCursor(TRUE);
        gbFullScreen = false;
    }

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

    if(iTexChecker) {
        glDeleteTextures(1, &iTexChecker);
        iTexChecker = 0;

    }

    if(fptr) {
        fprintf(fptr, "\nLog Closed Successfully!!..\n");
        fclose(fptr);
        fptr = NULL;
    }

}

void MakeCheckImage(void) {
    int i, j, c;

    for(i = 0; i < CHECKER_IMAGE_HEIGHT; i++) {
        for(j = 0; j < CHECKER_IMAGE_WIDTH; j++) {

            c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0)) * 255;

            CheckerImage[i][j][0] = (GLubyte)c;
            CheckerImage[i][j][1] = (GLubyte)c;
            CheckerImage[i][j][2] = (GLubyte)c;
            CheckerImage[i][j][3] = (GLubyte)c;
        }
    }
}