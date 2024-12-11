#include<windows.h>
#include<stdio.h>
#include<gl/GL.h>
#include<gl/glu.h>

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

//Arrays For Coord
GLfloat farrXCord[4];
GLfloat farrYCord[4];

//FUnc
LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);
void FillArray();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

    //Func
    int initialize(void);
    void display(void);
    void ToggleFullScreen(void);
    
    //func
    WNDCLASSEX wndclass;
    MSG msg;
    HWND hwnd;
    TCHAR szAppName[] = TEXT("2D-Presp");
    bool bDone = false;
    int iRet = 0;

    if(fopen_s(&fptr,"_ShapesLog.txt","w") != 0) {
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
        TEXT("Shapes"),
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

    //beautificationn calls 1 : tuz distortion in perspective talayla he function which will render smoot
    glShadeModel(GL_SMOOTH);

    //Beautification 2: Giving hint to Kuth Smoothing jast dyaychi
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    resize(WIN_WIDTH, WIN_HEIGHT);

    FillArray();

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

    //func
    void drawPoints(void);
    void drawBox(void);
    void drawGeo4(void);
    void drawGeo2(void);
    void drawGeo5(void);
    void drawFilledBox(void);

    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);

    drawPoints();

    drawGeo2();

    drawBox();

    drawGeo4();

    drawGeo5();

    drawFilledBox();

    SwapBuffers(ghdc);
}

void drawPoints() {

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-5.0f, 2.0f, -10.0f);

    glPointSize(3.0f);
    glBegin(GL_POINTS);

    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            glVertex2f(farrXCord[i], farrYCord[j]);
        }
    }
    glEnd();
}

void drawGeo2(void) {

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 2.0f, -10.0f);
    glLineWidth(3.0f);

    glBegin(GL_LINES);
    for(int i = 3; i > 0; i--) {    

        glVertex2f(farrXCord[0], farrYCord[i]);
        glVertex2f(farrXCord[3], farrYCord[i]);
        
        glVertex2f(farrXCord[3 - i], farrYCord[0]);
        glVertex2f(farrXCord[3 - i], farrYCord[3]);

    }    
        
    //slant One
    glVertex2f(farrXCord[1], farrYCord[3]);
    glVertex2f(farrXCord[0], farrYCord[2]);

    //slant two
    glVertex2f(farrXCord[3], farrYCord[2]);
    glVertex2f(farrXCord[1], farrYCord[0]);

    //slant Three
    glVertex2f(farrXCord[3], farrYCord[3]);
    glVertex2f(farrXCord[0], farrYCord[0]);

    //slant Four
    glVertex2f(farrXCord[2], farrYCord[3]);
    glVertex2f(farrXCord[0], farrYCord[1]);

    //slant Five
    glVertex2f(farrXCord[3], farrYCord[1]);
    glVertex2f(farrXCord[2], farrYCord[0]);
    
    glEnd();
}


void drawBox(void) {

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.0f, 2.0f, -10.0f);


    glLineWidth(3.0f);

    glBegin(GL_LINES);
   for(int i = 0; i < 4; i++) {    
        for(int j = 0; j < 4; j++) {
             glVertex2f(farrXCord[i], farrYCord[j]);
            glVertex2f(farrXCord[i+1], farrYCord[j]);

            glVertex2f(farrXCord[i], farrYCord[j]);
            glVertex2f(farrXCord[i], farrYCord[j+1]);  
        }
    }

    glEnd();
}

void drawGeo4(void) {

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-5.0f, -2.0f, -10.0f);
    glLineWidth(3.0f);

    glBegin(GL_LINES);
    for(int i = 0; i < 4; i++) {    
        for(int j = 0; j < 4; j++) {
            glVertex2f(farrXCord[i], farrYCord[j]);
            glVertex2f(farrXCord[i+1], farrYCord[j]);

            glVertex2f(farrXCord[i], farrYCord[j]);
            glVertex2f(farrXCord[i], farrYCord[j+1]);

        }
    }    
        
    //slant One
    glVertex2f(farrXCord[1], farrYCord[3]);
    glVertex2f(farrXCord[0], farrYCord[2]);

    //slant two
    glVertex2f(farrXCord[3], farrYCord[2]);
    glVertex2f(farrXCord[1], farrYCord[0]);

    //slant Three
    glVertex2f(farrXCord[3], farrYCord[3]);
    glVertex2f(farrXCord[0], farrYCord[0]);

    //slant Four
    glVertex2f(farrXCord[2], farrYCord[3]);
    glVertex2f(farrXCord[0], farrYCord[1]);

    //slant Five
    glVertex2f(farrXCord[3], farrYCord[1]);
    glVertex2f(farrXCord[2], farrYCord[0]);
    
    glEnd();
}

void drawGeo5(void) {

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, -2.0f, -10.0f);


    glLineWidth(3.0f);

    glBegin(GL_LINES);
    for(int i = 0; i < 4; i++) {           
        if(i == 0 || i == 3) {
            glVertex2f(farrXCord[0],farrYCord[i]);
            glVertex2f(farrXCord[3],farrYCord[i]);

            glVertex2f(farrXCord[i],farrYCord[3]);
            glVertex2f(farrXCord[i],farrYCord[0]);
        }
    }


    for(int i = 0; i < 6; i++) {
        if(i < 3) {
            glVertex2f(farrXCord[0], farrYCord[3]);
            glVertex2f(farrXCord[3], farrYCord[i]);

        }
        else {
            glVertex2f(farrXCord[0], farrYCord[3]);
            glVertex2f(farrXCord[6 - i], farrYCord[0]);

        }
    }

    glEnd();
}

void drawFilledBox(void) {
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.0f, -2.0f, -10.0f);

    glBegin(GL_QUADS);
    for(int i = 0; i < 3; i++) {

        if(i == 0){
            glColor3f(1.0f, 0.0f, 0.0f);
        }
        else if(i == 1){
            glColor3f(0.0f, 0.0f, 1.0f);
        }
        else if(i == 2){
            glColor3f(0.0f, 1.0f, 0.0f);
        }
        glVertex2f(farrXCord[i + 1], farrYCord[3]);
        glVertex2f(farrXCord[i], farrYCord[3]);
        glVertex2f(farrXCord[i], farrYCord[0]);
        glVertex2f(farrXCord[i + 1], farrYCord[0]);
    }
    glEnd();

    glLineWidth(3.0f);

    glBegin(GL_LINES);
   for(int i = 0; i < 4; i++) {    
       glColor3f(1.0f, 1.0f, 1.0f);
        for(int j = 0; j < 4; j++) {
            glVertex2f(farrXCord[i], farrYCord[j]);
            glVertex2f(farrXCord[i+1], farrYCord[j]);

            glVertex2f(farrXCord[i], farrYCord[j]);
            glVertex2f(farrXCord[i], farrYCord[j+1]);  
        }
    }
    glEnd();
}
void FillArray() {

    farrXCord[0] = -1.5f;
    farrXCord[1] = -0.5f;
    farrXCord[2] =  0.5f;
    farrXCord[3] =  1.5f;

    farrYCord[0] = -1.5f;
    farrYCord[1] = -0.5f;
    farrYCord[2] =  0.5f;
    farrYCord[3] =  1.5f;
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
