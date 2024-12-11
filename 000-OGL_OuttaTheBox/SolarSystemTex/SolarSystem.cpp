#include<windows.h>
#include<stdio.h>
#include<gl/GL.h>
#include<gl/glu.h>
#include"TexResHeader.h"
#include"Planets.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"winmm.lib")


//global variables
bool gbFullScreen = false;
DWORD dwStyle = NULL;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
HWND ghwnd = NULL;
bool gbActiveWindow = false;
HDC ghdc= NULL;
HGLRC ghrc = NULL;
FILE *fptr = NULL;
 
//rot var
GLUquadric *MySun = NULL;
GLUquadric *MyEarth = NULL;
//GLUquadric *MyMoon = NULL;
GLfloat fDay = 0.0f;
GLfloat fYear = 0.0f;
//GLfloat fMoonRot = 0.0f;
GLfloat fLookVal = 0.7f;
bool bRenderEarth = false;

//TexVars
GLuint iSunTex = 0;
//GLuint iMoonTex = 0;
GLuint iEarthTex = 0;
GLuint iGalaxyTex = 0;


LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

    //Func
    int initialize(void);
    void display(void);
    void update(void);
    void ToggleFullScreen(void);

    //func
    WNDCLASSEX wndclass;
    MSG msg;
    HWND hwnd;
    TCHAR szAppName[] = TEXT("3D-Presp");
    bool bDone = false;
    int iRet = 0;

    if(fopen_s(&fptr,"_SolarSystemLog.txt","w") != 0) {
        MessageBox(NULL,TEXT("Cannot Create Log!!.."),TEXT("ErrMsg"),MB_OK);
        exit(0);
    }
    else {
        fprintf(fptr,"Log Created Successful!!\n\n");
    }

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = NULL;
    wndclass.cbWndExtra = NULL;
    wndclass.lpszClassName = szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.lpfnWndProc = MyCallBack;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hbrBackground = CreateSolidBrush(RGB(0,0,0));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassEx(&wndclass);

    hwnd = CreateWindowEx(WS_EX_WINDOWEDGE,
        szAppName,
        TEXT("Solar System"),
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
    else if(iRet == -5) {
        fprintf(fptr, "One OF LoadTexture Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else {
        fprintf(fptr,"Initialization Successful!!!...\n");
    }

    ToggleFullScreen();
    ShowWindow(hwnd,iCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    //game loop
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
    void uninitialize(void);
    void ToggleFullScreen(void);
    void resize(int, int);

    //var
    bool bIsMax = false;

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
                
                case 's':
                case 'S':
                    if(!bIsMax) {
                        ShowWindow(hwnd, SW_MAXIMIZE);
                        bIsMax = true;
                    }
                    else {
                        ShowWindow(hwnd, SW_SHOWNORMAL);
                        bIsMax = false;
                    }
                break;

                case 'f':
                case 'F':
                    ToggleFullScreen();
                    break;
            }
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

    //var & func
    void resize(int, int);
    BOOL loadTexture(GLuint*, TCHAR[]);

    int iPixelFomatIndex = 0;
    PIXELFORMATDESCRIPTOR pfd;

    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize =sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;        

    ghdc = GetDC(ghwnd);

    iPixelFomatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFomatIndex == 0) {
        return(-1);
    }
    else {
        fprintf(fptr,"ChoosePixelFormat Successful!!..\t(Index = %d)\n",iPixelFomatIndex);
    }

    if(SetPixelFormat(ghdc, iPixelFomatIndex, &pfd) == FALSE) {
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
  
    glClearColor(0.0f,0.0f,0.0f,1.0f);
  
    glShadeModel(GL_SMOOTH);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    MySun = gluNewQuadric();
    gluQuadricTexture(MySun, TRUE);

    MyEarth = gluNewQuadric();
    gluQuadricTexture(MyEarth, TRUE);
/*  
    MyMoon = gluNewQuadric();
    gluQuadricTexture(MyMoon, TRUE);
 */ 
    glEnable(GL_TEXTURE_2D);

    if(loadTexture(&iGalaxyTex, MAKEINTRESOURCE(ID_GALAXY)) == FALSE) {
        return(-5);
    }
    else {
        fprintf(fptr, "Galaxy Texture LoadedSuccessfully!!..\n");
    }

    if(loadTexture(&iSunTex, MAKEINTRESOURCE(ID_SUN)) == FALSE) {
        return(-5);
    }
    else {
        fprintf(fptr, "Sun Texture Loaded Successfully!!..\n");
    }

    if(loadTexture(&iEarthTex, MAKEINTRESOURCE(ID_EARTH)) == FALSE) {
        return(-5);
    }
    else {
        fprintf(fptr, "Earth texture Loaded Successfully!!..\n");
    }
/*
    if(loadTexture(&iMoonTex, MAKEINTRESOURCE(ID_MOON)) == FALSE) {
        return(-5);
    }
    else {
        fprintf(fptr, "Moon Texture Loaded Successfully!!..\n");
    }
 */
    resize(WIN_WIDTH, WIN_HEIGHT);

    return(0);
}

BOOL loadTexture(GLuint *texture, TCHAR imgResID[]) {

    HBITMAP hBitmap;
    BITMAP bmp;
    BOOL bStatus = FALSE;

    //Code
    hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL),
        imgResID,
        IMAGE_BITMAP,
        0, 0,
        LR_CREATEDIBSECTION);

    if(hBitmap) {
        bStatus = TRUE;

        GetObject(hBitmap, sizeof(BITMAP), &bmp);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        glGenTextures(1, texture);

        glBindTexture(GL_TEXTURE_2D, *texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
            bmp.bmWidth, bmp.bmHeight,
            GL_BGR_EXT, GL_UNSIGNED_BYTE,
            bmp.bmBits);

        DeleteObject(hBitmap);
    }

    return(bStatus);
}

