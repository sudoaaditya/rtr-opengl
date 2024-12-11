#include<stdio.h>
#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include"Box.h"

#define _USE_MATH_DEFINES 1
#include<math.h>
#define WIN_WIDTH 600
#define WIN_HEIGHT 600
#define ARR_LIM WIN_WIDTH/20 * WIN_HEIGHT/20

#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")

//Global Variables
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
bool gbActivateWindow = false;
FILE *fptr = NULL;

LRESULT CALLBACK MyCallBack(HWND,UINT,WPARAM,LPARAM);

//global vars for clss

void RemoveEdges(void);
int GetIndx(int, int);
void Fill(void);
int Random(int,int);
void RemoveWalls(Box, Box);

int iWBox = 20;
int rows = floor(WIN_WIDTH/iWBox);
int cols = floor(WIN_HEIGHT/iWBox);
int iCArrIndx = 0;
int iCStIndx = 0;
Box Grid[ARR_LIM];
Box Stack[1000];
Box Neighbours[4];
Box CurrBox, NextBox;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreevInstance,LPSTR lpszCmdLine, int iCmdShow) {

    //var & Fun
    int initialize(void);
    void display(void);

    
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

    if(fopen_s(&fptr,"LogMaze.txt","w") != 0) {
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
    
    //calcus for centralizing window
    monWidth = GetSystemMetrics(SM_CXMAXIMIZED);
	monHeight = GetSystemMetrics(SM_CYMAXIMIZED);

	xPar = (monWidth / 2) - (WIN_WIDTH / 2);
	yPar = (monHeight / 2) - (WIN_HEIGHT / 2);

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
        sszApp,
        TEXT("MAZE"),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
        xPar,
        yPar,
        630,
        650,
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
        fprintf(fptr,"Initialization Successful!!..(%d,%d)\n",rows,cols);
    }

    ShowWindow(hwnd,iCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    Fill();

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
                //Cal to update
            }
            display();
        }
    }

    return((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

    //Func
    void uninitialize(void);
    void resize(int, int);


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
    //gluOrtho2D(-width/2,width/2,-height/2,height/2);
    gluOrtho2D(0,width,height,0);
}


void display(void) {

    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    for(int i=0; i<ARR_LIM; i++) {
        Grid[i].Show();
    }

    CurrBox.visitedBox=true;

   //NextBox = CurrBox.CheckNeighbour();

    //RemoveWalls(CurrBox, NextBox);

    //CurrBox = NextBox;

    SwapBuffers(ghdc);
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
//All Func of class and global related to draw..

void Fill(void) {
    
    //Fill  GRID with Box Objects
    for(int i=1; i<=rows; i++) {
        for(int j=1; j<=cols; j++) {
            Grid[iCArrIndx].PutVal(i,j);

            iCArrIndx++;
        }
    }

    CurrBox = Grid[0];
}

int GetIndx(int i, int j) {
    if(i < 0 || j < 0 || i > rows || j > cols) {
        return(-1);
    }
    else {
        return(j+(i*cols));
    }
}

int Random(int min, int max) {

    return( min + rand() / (RAND_MAX / (max - min + 1) + 1));
}

// top right bottom left 0 1 2 3
void RemoveWalls(Box a, Box b) {
  int x = a.x - b.x;
  if (x == 1) {
    a.walls[3] = false;
    b.walls[1] = false;
  } else if (x == -1) {
    a.walls[1] = false;
    b.walls[3] = false;
  }
  int y = a.y - b.y;
  if (y == 1) {
    a.walls[0] = false;
    b.walls[2] = false;
  } else if (y == -1) {
    a.walls[2] = false;
    b.walls[0] = false;
  }
}

Box::Box() {


}

Box::~Box() {
    if(this) {
        delete(this);
    }
}

void Box::PutVal(int i, int j) {
    this->x = i;
    this->y = j;
    
    for(int i =0; i<4; i++) {
        this->walls[i] = true;
    }
    
    this->visitedBox = false;

  //  fprintf(fptr,"\tEle[%d] : (%d & %d)\n",iCArrIndx,x,y);

}
Box Box::CheckNeighbour() {

    int pIndx = 0;
    Box top = Grid[GetIndx(x,y-1)];
    Box right = Grid[GetIndx(x+1,y)];
    Box bottom = Grid[GetIndx(x,y+1)];
    Box left = Grid[GetIndx(x-1,y)];

    fprintf(fptr,"%d & %d, %d & %d, %d & %d, %d & %d\n",top.x,top.y,right.x,right.y,bottom.x,bottom.y,left.x,left.y);
    
    if (!top.visitedBox) {
      Neighbours[pIndx] = top;
      pIndx++;
    }
    if (!right.visitedBox) {
      Neighbours[pIndx] = right;
      pIndx++;
    }
    if (!bottom.visitedBox) {
      Neighbours[pIndx] = bottom;
      pIndx++;
    }
    if (!left.visitedBox) {
      Neighbours[pIndx] = left;
      pIndx++;
    }
    int rIndx = Random(0,pIndx);

    return(Neighbours[rIndx]);

}
void Box::Show() {

    int iX = this->x*iWBox;
    int iY = this->y*iWBox;

    glColor3f(1.0f,1.0f,1.0f);
    glLineWidth(2.0f);

    //Top
    if(this->walls[0]) {
        glBegin(GL_LINES);
        glVertex2i(iX, iY);
        glVertex2i(iX+iWBox, iY);
        glEnd();
    }
    //Right
    if(this->walls[1]) {
        glBegin(GL_LINES);
        glVertex2i(iX+iWBox, iY);
        glVertex2i(iX+iWBox, iY+iWBox);
        glEnd();        
    }
   
    //Bottom
    if(this->walls[2]) {
        glBegin(GL_LINES);
        glVertex2i(iX+iWBox, iY+iWBox);
        glVertex2i(iX, iY+iWBox);
        glEnd();        
    }
    //Left
    if(this->walls[3]) {
        glBegin(GL_LINES);
        glVertex2i(iX, iY+iWBox);
        glVertex2i(iX, iY);
        glEnd();        
    }

    if(this->visitedBox) {
        //181 230 29
        glColor3f(0.709f,0.901f,0.113f);
        glRecti(iX+2,iY+2,iWBox-2,iWBox-2);
    }
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//    glRecti(iX, iY, iX+iWBox, iY+iWBox);
}
