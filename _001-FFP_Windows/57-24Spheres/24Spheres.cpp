#include<windows.h>
#include<stdio.h>
#include<gl/GL.h>
#include<gl/GLU.h>

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

GLfloat lightAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f }; 

GLfloat light_model_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat light_model_local_viewer[] = { 0.0f };

GLUquadric *Spheres[24];

GLfloat angleOfXRot = 0.0f;
GLfloat angleOfYRot = 0.0f;
GLfloat angleOfZRot = 0.0f;

GLint KeyPressed = 0;

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
    TCHAR szAppName[] = TEXT("3D-Ortho");
    bool bDone = false;
    int iRet = 0;

    if(fopen_s(&fptr,"_24SpheresLog.txt","w") != 0) {
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
        TEXT(" 24 Spheres "),
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
    static bool bIsMax = false;

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

                case 'X':
                case 'x':
                    KeyPressed = 1;
                    angleOfXRot = 0.0f;
                    break;
                
                case 'Y':
                case 'y':
                    KeyPressed = 2;
                    angleOfYRot = 0.0f;
                    break;

                case 'Z':
                case 'z':
                    KeyPressed = 3;
                    angleOfZRot = 0.0f;
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

    glEnable(GL_AUTO_NORMAL);   //TO Let OGL calculate and Take Care Of Normals
    glEnable(GL_NORMALIZE);     // To Rescale & rESIZE nOMALS Mathematically.

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0,  GL_POSITION, lightPosition);
    
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, light_model_local_viewer);
    glEnable(GL_LIGHT0);



    for(int i = 0; i < 24; i++) {
        Spheres[i] = gluNewQuadric();
    }

    glClearColor(0.25f,0.25f,0.25f,1.0f);
 
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

    if(width < height) {
        glOrtho(0.0f,
            15.5f,
            0.0f,
            15.5f * (GLfloat)height/(GLfloat)width,
            -10.0f,
            10.0f);
    }
    else {
        glOrtho(0.0f,
            15.5f * (GLfloat)width/(GLfloat)height,
            0.0f,
            15.5f,
            -10.0f,
            10.0f);
    }
}

void display(void) {

    void draw24Spheres(void);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if(KeyPressed == 1) {
        glRotatef(angleOfXRot, 1.0f, 0.0f, 0.0f);
        lightPosition[1] = angleOfXRot;
    }
    else if(KeyPressed == 2) {
        glRotatef(angleOfYRot, 0.0f, 1.0f, 0.0f);
        lightPosition[2] = angleOfYRot;
    }
    else if(KeyPressed == 3) {
        glRotatef(angleOfZRot, 0.0f, 0.0f, 1.0f);
        lightPosition[0] = angleOfZRot;
    }
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    draw24Spheres();

    SwapBuffers(ghdc);

}