void resize(int width, int height) {

    if(height <= 0) {
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
    
    static bool bStartSong = false;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(!bStartSong) {
        PlaySound(MAKEINTRESOURCE(ID_SONG), NULL, SND_ASYNC | SND_NODEFAULT | SND_LOOP | SND_RESOURCE);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    gluLookAt(0.0f, 0.0f, fLookVal, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    //gluLookAt(0.0f, 0.0f, 6.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, iGalaxyTex);
    glBegin(GL_QUADS);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(5.2f, 3.0f, -2.0f);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-5.2f, 3.0f, -2.0f);
    
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-5.2f, -3.0f, -2.0f);
    
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(5.2f, -3.0f, -2.0f);
    glEnd();
    
    glPopMatrix();
    glPushMatrix(); 

    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); 
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); 

    glBindTexture(GL_TEXTURE_2D, iSunTex);
    gluSphere(MySun, 0.9f, 30, 30);

    glPopMatrix(); 
    glPushMatrix(); 

    glRotatef((GLfloat)fYear, 0.0f, 1.0f, 0.0f); 
    glTranslatef(1.9f, 0.0f, 0.0f);
    
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f); 
    glRotatef((GLfloat)fDay, 0.0f, 0.0f, 1.0f); 
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindTexture(GL_TEXTURE_2D, iEarthTex);
    gluSphere(MyEarth, 0.3f, 40, 40);
/*     glPushMatrix();

    glRotatef(fMoonRot, 0.0f, 1.0f, 0.0f);
    glTranslatef(0.5f, 0.0f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindTexture(GL_TEXTURE_2D, iMoonTex);
    gluSphere(MyMoon, 0.12f, 30, 30);

    glPopMatrix(); 
 */    glPopMatrix();

    SwapBuffers(ghdc);
    bStartSong = true;
}

void update(void) {

    fLookVal += 0.0003f;
    if(fLookVal >= 5.0f) {
        fLookVal = 5.0f;
        bRenderEarth = true;
    }

    fDay += 0.09f;
    if(fDay >= 360.0f) {
        fDay = 0.0f;
    }
    fYear += 0.009f;
    if(fYear >= 360.0f) {
        fYear = 0.0f;
    }
/*
    fMoonRot += 0.1f;
    if(fMoonRot >= 360.0f) {
        fMoonRot = 0.0f;
    } */
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

    if(MySun) {
        gluDeleteQuadric(MySun);
        MySun = NULL;
    }

    if(MyEarth) {
        gluDeleteQuadric(MyEarth);
        MyEarth = NULL;
    }
/* 
    if(MyMoon){
        gluDeleteQuadric(MyMoon);
        MyMoon = NULL;
    }
 */
    if(iEarthTex) {
        glDeleteTextures(1, &iEarthTex);
        iEarthTex = 0;
    }

    if(iSunTex) {
        glDeleteTextures(1, &iSunTex);
        iSunTex = 0;
    }
/* 
    if(iMoonTex) {
        glDeleteTextures(1, &iMoonTex);
        iMoonTex = 0;
    }
 */
    if(iGalaxyTex) {
        glDeleteTextures(1, &iGalaxyTex);
        iGalaxyTex = 0;
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
