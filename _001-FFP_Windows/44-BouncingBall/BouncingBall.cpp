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

//Global Variables
HWND ghwnd = NULL;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
bool gbFullScreen = false;
DWORD dwStyle = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
bool gbActivateWindow = false;
FILE *fptr = NULL;

//Updation Params
GLfloat fxTrans = -2.45f;
GLfloat fyTrans = 1.1f;
GLfloat fSqueez = 1.0f;
GLfloat fTransSpeed = 0.0004;


LRESULT CALLBACK MyCallBack(HWND,UINT,WPARAM,LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreevInstance,LPSTR lpszCmdLine, int iCmdShow) {

    //var & Fun
    int initialize(void);
    void display(void);
    void ToggleFullScreen(void);
    void update(void);
    
    //Var
    WNDCLASSEX wndclass;
    MSG msg;
    TCHAR sszApp[] = TEXT("OGL_PRESP");
    HWND hwnd;
    bool bDone = false;
    int iRet = 0;
    int monWidth, monHeight;
	int xPar = 0;
	int yPar = 0;

    if(fopen_s(&fptr,"BBMaze.txt","w") != 0) {
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
        TEXT("Bouncing Ball"),
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

                case 'F':
				case 'f':
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

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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
		((GLfloat)width / (GLfloat)height),
		0.1f,
		100.0f);

}

void display(void) {

    //func del
    void drawBall(GLfloat);
    void drawPlane();

    glClear(GL_COLOR_BUFFER_BIT);
    

    drawPlane();

    drawBall(0.35f);


    SwapBuffers(ghdc);
}
/*     glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glVertex3f(1.27f, 0.6f, 0.0f);
    glEnd(); */

void drawBall(GLfloat fRadius) {

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glTranslatef(fxTrans, fyTrans, 0.0f);

    glScalef(fSqueez, fSqueez, fSqueez);
    glLineWidth(4.0f);

    glBegin(GL_LINE_LOOP);
    glColor3f(1.0f, 0.0f, 0.0f);
    for(GLfloat angle=0.0f; angle <= 2.0f*M_PI; angle += 0.01f) {
        
        GLfloat fXCord = fRadius * cosf(angle);
        GLfloat fYCord = fRadius * sinf(angle);

        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(fXCord, fYCord, 0.0f);
    }

    glEnd();

}

void drawPlane(void) {
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0f, 0.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glLineWidth(8.0f);
    glBegin(GL_LINES);

    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-3.0f, -1.3f, 0.0f);
    glVertex3f(3.0f, -1.3f, 0.0f);

    glEnd();
}

void update(void) {
    
    static bool bGroundReached = false, bOneBComp = false , bTwoBStart = false;

    if(!bGroundReached && !bOneBComp && !bTwoBStart) {
    
        if(fxTrans <= -0.35f) {
            fxTrans += fTransSpeed;
        }

        if(fyTrans >= -2.3f) {
            fyTrans -= fTransSpeed;
            if(fyTrans <= -0.95f) {
                fyTrans = -0.95f;
                bGroundReached = true;
            }
        }
        
        if(fyTrans <= -0.6f && fyTrans >= -1.3f) {
            fSqueez -= 0.00005;
        }
        

    }
    
    if(bGroundReached && !bOneBComp && !bTwoBStart) {

        if(fxTrans <= 1.0f){
            fxTrans += fTransSpeed;
            if(fxTrans >= 1.0f) {
                fxTrans = 1.0f;
                bOneBComp = true;
            }
        }
        if(fyTrans <= 0.5f){
            fyTrans += fTransSpeed;
            if(fyTrans >= 0.6f) {
                fxTrans = 0.6f;

            } 
        }
        
        if(fyTrans <= -0.6f && fyTrans >= -1.3f) {
            fSqueez += 0.00005;
        }
    }

    if(bGroundReached && bOneBComp && !bTwoBStart) {
        
        if(fxTrans >= 1.0f){
            fxTrans += fTransSpeed - 0.0001f;
            if(fxTrans >= 2.4f) {
                fxTrans = 2.4f;
                bTwoBStart = true;
            }

        }
        if(fyTrans <= 0.5f) {
            fyTrans -= fTransSpeed - 0.0001f;
            if(fyTrans <= -0.95f) {
                fyTrans = -0.95f;
            }
        }
        if(fyTrans <= -0.6f && fyTrans >= -1.3f) {
            fSqueez -= 0.00005;
        }
    }

    if( bGroundReached && bOneBComp && bTwoBStart) {
        fxTrans += fTransSpeed - 0.0002f;
        fyTrans += fTransSpeed - 0.0002f;
        
        if(fyTrans <= -0.6f && fyTrans >= -1.3f) {
            fSqueez += 0.00005;
        }
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