void draw24Spheres(void) {

    //Local to all
    GLfloat matAmb[4];
    GLfloat matDiff[4];
    GLfloat matSpec[4];
    GLfloat matShine;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //for Spere 1 in col 1 : Emerald
    matAmb[0] = 0.0215f;
    matAmb[1] = 0.1745;
    matAmb[2] = 0.0215;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.07568f;
    matDiff[1] = 0.61424f;
    matDiff[2] = 0.07568f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.633f;
    matSpec[1] = 0.727811f;
    matSpec[2] = 0.633f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.6f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.5f, 14.0f, 0.0f);
    gluSphere(Spheres[0], 1.0f, 30, 30);


    //for Spere 2 in col 1 : Jade
    matAmb[0] = 0.135f;
    matAmb[1] = 0.2225f;
    matAmb[2] = 0.1575f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.54f;
    matDiff[1] = 0.89f;
    matDiff[2] = 0.63f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.316228f;
    matSpec[1] = 0.316228f;
    matSpec[2] = 0.316228f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.1f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.5f, 11.5f, 0.0f);
    gluSphere(Spheres[1], 1.0f, 30, 30);
    
    
    //for Spere 3 in col 1 : Obsidian
    matAmb[0] = 0.05375f;
    matAmb[1] = 0.05f;
    matAmb[2] = 0.06652f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.18275f;
    matDiff[1] = 0.17f;
    matDiff[2] = 0.22525f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.332741f;
    matSpec[1] = 0.328634f;
    matSpec[2] = 0.346435f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.3f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.5f, 9.0f, 0.0f);
    gluSphere(Spheres[2], 1.0f, 30, 30);
    
    
    //for Spere 4 in col 1 : Pearl
    matAmb[0] = 0.25f;
    matAmb[1] = 0.20725f;
    matAmb[2] = 0.20725f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 1.0f;
    matDiff[1] = 0.829f;
    matDiff[2] = 0.829f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.296648f;
    matSpec[1] = 0.296648f;
    matSpec[2] = 0.296648f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.088f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.5f, 6.5f, 0.0f);
    gluSphere(Spheres[3], 1.0f, 30, 30);
    
    //for Spere 5 in col 1 : RUBY
    matAmb[0] = 0.1745f;
    matAmb[1] = 0.01175f;
    matAmb[2] = 0.01175f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.61424f;
    matDiff[1] = 0.04136f;
    matDiff[2] = 0.04136f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.727811f;
    matSpec[1] = 0.626959f;
    matSpec[2] = 0.626959f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.6f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.5f, 4.0f, 0.0f);
    gluSphere(Spheres[4], 1.0f, 30, 30);
    
    //for Spere 6 in col 1 : TurQuoise
    matAmb[0] = 0.1f;
    matAmb[1] = 0.18725f;
    matAmb[2] = 0.1754f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.396f;
    matDiff[1] = 0.74151f;
    matDiff[2] = 0.69102f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.297254f;
    matSpec[1] = 0.30829f;
    matSpec[2] = 0.306678f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.1f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.5f, 1.5f, 0.0f);
    gluSphere(Spheres[5], 1.0f, 30, 30);
    
    
    //for Spere 1 in col 2 : Brass
    matAmb[0] = 0.329412f;
    matAmb[1] = 0.223529f;
    matAmb[2] = 0.027451f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.780392f;
    matDiff[1] = 0.568627f;
    matDiff[2] = 0.113725f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.992157f;
    matSpec[1] = 0.941176f;
    matSpec[2] = 0.807843f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.21794872f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(11.0f, 14.0f, 0.0f);
    gluSphere(Spheres[6], 1.0f, 30, 30);
    
    //for Spere 2 in col 2 : Bronze
    matAmb[0] = 0.2125f;
    matAmb[1] = 0.1275f;
    matAmb[2] = 0.054f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.714f; 
    matDiff[1] = 0.4284f;
    matDiff[2] = 0.18144f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.393548f;
    matSpec[1] = 0.271906f;
    matSpec[2] = 0.166721f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.2f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(11.0f, 11.5f, 0.0f);
    gluSphere(Spheres[7], 1.0f, 30, 30);
    
    //for Spere 3 in col 2 : Chrome
    matAmb[0] = 0.25f;
    matAmb[1] = 0.25f;
    matAmb[2] = 0.25f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.4f;
    matDiff[1] = 0.4f;
    matDiff[2] = 0.4f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.774597f;
    matSpec[1] = 0.774597f;
    matSpec[2] = 0.774597f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.6f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(11.0f, 9.0f, 0.0f);
    gluSphere(Spheres[8], 1.0f, 30, 30);
    
    //for Spere 4 in col 2 : Copper
    matAmb[0] = 0.19125f;
    matAmb[1] = 0.0735f;
    matAmb[2] = 0.0225f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.7038f;
    matDiff[1] = 0.27048f;
    matDiff[2] = 0.0828f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.256777f;
    matSpec[1] = 0.137622f;
    matSpec[2] = 0.0828f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.1f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(11.0f, 6.5f, 0.0f);
    gluSphere(Spheres[9], 1.0f, 30, 30);
    
    //for Spere 5 in col 2 : Gold
    matAmb[0] = 0.24725f;
    matAmb[1] = 0.1995f;
    matAmb[2] = 0.0745f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.75164f;
    matDiff[1] = 0.60648f;
    matDiff[2] = 0.22648f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.628281f;
    matSpec[1] = 0.555802f;
    matSpec[2] = 0.366065f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.4f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(11.0f, 4.0f, 0.0f);
    gluSphere(Spheres[10], 1.0f, 30, 30);
    
    //for Spere 6 in col 2 : Silver
    matAmb[0] = 0.19225f;
    matAmb[1] = 0.19225f;
    matAmb[2] = 0.19225f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.50754f;
    matDiff[1] = 0.50754f;
    matDiff[2] = 0.50754f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.508273f;
    matSpec[1] = 0.508273f;
    matSpec[2] = 0.508273f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.4f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(11.0f, 1.5f, 0.0f);
    gluSphere(Spheres[11], 1.0f, 30, 30);
    
    
    //for Spere 1 in col 3 : Black Plastic
    matAmb[0] = 0.0f;
    matAmb[1] = 0.0f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.01f;
    matDiff[1] = 0.01f;
    matDiff[2] = 0.01f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.50f;
    matSpec[1] = 0.50f;
    matSpec[2] = 0.50f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(16.5f, 14.0f, 0.0f);
    gluSphere(Spheres[12], 1.0f, 30, 30);
    
    //for Spere 2 in col 3 : Cyan Plastic
    matAmb[0] = 0.0f;
    matAmb[1] = 0.1f;
    matAmb[2] = 0.06f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.0f;
    matDiff[1] = 0.50980392f;
    matDiff[2] = 0.50980392f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.50196078;
    matSpec[1] = 0.50196078;
    matSpec[2] = 0.50196078;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(16.5f, 11.5f, 0.0f);
    gluSphere(Spheres[13], 1.0f, 30, 30);
    
    //for Spere 3 in col 3 : Green Plastic
    matAmb[0] = 0.0f;
    matAmb[1] = 0.0f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.1f;
    matDiff[1] = 0.35f;
    matDiff[2] = 0.1f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.45f;
    matSpec[1] = 0.55f;
    matSpec[2] = 0.45f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(16.5f, 9.0f, 0.0f);
    gluSphere(Spheres[14], 1.0f, 30, 30);
    
    //for Spere 4 in col 3 : Red Plastic
    matAmb[0] = 0.0f;
    matAmb[1] = 0.0f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.5f;
    matDiff[1] = 0.0f;
    matDiff[2] = 0.0f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);;

    matSpec[0] = 0.7f;
    matSpec[1] = 0.6f;
    matSpec[2] = 0.6f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(16.5f, 6.5f, 0.0f);
    gluSphere(Spheres[15], 1.0f, 30, 30);
    
    //for Spere 5 in col 3 : White Plastic
    matAmb[0] = 0.0f;
    matAmb[1] = 0.0f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.55f;
    matDiff[1] = 0.55f;
    matDiff[2] = 0.55f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.70f;
    matSpec[1] = 0.70f;
    matSpec[2] = 0.70f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(16.5f, 4.0f, 0.0f);
    gluSphere(Spheres[16], 1.0f, 30, 30);
    
    //for Spere 6 in col 3 : Yellow Plastic
    matAmb[0] = 0.0f;
    matAmb[1] = 0.0f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.5f;
    matDiff[1] = 0.5f;
    matDiff[2] = 0.0f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.60f;
    matSpec[1] = 0.60f;
    matSpec[2] = 0.50f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(16.5f, 1.5f, 0.0f);
    gluSphere(Spheres[17], 1.0f, 30, 30);
    
    
    //for Spere 1 in col 4 : Black Rubber
    matAmb[0] = 0.02f;
    matAmb[1] = 0.02f;
    matAmb[2] = 0.02f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.01f;
    matDiff[1] = 0.01f;
    matDiff[2] = 0.01f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.4f;
    matSpec[1] = 0.4f;
    matSpec[2] = 0.4f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(22.0f, 14.0f, 0.0f);
    gluSphere(Spheres[18], 1.0f, 30, 30);
    
    //for Spere 2 in col 4 : CYAN Rubber
    matAmb[0] = 0.0f;
    matAmb[1] = 0.05f;
    matAmb[2] = 0.05f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.4f;
    matDiff[1] = 0.5f;
    matDiff[2] = 0.5f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.04f;
    matSpec[1] = 0.7f;
    matSpec[2] = 0.7f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(22.0f, 11.5f, 0.0f);
    gluSphere(Spheres[19], 1.0f, 30, 30);
    
    //for Spere 3 in col 4 : Green Rubber
    matAmb[0] = 0.0f;
    matAmb[1] = 0.05f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.4f;
    matDiff[1] = 0.5f;
    matDiff[2] = 0.4f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.04f;
    matSpec[1] = 0.7f;
    matSpec[2] = 0.04f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(22.0f, 9.0f, 0.0f);
    gluSphere(Spheres[20], 1.0f, 30, 30);
    
    //for Spere 4 in col 5 : Red Rubber
    matAmb[0] = 0.05f;
    matAmb[1] = 0.0f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.5f;
    matDiff[1] = 0.4f;
    matDiff[2] = 0.4f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.7f;
    matSpec[1] = 0.04f;
    matSpec[2] = 0.04f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(22.0f, 6.5f, 0.0f);
    gluSphere(Spheres[21], 1.0f, 30, 30);
    
    //for Spere 5 in col 4 : White Rubber
    matAmb[0] = 0.05f;
    matAmb[1] = 0.05f;
    matAmb[2] = 0.05f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.5f;
    matDiff[1] = 0.5f;
    matDiff[2] = 0.5f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.7f;
    matSpec[1] = 0.7f;
    matSpec[2] = 0.7f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(22.0f, 4.0f, 0.0f);
    gluSphere(Spheres[22], 1.0f, 30, 30);
    
    //for Spere 6 in col 4 : Yellow Rubber
    matAmb[0] = 0.05f;
    matAmb[1] = 0.05f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.5f;
    matDiff[1] = 0.5f;
    matDiff[2] = 0.4f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.7f;
    matSpec[1] = 0.7f;
    matSpec[2] = 0.04f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(22.0f, 1.5f, 0.0f);
    gluSphere(Spheres[23], 1.0f, 30, 30);
    

}

void update(void) {
    
    angleOfXRot += 0.09f;
    if(angleOfXRot >= 360.0f) {
        angleOfXRot = 0.0f;
    }

    angleOfYRot += 0.09f;
    if(angleOfYRot >= 360.0f) {
        angleOfYRot = 0.0f;
    }

    angleOfZRot += 0.09f;
    if(angleOfZRot >= 360.0f) {
        angleOfZRot = 0.0f;
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

    for(int i = 0; i < 24; i++) {
        gluDeleteQuadric(Spheres[i]);
        Spheres[i] = NULL;
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

