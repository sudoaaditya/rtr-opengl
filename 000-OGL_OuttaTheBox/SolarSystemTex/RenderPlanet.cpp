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

typedef class Planet Planet;

Planet *FirstPlanet = nullptr;
Planet *Travel = nullptr;


//global variables
bool gbFullScreen = false;
DWORD dwStyle = NULL;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
HWND ghwnd = NULL;
bool gbActiveWindow = false;
HDC ghdc= NULL;
HGLRC ghrc = NULL;
FILE *fptr = NULL;
PlanetQueue *Galaxy;

int iCallCnt = 0, itrCnt = 0;
GLuint iGalaxyTex = 0;
GLuint iSatRingsTex = 0;
GLUquadric *ringObj = NULL;
GLfloat fSatInRad = 0.0f, fSatOutRad = 0.0f;

//Rendering Scene pars
GLfloat fcamTrans = 1.0f;
GLfloat fcamTogVal = 0.0028f;
bool bSceneOne = false;
bool bSceneTwo = false;
bool bSceneThree = false;
bool bSceneFour = false;
bool bStartSceneFour = false;
bool bEndCredits = false;
bool bEndNow = false;
int iEndCnt = 0;

// Song
bool bStartSong = false;

//Let There Be LIGHT!
GLfloat lightAmbient[] = {0.7f, 0.7f, 0.7f, 1.0f};
GLfloat lightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat lightPosition[] = {0.0f, 1.0f, 1.0f, 1.0f};

//Font Parameters
unsigned int listBaseOne;
unsigned int listBaseTwo;
unsigned int listBaseThree;
GLfloat fColorArr[4];
GLfloat fAlphaVal1 = 0.0f;
GLfloat fAlphaVal2 = 0.0f;


LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

    //Func
    int initialize(void);
    void display(void);
    void update(void);
    void ToggleFullScreen(void);
    void CreatePlanetQueue(void);

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

    fflush(fptr);
    CreatePlanetQueue();
    fprintf(fptr, "After CreateQueue!!..\n\n");
    fflush(fptr);


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
    unsigned int CreateBitmapFont(char *, int, int);
    
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

    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    ringObj = gluNewQuadric();
    gluQuadricTexture(ringObj, GL_TRUE);


    glEnable(GL_TEXTURE_2D);

    if(loadTexture(&iGalaxyTex, MAKEINTRESOURCE(ID_GALAXY)) == FALSE) {
        return(-5);
    }
    else {
        fprintf(fptr,"Galaxy texture Loaded Successfully!!..\n");
    }

    if(loadTexture(&iSatRingsTex, MAKEINTRESOURCE(ID_SATRINGS)) == FALSE) {
        return(-5);
    }
    else {
        fprintf(fptr,"Saturn Rings texture Loaded Successfully!!..\n");
    }

    
    listBaseOne = CreateBitmapFont("Trebuchet MS", 60, 25);
    if(listBaseOne == 0) {
        return(-5);
    }
    else {
        fprintf(fptr, "Font One Created Successfully!\n");
    }

    listBaseTwo = CreateBitmapFont("Trebuchet MS", 50, 21);
    if(listBaseTwo == 0) {
        return(-5);
    }
    else {
        fprintf(fptr, "Font Two Created Successfully!\n");
    }

    listBaseThree = CreateBitmapFont("Calibri ", 33, 15);
    if(listBaseThree == 0) {
        return(-5);
    }
    else {
        fprintf(fptr, "Font Three Created Successfully!\n");
    }


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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        gluBuild2DMipmaps(GL_TEXTURE_2D, 3,
            bmp.bmWidth, bmp.bmHeight,
            GL_BGR_EXT, GL_UNSIGNED_BYTE,
            bmp.bmBits);

        DeleteObject(hBitmap);
    }

    return(bStatus);
}


unsigned int CreateBitmapFont(char *fontName, int fontWd, int fontHt) {
    
    HFONT hMyFont;
    unsigned int fontBase;

    fontBase = glGenLists(96);

		hMyFont = CreateFont(fontWd, fontHt, 0, 0, FW_BOLD,
			NULL, NULL, NULL, NULL,
			NULL, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
			TEXT("Trebuchet MS"));


    if(!hMyFont){
        return(0);
    }

    SelectObject(ghdc, hMyFont);
    wglUseFontBitmaps(ghdc, 32, 96, fontBase);

    return(fontBase);

}

