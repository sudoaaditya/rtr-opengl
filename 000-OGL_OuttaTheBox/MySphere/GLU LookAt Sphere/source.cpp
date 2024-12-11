#include<Windows.h>

#include<gl/GL.h>
#include<gl/GLU.h>

#include<math.h>

#include <stdio.h>

#include "header.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HDC ghdc = nullptr;
HWND ghwnd = nullptr;
HGLRC ghrc = nullptr;
DWORD dwStyle;

WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};

bool gbFullscreen = false;
bool gbEscapeKeyIsPressed = false;
bool gbActiveWindow = false;

int keyIsPressed = 0;
GLuint Texture_Earth; //to store texture data 
GLuint Texture_Star;

static float rotateAngle = 0.0f;
static float radius = 1.0f;

GLUquadric* quadric = nullptr;
GLUquadric* starQuadric = nullptr;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    void initialize(void);
    void uninitialize(void);
    void display(void);
    void update(void);

    HWND hwnd;
    WNDCLASSEX wndclass;
    MSG msg;
    TCHAR szAppName[] = TEXT("Earth LookAt");
    bool bDone = false;

    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hbrBackground = CreateSolidBrush(RGB(255, 255,255));
    wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.hInstance = hInstance;
    wndclass.lpfnWndProc = WndProc;
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = nullptr;
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);

    RegisterClassEx(&wndclass);
    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                          szAppName,
                          TEXT("Earth LookAt"),
                          WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          WIN_WIDTH,
                          WIN_HEIGHT,
                          nullptr,
                          nullptr,
                          hInstance,
                          nullptr);
    ghwnd = hwnd;

    initialize();
    ShowWindow(hwnd, nCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    while (bDone == false)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bDone = true;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if (gbActiveWindow == true)
            {
                if (gbEscapeKeyIsPressed == true)
                {
                    bDone = true;
                }
                update();
                display();
            }
        }
    }

    uninitialize();
    return ((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    void ToggleFullscreen(void);
    void resize(int, int);
    void uninitialize(void);

    switch (iMsg)
    {
    case WM_CREATE:
        break;
    case WM_ACTIVATE:
        if (HIWORD(wParam) == 0)
            gbActiveWindow = true;
        else
            gbActiveWindow = false;
        break;
    case WM_KEYDOWN:
        switch (wParam)
        {
        case VK_ESCAPE:
            DestroyWindow(hwnd);
            break;
        case VK_UP:
            radius += 0.05f;
            break;
        case VK_DOWN:
            radius -= 0.05f;
            break;
        case 0x46:
            if (gbFullscreen == false)
            {
                ToggleFullscreen();
                gbFullscreen = true;
            }
            else
            {
                ToggleFullscreen();
                gbFullscreen = false;
            }
            break;
        default:
            glDisable(GL_TEXTURE_2D);
            keyIsPressed = 0;
            break;
        }
        break;
    case WM_SIZE:
        resize(LOWORD(lParam), HIWORD(lParam));
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}


void ToggleFullscreen(void)
{
    MONITORINFO mi;
    if (gbFullscreen == false)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        if (dwStyle & WS_OVERLAPPEDWINDOW)
        {
            mi = {sizeof(MONITORINFO)};
            if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY),
                                                                     &mi))
            {
                SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                             mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
                             SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }
        ShowCursor(FALSE);
    }
    else
    {
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowCursor(TRUE);
    }
}

void initialize(void)
{
    void resize(int, int);
    int LoadGLTextures(GLuint*, TCHAR []);


    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex;

    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 24;

    ghdc = GetDC(ghwnd);

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if (iPixelFormatIndex == 0)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = nullptr;
    }

    if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = nullptr;
    }

    ghrc = wglCreateContext(ghdc);
    if (ghrc == nullptr)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = nullptr;
    }

    if (wglMakeCurrent(ghdc, ghrc) == FALSE)
    {
        wglDeleteContext(ghrc);
        ghrc = nullptr;
        ReleaseDC(ghwnd, ghdc);
        ghdc = nullptr;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //white color

    glShadeModel(GL_SMOOTH);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

    glEnable(GL_TEXTURE_2D);

    LoadGLTextures(&Texture_Earth,MAKEINTRESOURCE(IDB_EARTH));
    LoadGLTextures(&Texture_Star, MAKEINTRESOURCE(IDB_STAR));

    quadric = gluNewQuadric();
    gluQuadricTexture(quadric, TRUE);

    starQuadric = gluNewQuadric();
    gluQuadricTexture(starQuadric, TRUE);

    resize(WIN_WIDTH, WIN_HEIGHT);
}


int LoadGLTextures(GLuint* texture, TCHAR imageResourceId[])
{
    HBITMAP hBitmap;
    BITMAP bmp;
    int iStatus = FALSE;
    HRESULT hr;

    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(nullptr),
                                 imageResourceId,
                                 IMAGE_BITMAP,
                                 0,
                                 0,
                                 LR_CREATEDIBSECTION);

    hr = HRESULT_FROM_WIN32(GetLastError());


    if (hBitmap)
    {
        iStatus = TRUE;
        GetObject(hBitmap, sizeof(bmp), &bmp);

        glGenTextures(1, texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        glBindTexture(GL_TEXTURE_2D, *texture);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
        gluBuild2DMipmaps(GL_TEXTURE_2D, 3, bmp.bmWidth, bmp.bmHeight,GL_BGR_EXT,GL_UNSIGNED_BYTE, bmp.bmBits);
        DeleteObject(hBitmap);
    }

    return iStatus;
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    static int eyeZ = 0.0f;

    gluLookAt(0.0f, 0.0f, radius * sin(rotateAngle), 
              0.0f, 0.0f, 0.0f, 
              0.0f, 1.0f, 0.0f);

    //gluLookAt(0.0f, 0.0f, eyeZ,
    //          0.0f, 0.0f, 0.0f,
    //          0.0f, 1.0f, 0.0f);

    eyeZ += 1.0f;

    glRotatef(270.0f, 1.0f, 0.0f, 0.0f);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, Texture_Earth);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    gluSphere(quadric, 0.30, 30, 30);

    glBindTexture(GL_TEXTURE_2D, Texture_Star);
    gluSphere(quadric, 1.0, 30, 30);

    SwapBuffers(ghdc);
}

void update(void)
{
    rotateAngle += 0.0005f;
    if (rotateAngle >= 360.0f)
        rotateAngle = 1.0f;
}

void resize(int width, int height)
{
    if (height == 0)
        height = 1;
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void uninitialize(void)
{
    if (gbFullscreen == true)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

        ShowCursor(TRUE);
    }

    if (Texture_Earth)
    {
        glDeleteTextures(1, &Texture_Earth);
        Texture_Earth = 0;
    }

    if (Texture_Star)
    {
        glDeleteTextures(1, &Texture_Star);
        Texture_Star = 0;
    }

    wglMakeCurrent(nullptr, nullptr);

    wglDeleteContext(ghrc);
    ghrc = nullptr;

    ReleaseDC(ghwnd, ghdc);
    ghdc = nullptr;

    DestroyWindow(ghwnd);
    ghwnd = nullptr;
}
