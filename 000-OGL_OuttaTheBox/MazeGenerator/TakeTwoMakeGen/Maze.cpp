#include<stdio.h>
#include<iostream>
#include<vector>
#include<stack>
#include<time.h>    //For srand
using namespace std;

#include<windows.h>
#include<gl/GL.h>
#include<gl/GLU.h>
#include"Box.h"

#define _USE_MATH_DEFINES 1
#include<math.h>
#define WIN_WIDTH 600
#define WIN_HEIGHT 600

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

//Glob For Boxes

int iWBox = 20;
int rows = floor(WIN_WIDTH/iWBox);
int cols = floor(WIN_HEIGHT/iWBox);

vector<Box> Grid;
int IndxArr[4];
stack <Box*> Stack;

Box *Current;


//Func
int GetIndx(int, int);
int Random(int, int);
void InitVector(void);
void RemoveWalls(Box*, Box*);


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

    //Create Vector Objects
    InitVector();

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

    srand(time(NULL));

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

   static int iFrmRate = 0;
    int iRetIndx = 0;
    Box *Next;

    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    for(int i = 0; i < Grid.size(); i++) {
        Grid[i].Show();
    }

    //ONE
    Current->visitedBox = true;
    Current->MakeBold();

    if(iFrmRate == 20) {

        iRetIndx = Current->CheckNeighbour();

        if(iRetIndx != -1) {
            
            Next = &Grid.at(iRetIndx);

            Next->visitedBox = true;

            //TWO
            Stack.push(Current);

            //THREE
            RemoveWalls(Current, Next);
            
            //FOUR
            Current = Next;
        }
        else if(!Stack.empty()) {
           // fprintf(fptr,"Here");
            Current = Stack.top();
            Stack.pop();
       }
        iFrmRate = 0;
    }
    iFrmRate++;
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

//Coding Implementation

Box::Box(int x_ , int y_) {
    x = x_;
    y = y_;

    for(int i=0; i<4; i++) {
        walls[i] = true;
    }

    visitedBox = false;
}

Box::~Box() {

}


int Box::CheckNeighbour(){

    int iCArrIndx = 0, iRet;

    //Flushing Array
    for(int i = 0; i<4; i++) {
        IndxArr[i] = 0;
    }

    //Top 
    iRet = GetIndx(this->x, this->y-1);
    if(iRet != -1 && Grid.at(iRet).visitedBox == false) {
        IndxArr[iCArrIndx] = iRet;
        iCArrIndx++;
    }

    //Right
    iRet = GetIndx(this->x+1, this->y);
    if(iRet != -1 && Grid.at(iRet).visitedBox == false) {
        IndxArr[iCArrIndx] = iRet;
        iCArrIndx++;        
    }

    //Bottom
    iRet =GetIndx(this->x, this->y+1);
    if(iRet != -1 && Grid.at(iRet).visitedBox == false) {
        IndxArr[iCArrIndx] = iRet;
        iCArrIndx++;        
    }

    //Left
    iRet = GetIndx(this->x-1, this->y);
    if(iRet != -1 && Grid.at(iRet).visitedBox == false) {
        IndxArr[iCArrIndx] = iRet;
        iCArrIndx++;        
    }

    if(iCArrIndx == 0) {
        return(-1);
    }
    else {
        int IRand = floor(Random(0, iCArrIndx-1));
        return(IndxArr[IRand]);
    }
}

void Box::Show() {

    int iX = this -> x * iWBox;
    int iY = this -> y * iWBox;

    //glColor3f(1.0f, 1.0f, 1.0f);
    glColor3f(0.0f, 0.0f, 0.0f);
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
        //156 69 156
        //glColor3f(0.611f,0.0270f,0.611f);
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2i(iX, iY);
        glVertex2i(iX+iWBox, iY);
        glVertex2i(iX+iWBox, iY+iWBox);        
        glVertex2i(iX, iY+iWBox);
        glEnd();
    } 

//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//    glRecti(iX, iY, iX+iWBox, iY+iWBox);
}

void Box::MakeBold(void) {
    int iX = this->x * iWBox;
    int iY = this->y * iWBox;
    glColor3f(0.0f, 1.0f, 0.0f);

    glBegin(GL_QUADS);
    glVertex2i(iX, iY);
    glVertex2i(iX+iWBox, iY);
    glVertex2i(iX+iWBox, iY+iWBox);        
    glVertex2i(iX, iY+iWBox);
    glEnd();        
}

int operator!(Box &b) {

    if(b.x>=0 && b.y>=0) {
        return(1);
    }
    else {
        return(0);
    }
}


//Normal Function

void InitVector(void){

    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            Grid.push_back(Box(i,j));
        }
    }

    Current = &Grid[0];

}

int GetIndx(int i, int j) {
    if(i < 0 || j < 0 || i > cols-1 || j > rows-1) {
        return(-1);
    }
    return(j+(i * cols));
}

int Random(int min, int max) {

    return(min +( rand() % (max - min + 1)));
}

void RemoveWalls(Box *one, Box *two) {

    int xDiff = one->x - two->x;

    if (xDiff == 1) {
        one->walls[3] = false;
        two->walls[1] = false;
    }
    else if (xDiff == -1) {
        one->walls[1] = false;
        two->walls[3] = false;
    }

    int yDiff = one->y - two->y;
    if (yDiff == 1) {
        one->walls[0] = false;
        two->walls[2] = false;
    } 
    else if (yDiff == -1) {
        one->walls[2] = false;
        two->walls[0] = false;
    }

}