void RenderFont(GLfloat posX, GLfloat posY, GLfloat posZ, unsigned int listBase, char *str, GLfloat *colArr) {

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    if(listBaseOne == 0 || (!str)) {
        return;
    }

    glColor4f(colArr[0], colArr[1], colArr[2], colArr[3]);
    glRasterPos3f(posX, posY, posZ);

    glPushAttrib(GL_LIST_BIT);
        glListBase(listBase - 32);
        glCallLists((int)strlen(str), GL_UNSIGNED_BYTE, str);
    glPopAttrib();

    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}


void resize(int width, int height) {

    if(height <= 0) {
        height = 1;
    }

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(420.0f, 
        (GLfloat)width / (GLfloat)height,
        0.1f,
        100.0f);

}

void display(void) {

    void printString(char *s);
    void drawGalaxy();
    void renderText();
    void renderSceneThree();
    void renderSceneTwo();
    void renderSceneOne();
    void renderSceneFour();
    void renderFinals();
    
    static int iFrameRate = 0;
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if(bStartSong) {
        PlaySound(MAKEINTRESOURCE(ID_MUSIC), NULL, SND_ASYNC | SND_NODEFAULT | SND_LOOP | SND_RESOURCE);
        bStartSong = false;
    }

    if(!bSceneOne){
        gluLookAt(0.0f, 0.0f, fcamTrans, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        renderSceneOne();
    }
    if(bSceneOne && !bSceneTwo) {
        gluLookAt(0.0f, fcamTrans, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        renderSceneTwo();
    }
    if(bSceneOne && bSceneTwo && !bSceneThree) {
        gluLookAt(0.0f, fcamTrans, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
        renderSceneThree();
    }
    if(bSceneOne && bSceneTwo && bSceneThree && bStartSceneFour && !bEndCredits) {
        gluLookAt(0.0f, 0.0f, fcamTrans, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        renderSceneFour();   
        if(bSceneFour) {
            renderText();
        }     
    }
/* 
    //gluLookAt(-12.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    gluLookAt(0.0f, 0.0f, 12.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    if(!bEndCredits) {
        //renderSceneFour();
        //renderText();
        bEndCredits = true;
    } */
    
    if(!bEndCredits) {
        drawGalaxy();
    }

    if(!bStartSceneFour && !bEndCredits) {
        glPushMatrix();
        Galaxy->Render();
        glPopMatrix();
    }
    else if(bStartSceneFour && !bEndCredits) {
        glPushMatrix();
        Galaxy->RenderFour();
        glPopMatrix();
    }
    else if(bEndCredits){
        gluLookAt(0.0f, 0.0f, 12.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        renderFinals();
    }
    
    if(bEndNow == true) {
        iEndCnt++;
        if(iEndCnt >= 400) {
            iEndCnt = 400;
            DestroyWindow(ghwnd);
        }
    }
    SwapBuffers(ghdc);

}

void drawGalaxy(void) {

    glDisable(GL_LIGHTING);
    GLUquadric *myGal = gluNewQuadric();
    static GLfloat fGalRot = 0.0f;
    gluQuadricTexture(myGal, GL_TRUE);
    
    glPushMatrix();
    //glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
    glRotatef(fGalRot, 1.0f, 1.0f, 1.0f);
    fGalRot += 0.034567f;
    if(fGalRot >= 360.0f) {
        fGalRot = 0.0f;
    }
    glBindTexture(GL_TEXTURE_2D, iGalaxyTex);

    gluSphere(myGal, 12.0f, 100, 100);

    
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void update(void) {
    Galaxy->UpdatePlanetPos();

    if(itrCnt >= 160) {
        fAlphaVal1 += 0.006f;
        if(fAlphaVal1 >= 1.0f) {
            fAlphaVal1  = 1.0f;
        }
    }
    if(bEndCredits == true) {
        fAlphaVal2 += 0.0006f;
        if(fAlphaVal2 >= 1.0f) {
            fAlphaVal2 = 1.0f;
        }
    }
}

void renderSceneOne() {

    static int flag = 0;
    if(fcamTrans <= 11.0 && flag == 0)  {
        fcamTrans += fcamTogVal;;
        if(fcamTrans >= 11.0f)
            flag = 1;
    }
    if(flag == 1) {
        fcamTrans -= fcamTogVal;;
        if(fcamTrans <= 1.0f)
            bSceneOne = true;
    }
}

void renderSceneTwo() {

    fcamTrans += fcamTogVal + 0.0002;
    if(fcamTrans >= 11.0f) {
        fcamTrans = 11.0f;
        bSceneTwo = true;
    }
}

void renderSceneThree() {

    static int icnt = 0;

    if(FirstPlanet == nullptr) {
        return;
    }
    else {
        Travel = FirstPlanet;

        while(Travel != nullptr) {

            if(strcmp(Travel->m_PlanetName, "sun") != 0) {
                Travel->m_fRotationAlongSun += 0.9f;
                Travel->m_fRevolutions += 0.7f;
            }

            Travel = Travel->m_NextPlanet;
        }
    }
    icnt++;
    if(icnt >= 400){
        icnt = 400;
    }


    if(icnt == 400){
        fcamTrans -= fcamTogVal + 0.002;
        if(fcamTrans <= 1.0f){
            fcamTrans = 1.0f;
            bSceneThree = true;
            bStartSceneFour = true;
        }
    }

}

void renderText() {
    
    void RenderFont(GLfloat, GLfloat, GLfloat, unsigned int, char*, GLfloat*);
    void drawLine(GLfloat* , GLfloat*, GLfloat*, GLfloat);

    // COM : 50 135 168
    fColorArr[0] = 0.196f;
    fColorArr[1] = 0.529f;
    fColorArr[2] = 0.658f;
    fColorArr[3] = 1.0f;
    if (itrCnt >= 60) {
        RenderFont(-2.35f, -0.1f, 6.0f, listBaseOne, "You Live Here.", fColorArr);
    }
    if(itrCnt >= 100){
        GLfloat lineWidth = 4.45f;
        GLfloat posOne[] = {-2.2f, -0.2f, 6.0f};
        GLfloat posTwo[] = {-2.2f, -1.65f, 6.0f};

        drawLine(posOne, posTwo, fColorArr, lineWidth);

        GLfloat arPosOne[] = {-2.1f, -1.45f, 6.0f};
        GLfloat arPosTwo[] = {-2.2f, -1.65f, 6.0f};

        drawLine(arPosOne, arPosTwo, fColorArr, lineWidth);
        
        GLfloat arrPosOne[] = {-2.3f, -1.45f, 6.0f};
        GLfloat arrPosTwo[] = {-2.2f, -1.65f, 6.0f};

        drawLine(arrPosOne, arrPosTwo, fColorArr, lineWidth);
    }
    if( itrCnt >= 160) { 
        fColorArr[3] = fAlphaVal1;   
        RenderFont(-5.5f, -2.8f, 6.0f, listBaseTwo, "& Still Most of the People on This Planet Are Able to Carry EGO, ", fColorArr);
        RenderFont(-5.5f, -3.3f, 6.0f, listBaseTwo, "BIGGER Than This Entire UNIVERSE!!. Ironic, isn't it? ", fColorArr);
    }

    if(itrCnt <= 400) {
        itrCnt++;
    }
    else {
        itrCnt = 401;
        bEndCredits = true;
    }
/*  */
}

void drawLine(GLfloat *fposOne, GLfloat *fposTwo, GLfloat *fcolVec, GLfloat fLineWidth) {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    glLineWidth(fLineWidth);
    glColor3f(fcolVec[0], fcolVec[1], fcolVec[2]);
    glBegin(GL_LINES);
    glVertex3f(fposOne[0], fposOne[1], fposOne[2]);
    glVertex3f(fposTwo[0], fposTwo[1], fposTwo[2]);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}


void  renderFinals() {
    static int endCntr = 0;
    void RenderFont(GLfloat, GLfloat, GLfloat, unsigned int, char*, GLfloat*);

    fColorArr[0] = 0.0f;
    fColorArr[1] = 1.0f;
    fColorArr[2] = 0.0f;
    fColorArr[3] = fAlphaVal2;
    
    RenderFont(-10.0f, 0.5f, 0.0f, listBaseOne, "Our Fate Is Governed By The Choices We Make!.", fColorArr);
    RenderFont(4.0f, -0.9f, 0.0f, listBaseThree, "- Deepian.", fColorArr);
    RenderFont(-1.5f, -4.0f, 0.0f, listBaseThree, " Thank You!", fColorArr);

    endCntr++;
    if(endCntr > 1200 && fAlphaVal2 == 1.0f) {
        endCntr = 1200;
        bEndNow = true;
    }
}


void renderSceneFour(void) {

    static bool bEnter = false;

    if(!bEnter){        
        if(FirstPlanet == nullptr) {
            return;
        }
        else {
            Travel = FirstPlanet;

            while(Travel != nullptr) {

                if(strcmp(Travel->m_PlanetName, "sun") != 0) { 
                    Travel->m_fRotationAlongSun += 0.0f;
                    Travel->m_fRevolutions = 0.0f;
                    Travel->m_RevUpVal = 0.0f;
                    Travel->m_RotUpVal = 0.0f;
                }
                if(strcmp(Travel->m_PlanetName, "sun") == 0) {
                    Travel->m_fPlanetRad = 7.5f;
                    Travel->m_fTransX = 0.0f;
                    Travel->m_fTransY = 1.0f;
                    Travel->m_fTransZ = -7.0f;
                }
                else if(strcmp(Travel->m_PlanetName, "mercury") == 0) {
                    Travel->m_fPlanetRad = 0.1f;
                    Travel->m_fTransX = -4.4f;
                    Travel->m_fTransY = -3.0f;
                    Travel->m_fTransZ = 3.0f;
                }
                else if(strcmp(Travel->m_PlanetName, "venus") == 0) {
                    Travel->m_fPlanetRad = 0.13f;
                    Travel->m_fTransX = -4.0f;
                    Travel->m_fTransY = -3.0f;
                    Travel->m_fTransZ = 3.0f;
                }
                else if(strcmp(Travel->m_PlanetName, "earth") == 0) {
                    Travel->m_fPlanetRad = 0.3f;
                    Travel->m_fTransX = -3.3f;
                    Travel->m_fTransY = -3.0f;
                    Travel->m_fTransZ = 3.0f;
                }
                else if(strcmp(Travel->m_PlanetName, "mars") == 0) {
                    Travel->m_fPlanetRad = 0.28f;
                    Travel->m_fTransX = -2.5f;
                    Travel->m_fTransY = -3.0f;
                    Travel->m_fTransZ = 3.0f;
                } 
                else if(strcmp(Travel->m_PlanetName, "jupiter") == 0) {
                    Travel->m_fPlanetRad = 0.9f;
                    Travel->m_fTransX = -1.0f;
                    Travel->m_fTransY = -2.6f;
                    Travel->m_fTransZ = 3.0f;
                }
                else if(strcmp(Travel->m_PlanetName, "saturn") == 0) {
                    Travel->m_fPlanetRad = 0.87f;
                    Travel->m_fTransX = 1.3f;
                    Travel->m_fTransY = -2.6f;
                    Travel->m_fTransZ = 3.0f;
                    Travel->m_RingRadArr[0] = 1.02f;
                    Travel->m_RingRadArr[1] = 1.35f;
                }
                else if(strcmp(Travel->m_PlanetName, "uranus") == 0) {
                    Travel->m_fPlanetRad = 0.6f;
                    Travel->m_fTransX = 3.38f;
                    Travel->m_fTransY = -2.7f;
                    Travel->m_fTransZ = 3.0f;
                }
                else if(strcmp(Travel->m_PlanetName, "neptune") == 0) {
                    Travel->m_fPlanetRad = 0.5f;
                    Travel->m_fTransX = 4.8f;
                    Travel->m_fTransY = -2.7f;
                    Travel->m_fTransZ = 3.0f;
                }
                Travel = Travel->m_NextPlanet;
            }
        }
    }else {
        fcamTrans += fcamTogVal + 0.003;
        if(fcamTrans >= 12.0f ){
            fcamTrans = 12.0f;
            bSceneFour = true;
        }
    }
    bEnter = true;
} 

PlanetQueue::PlanetQueue() {

    fprintf(fptr,"Entering Constructor : %d\n",iCallCnt);
    fprintf(fptr, ".......................................................\n");
    iCallCnt++;

}

PlanetQueue::~PlanetQueue() {
    
    fprintf(fptr,"Entering Destructor : %d\n",iCallCnt);
    
    if(FirstPlanet == nullptr) {
        return;
    }
    else {
        while(FirstPlanet != nullptr)
            DequeuePlanet();
    }
}

void PlanetQueue::EnqueuePlanet(const char* PlanName) {

    fprintf(fptr,"Entering EnQueue : %d\n",iCallCnt);


    Planet *newPlanetObj = new Planet();

    InsertData(newPlanetObj, PlanName);

    if(FirstPlanet == nullptr) {

        fprintf(fptr,"....................................EnQueue If Called\n");

        FirstPlanet = newPlanetObj;

    }
    else {

        fprintf(fptr,"....................................EnQueue else Called\n");
        Travel = FirstPlanet;

        while(Travel->m_NextPlanet != nullptr) {
            Travel = Travel->m_NextPlanet;
        }
        Travel->m_NextPlanet = newPlanetObj;
    }
    
    fprintf(fptr,"Exiting EnQueue : %d\n",iCallCnt);
    iCallCnt++;
    return;

}

void PlanetQueue::DequeuePlanet() {

    fprintf(fptr,"Entering DeQueue : %d\n",iCallCnt);

    if(FirstPlanet == nullptr) {
        return;
    }
    else {
        fprintf(fptr, "Destroying Planet!!..\n");
        Planet *TempObj = FirstPlanet   ;
        FirstPlanet = FirstPlanet->m_NextPlanet;
        delete(TempObj);
    }

    fprintf(fptr,"Exiting DeQueue : %d\n",iCallCnt);
    iCallCnt--;
}

void PlanetQueue::InsertData(Planet *&Planet, const char* PlanName) {

    fprintf(fptr,"Enetering InsertData : %d, %s\n",iCallCnt, PlanName);

    BOOL loadTexture(GLuint*, TCHAR[]);

    if(strcmp(PlanName, "sun") == 0) {
        fprintf(fptr,"Sun Called!!..\n");

        Planet->m_PlanetName = PlanName;
        Planet->m_fTransFromSun = 0.0f;
        Planet->m_fRotationAlongSun = 0.0f;
        Planet->m_fRevolutions = 0.0f;
        Planet->m_quadPlnetObj = gluNewQuadric();
        Planet->m_fPlanetRad = 0.9f;
        if(loadTexture(&Planet->m_iPlanetTex, MAKEINTRESOURCE(ID_SUN)) == FALSE) {
            fprintf(fptr, "Texture Loading Failed!!..\n");
        }
        else {
            fprintf(fptr, "Sun Texture Loaded!!..\n");  
        }
    }
    else if(strcmp(PlanName, "mercury") == 0) {
        fprintf(fptr, "Mercury Called!!..\n");

        Planet->m_PlanetName = PlanName;
        Planet->m_fTransFromSun = 1.4f;
        Planet->m_fRotationAlongSun = 0.0f;
        Planet->m_fRevolutions = 0.0f;
        Planet->m_quadPlnetObj = gluNewQuadric();
        Planet->m_fPlanetRad = 0.2f;
        Planet->m_RotUpVal = 0.035f;
        Planet->m_RevUpVal = 0.08f;
        if(loadTexture(&Planet->m_iPlanetTex, MAKEINTRESOURCE(ID_MERCURY)) == FALSE) {
            fprintf(fptr, "Texture Loading Failed!!..\n");
        }
        else {
            fprintf(fptr, "Mercury Texture Loaded!!..\n");  
        }
    }
    else if(strcmp(PlanName, "venus") == 0) {
        fprintf(fptr, "Venus Called!!..\n");

        Planet->m_PlanetName = PlanName;
        Planet->m_fTransFromSun = 2.0f;
        Planet->m_fRotationAlongSun = 0.0f;
        Planet->m_fRevolutions = 0.0f;
        Planet->m_quadPlnetObj = gluNewQuadric();
        Planet->m_fPlanetRad = 0.2f;
        Planet->m_RotUpVal = 0.032f;
        Planet->m_RevUpVal = 0.1f;
        if(loadTexture(&Planet->m_iPlanetTex, MAKEINTRESOURCE(ID_VENUS)) == FALSE) {
            fprintf(fptr, "Texture Loading Failed!!..\n");
        }
        else {
            fprintf(fptr, "Venus Texture Loaded!!..\n");  
        }
    }
    else if(strcmp(PlanName, "earth") == 0) {
        fprintf(fptr,"Earth Called!!..\n");

        Planet->m_PlanetName = PlanName;
        Planet->m_fTransFromSun = 2.7f;
        Planet->m_fRotationAlongSun = 0.0f;
        Planet->m_fRevolutions = 0.0f;
        Planet->m_quadPlnetObj = gluNewQuadric();
        Planet->m_fPlanetRad = 0.3f;
        Planet->m_RotUpVal = 0.04f;
        Planet->m_RevUpVal = 0.5f;
        if(loadTexture(&Planet->m_iPlanetTex, MAKEINTRESOURCE(ID_EARTH)) == FALSE) {
            fprintf(fptr, "Texture Loading Failed!!..\n");
        }
        else {
            fprintf(fptr, "Earth Texture Loaded!!..\n");  
        }
    }
    else if(strcmp(PlanName, "mars") == 0) {
        fprintf(fptr, "Mars Called!!..\n");

        Planet->m_PlanetName = PlanName;
        Planet->m_fTransFromSun = 3.4f;
        Planet->m_fRotationAlongSun = 0.0f;
        Planet->m_fRevolutions = 0.0f;
        Planet->m_quadPlnetObj = gluNewQuadric();
        Planet->m_fPlanetRad = 0.22f;
        Planet->m_RotUpVal = 0.05f;
        Planet->m_RevUpVal = 0.46f;
        if(loadTexture(&Planet->m_iPlanetTex, MAKEINTRESOURCE(ID_MARS)) == FALSE) {
            fprintf(fptr, "Texture Loading Failed!!..\n");
        }
        else {
            fprintf(fptr, "Mars Texture Loaded!!..\n");  
        }
    }
    else if(strcmp(PlanName, "jupiter") == 0) {
        fprintf(fptr, "Jupiter Called!!..\n");

        Planet->m_PlanetName = PlanName;
        Planet->m_fTransFromSun = 4.4f;
        Planet->m_fRotationAlongSun = 0.0f;
        Planet->m_fRevolutions = 0.0f;
        Planet->m_quadPlnetObj = gluNewQuadric();
        Planet->m_fPlanetRad = 0.55f;
        Planet->m_RotUpVal = 0.08f;
        Planet->m_RevUpVal = 0.78f;
        if(loadTexture(&Planet->m_iPlanetTex, MAKEINTRESOURCE(ID_JUPITER)) == FALSE) {
            fprintf(fptr, "Texture Loading Failed!!..\n");
        }
        else {
            fprintf(fptr, "Jupiter Texture Loaded!!..\n");  
        }
    }
    else if(strcmp(PlanName, "saturn") == 0) {
        fprintf(fptr, "Saturn Called!!..\n");

        Planet->m_PlanetName = PlanName;
        Planet->m_fTransFromSun = 5.9f;
        Planet->m_fRotationAlongSun = 0.0f;
        Planet->m_fRevolutions = 0.0f;
        Planet->m_quadPlnetObj = gluNewQuadric();
        Planet->m_fPlanetRad = 0.5f;
        Planet->m_RotUpVal = 0.07f;
        Planet->m_RevUpVal = 0.60f;
        if(loadTexture(&Planet->m_iPlanetTex, MAKEINTRESOURCE(ID_SATURN)) == FALSE) {
            fprintf(fptr, "Texture Loading Failed!!..\n");
        }
        else {
            fprintf(fptr, "SAturn Texture Loaded!!..\n");  
        }
    }
    else if(strcmp(PlanName, "uranus") == 0) {
        fprintf(fptr, "Uranus Called!!..\n");
        
        Planet->m_PlanetName = PlanName;
        Planet->m_fTransFromSun = 7.1f;
        Planet->m_fRotationAlongSun = 0.0f;
        Planet->m_fRevolutions = 0.0f;
        Planet->m_quadPlnetObj = gluNewQuadric();
        Planet->m_fPlanetRad = 0.25f;
        Planet->m_RotUpVal = 0.06f;
        Planet->m_RevUpVal = 0.55f;
        if(loadTexture(&Planet->m_iPlanetTex, MAKEINTRESOURCE(ID_URANUS)) == FALSE) {
            fprintf(fptr, "Texture Loading Failed!!..\n");
        }
        else {
            fprintf(fptr, "Uranus Texture Loaded!!..\n");  
        }
    }
    else if(strcmp(PlanName, "neptune") == 0) {
        fprintf(fptr, "Neptune Called!!..\n");
        
        Planet->m_PlanetName = PlanName;
        Planet->m_fTransFromSun = 7.9f;
        Planet->m_fRotationAlongSun = 0.0f;
        Planet->m_fRevolutions = 0.0f;
        Planet->m_quadPlnetObj = gluNewQuadric();
        Planet->m_fPlanetRad = 0.28f;
        Planet->m_RotUpVal = 0.0678f;
        Planet->m_RevUpVal = 0.54f;
        if(loadTexture(&Planet->m_iPlanetTex, MAKEINTRESOURCE(ID_NEPTUNE)) == FALSE) {
            fprintf(fptr, "Texture Loading Failed!!..\n");
        }
        else {
            fprintf(fptr, "Neptune Texture Loaded!!..\n");  
        }
    }
    else {
        fprintf(fptr, "Else Called Inert data!!..");
        return;
    }

    if(strcmp(Planet->m_PlanetName, "saturn") ==0) {
        Planet->m_RingRadArr[0] = 0.65f;
        Planet->m_RingRadArr[1] = 0.90f;
    }
    else {
        Planet->m_RingRadArr[0] = 0.0f;
        Planet->m_RingRadArr[1] = 0.0f;
    }

    Planet->m_NextPlanet = nullptr;
    gluQuadricTexture(Planet->m_quadPlnetObj, GL_TRUE);
    Planet->m_fTransX = 0.0f;
    Planet->m_fTransY = 0.0f;
    Planet->m_fTransZ = 0.0f;
    fprintf(fptr,"Exiting InsertData : %d\n",iCallCnt);
}

void PlanetQueue::Render() {

    static bool bEnter = false;
    if(!bEnter) {
        fprintf(fptr,"Entering Render : %d\n",iCallCnt);
        fflush(fptr);
    }
    if(FirstPlanet == nullptr) {
        return;
    }
    else {
        Travel = FirstPlanet;

        while(Travel != nullptr) {

            glPushMatrix(); 

            glRotatef(Travel->m_fRotationAlongSun, 0.0f, 1.0f, 0.0f); 
            glTranslatef(Travel->m_fTransFromSun, 0.0f, 0.0f);

            if(strcmp(Travel->m_PlanetName, "saturn") == 0) {
                glPushMatrix();
                if(!bEnter)
                    fprintf(fptr, "Rings Called");
                //glTranslatef(Travel->m_fTransFromSun, 0.0f, 0.0f);
                glRotatef(80.0f, 1.0f, 0.0f, 0.0f);
                glRotatef(Travel->m_fRevolutions, 0.0f, 0.0f, 1.0f); 
                glBindTexture(GL_TEXTURE_2D, iSatRingsTex);
                gluDisk(ringObj, Travel->m_RingRadArr[0], Travel->m_RingRadArr[1], 70, 70);
                glPopMatrix();
            }

            glRotatef(90.0f, 1.0f, 0.0f, 0.0f); 
            glRotatef(Travel->m_fRevolutions, 0.0f, 0.0f, 1.0f); 
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            glBindTexture(GL_TEXTURE_2D, Travel->m_iPlanetTex);
            gluSphere(Travel->m_quadPlnetObj, Travel->m_fPlanetRad, 120, 120);

            glPopMatrix();

            Travel = Travel->m_NextPlanet;
        }
    }
    bEnter = true;
}

void PlanetQueue::RenderFour(void) {

    static bool bEnter = false;
    if(!bEnter) {
        fprintf(fptr,"Entering Render Four : %d\n",iCallCnt);
        fflush(fptr);
    }
    if(FirstPlanet == nullptr) {
        return;
    }
    else {
        Travel = FirstPlanet;

        while(Travel != nullptr) {

            glPushMatrix();

            glTranslatef(Travel->m_fTransX, Travel->m_fTransY, Travel->m_fTransZ);

            if(strcmp(Travel->m_PlanetName, "saturn") == 0) {
                glPushMatrix();
                if(!bEnter)
                    fprintf(fptr, "Rings Render Four\n");
                glRotatef(80.0f, 1.0f, 0.0f, 0.0f);
                glRotatef(15.0f, 0.0f, 1.0f, 0.0f);
                glBindTexture(GL_TEXTURE_2D, iSatRingsTex);
                gluDisk(ringObj, Travel->m_RingRadArr[0], Travel->m_RingRadArr[1], 70, 70);
                glPopMatrix();
            }

            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            if(strcmp(Travel->m_PlanetName, "saturn") == 0)
                glRotatef(15.0f, 0.0f, 1.0f, 0.0f);
            else if(strcmp(Travel->m_PlanetName, "earth") == 0)
                glRotatef(60.0f,0.0f,0.0f, 1.0f);
            glBindTexture(GL_TEXTURE_2D, Travel->m_iPlanetTex);
            gluSphere(Travel->m_quadPlnetObj, Travel->m_fPlanetRad, 300, 300);

            glPopMatrix();

            Travel = Travel->m_NextPlanet;
        }
    }
    bEnter = true;
    
}

void PlanetQueue::UpdatePlanetPos(void) {

    if(FirstPlanet == nullptr) {
        return;
    }
    else {
        Travel = FirstPlanet;

        while(Travel != nullptr) {

            Travel->m_fRotationAlongSun += Travel->m_RotUpVal;
            if(Travel->m_fRotationAlongSun >= 360.0f) {
                Travel->m_fRotationAlongSun = 0.0f;
            }

            Travel->m_fRevolutions += Travel->m_RevUpVal;
            if(Travel->m_fRevolutions >= 360.0f) {
                Travel->m_fRevolutions = 0.0f;
            }

            Travel = Travel->m_NextPlanet;
        }
    }
}

void CreatePlanetQueue(void) {

    fprintf(fptr,"Called Create Queue\n");

    Galaxy = new PlanetQueue();

    Galaxy->EnqueuePlanet("sun");
    fflush(fptr);
    Galaxy->EnqueuePlanet("mercury");
    fflush(fptr);
    Galaxy->EnqueuePlanet("venus");
    fflush(fptr);
    Galaxy->EnqueuePlanet("earth");
    fflush(fptr);
    Galaxy->EnqueuePlanet("mars");
    fflush(fptr);
    Galaxy->EnqueuePlanet("jupiter");
    fflush(fptr);
    Galaxy->EnqueuePlanet("saturn");
    fflush(fptr);
    Galaxy->EnqueuePlanet("uranus");
    fflush(fptr);
    Galaxy->EnqueuePlanet("neptune");
    fflush(fptr); 
    fprintf(fptr,"Out Of CreateQueue\n");
    fflush(fptr);
    bStartSong = true;    
}

void DestroyPlanetQueue() {

    Galaxy->~PlanetQueue();

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

    if(listBaseOne) {
        glDeleteLists(listBaseOne, 96);
        listBaseOne = 0;
    }

    if(listBaseTwo) {
        glDeleteLists(listBaseTwo, 96);
        listBaseTwo = 0;
    }

    if(Galaxy) {
       /*  Galaxy->DequeuePlanet();
        fprintf(fptr, "Dequeue Successful!!..\n"); */
        delete(Galaxy);
        Galaxy = NULL;
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
