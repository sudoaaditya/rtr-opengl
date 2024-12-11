#include<windows.h>
#include<stdio.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include"Structure.h"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")

//global variables
bool gbFullScreen = false;
DWORD dwStyle = NULL;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
HWND ghwnd = NULL;
bool gbActiveWindow = false;
HDC ghdc= NULL;
HGLRC ghrc = NULL;
FILE *fptr = NULL;

//Lights Vars
bool bLights = false;
Light lights[3];

GLfloat MaterialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat MaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat MaterialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat MaterialShininess[] = { 128.0f }; 

GLUquadric *Sphere = NULL;


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

    if(fopen_s(&fptr,"_ThreeLightsSphereLog.txt","w") != 0) {
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

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
        szAppName,
        TEXT("Three Moving Lights on a Steady Sphere"),
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
    void FillLightZero(void);
    void FillLightOne(void);
    void FillLightTwo(void);

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

    glShadeModel(GL_SMOOTH);

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    FillLightOne();
    FillLightZero();
    FillLightTwo();

    for(int i = 0; i < 3; i++) {
        glLightfv(GL_LIGHT0 + i, GL_AMBIENT, lights[i].Ambient);
        glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, lights[i].Diffuse);
        glLightfv(GL_LIGHT0 + i, GL_SPECULAR, lights[i].Specular);
        glEnable(GL_LIGHT0 + i);
    }


    glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);
    
    glClearColor(0.0f,0.0f,0.0f,1.0f);
 
    resize(WIN_WIDTH, WIN_HEIGHT);

    return(0);
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

    void drawSphere(void);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();

    gluLookAt(0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    for(int i = 0; i < 3; i++) {
      //int i = 1;  
        //changing 0 = y, 1 = x, 2 = y;
        glPushMatrix();
        if(i == 0) {
            glRotatef(lights[i].RotAngle, 1.0f, 0.0f, 0.0f);
        }
        else if(i == 1) {
            glRotatef(lights[i].RotAngle, 0.0f, 1.0f, 0.0f);
        }
        else if(i == 2) {
            glRotatef(lights[i].RotAngle, 0.0f, 0.0f, 1.0f);
        }
        lights[i].Position[(3-i)%2] = lights[i].RotAngle;
        glLightfv(GL_LIGHT0 + i, GL_POSITION, lights[i].Position);
        glPopMatrix();
    }

    drawSphere();

    glPopMatrix();

    SwapBuffers(ghdc);
}

void drawSphere(void) {

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    Sphere = gluNewQuadric();

    gluSphere(Sphere, 0.7f, 100, 100);
    
}

void update(void) {

    for(int i = 0; i < 3; i++) {
        lights[i].RotAngle += 0.09f;
        if(lights[i].RotAngle >= 360.0f) {
            lights[i].RotAngle = 0.0f;
        }

    }
}

void FillLightZero(void) {

    lights[0].Ambient[0] = 0.0f;
    lights[0].Ambient[1] = 0.0f;
    lights[0].Ambient[2] = 0.0f;
    lights[0].Ambient[3] = 1.0f;

    lights[0].Diffuse[0] = 1.0f;
    lights[0].Diffuse[1] = 0.0f;
    lights[0].Diffuse[2] = 0.0f;
    lights[0].Diffuse[3] = 1.0f;

    lights[0].Specular[0] = 1.0f;
    lights[0].Specular[1] = 0.0f;
    lights[0].Specular[2] = 0.0f;
    lights[0].Specular[3] = 1.0f;

    lights[0].Position[0] = 0.0f;
    lights[0].Position[1] = 0.0f;
    lights[0].Position[2] = 0.0f;
    lights[0].Position[3] = 1.0f;

    lights[0].RotAngle = 0.0f;

}

void FillLightOne(void) {

    lights[1].Ambient[0] = 0.0f;
    lights[1].Ambient[1] = 0.0f;
    lights[1].Ambient[2] = 0.0f;
    lights[1].Ambient[3] = 1.0f;

    lights[1].Diffuse[0] = 0.0f;
    lights[1].Diffuse[1] = 1.0f;
    lights[1].Diffuse[2] = 0.0f;
    lights[1].Diffuse[3] = 1.0f;

    lights[1].Specular[0] = 0.0f;
    lights[1].Specular[1] = 1.0f;
    lights[1].Specular[2] = 0.0f;
    lights[1].Specular[3] = 1.0f;

    lights[1].Position[0] = 0.0f;
    lights[1].Position[1] = 0.0f;
    lights[1].Position[2] = 0.0f;
    lights[1].Position[3] = 1.0f;

    lights[1].RotAngle = 0.0f;
    
}

void FillLightTwo(void) {

    lights[2].Ambient[0] = 0.0f;
    lights[2].Ambient[1] = 0.0f;
    lights[2].Ambient[2] = 0.0f;
    lights[2].Ambient[3] = 1.0f;

    lights[2].Diffuse[0] = 0.0f;
    lights[2].Diffuse[1] = 0.0f;
    lights[2].Diffuse[2] = 1.0f;
    lights[2].Diffuse[3] = 1.0f;

    lights[2].Specular[0] = 0.0f;
    lights[2].Specular[1] = 0.0f;
    lights[2].Specular[2] = 1.0f;
    lights[2].Specular[3] = 1.0f;

    lights[2].Position[0] = 0.0f;
    lights[2].Position[1] = 0.0f;
    lights[2].Position[2] = 0.0f;
    lights[2].Position[3] = 1.0f;

    lights[2].RotAngle = 0.0f;
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

    if(Sphere) {
        gluDeleteQuadric(Sphere);
        Sphere = NULL;
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

