#include<windows.h>
#include<stdio.h>
#include<gl/glew.h>
#include<gl/GL.h>
#include"./../vmath/vmath.h"
using namespace vmath;

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

enum {
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD0
};

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")

//Glob Vars
bool gbFullScreen = false;
bool gbActiveWindow = false;
DWORD dwStyle = NULL;
HWND ghwnd = NULL;
HDC ghdc =  NULL;
HGLRC ghrc = NULL;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
FILE *fptr = NULL;

// Shader Variables
GLuint gVertexShaderObj;
GLuint gFragmentShaderObj;
GLuint gProgramShaderObj;
GLuint vao, vboAxes, vaoHor, vaoVert, vboLinesHor, vboLinesVert, vaoHorDwn, vboLinesHorDwn, vaoVertDwn, vboLinesVertDwn;
GLuint vboCol1;
GLuint vaoTriangle, vboTrianglePos, vboTriangleCol;
GLuint vaoRectangle, vboRectanglePos, vboRectangleCol;
GLuint vaoCircle, vboCirclePos, vboCircleCol;
GLuint vaoOutCirc, vboOutCircPos, vboOutCircCol;
GLuint mvpUniform;
mat4 perspectiveProjMat;
int circleLen = 0;

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {

    //local Vars & Funcs
    WNDCLASSEX wndclass;
    MSG msg;
    TCHAR szAppName[] = TEXT("OGL_PP");
    HWND hwnd;
    int iRet;
    bool bDone = false;

    void display(void);
    int initialize(void);
    void ToggleFullScreen(void);

    if(fopen_s(&fptr, "_GraphLog.txt", "w") != 0) {
        MessageBox(NULL, TEXT("Cannot Create Log File!!.."), TEXT("ErrMsg"), MB_OK);
        exit(0);
    }
    else {
        fprintf(fptr, "File created Successfully!!..\n\n");
    }

    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;
    wndclass.lpfnWndProc = MyCallBack;
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hInstance = hInstance;

    RegisterClassEx(&wndclass);

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                        szAppName,
                        TEXT("PP_GraphPaper"),
                        WS_OVERLAPPEDWINDOW |
                        WS_CLIPCHILDREN|
                        WS_CLIPSIBLINGS|
                        WS_VISIBLE,
                        100, 100,
                        WIN_WIDTH, WIN_HEIGHT,
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
        fprintf(fptr,"glewInit Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else {
        fprintf(fptr,"Initialization Successful!!!...\n");
    }

    ShowWindow(hwnd, iCmdShow);
    SetFocus(hwnd);
    SetForegroundWindow(hwnd);
    ToggleFullScreen();

    //Msg Loop 
    while(!bDone) {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)){
            if(msg.message == WM_QUIT){
                bDone = true;
            }
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else {
            if(gbActiveWindow){
                display();
            }
        }
    }

return((int)msg.wParam);
}

LRESULT CALLBACK MyCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

    //func and vars
    void ToggleFullScreen(void);
    void uninitialize(void);
    void resize(int, int);

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

void ToggleFullScreen(void) {

    MONITORINFO mi;

    if(!gbFullScreen) {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

        if(dwStyle & WS_OVERLAPPEDWINDOW) {
            mi = {sizeof(MONITORINFO)};

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
        ShowCursor(FALSE);
        gbFullScreen = true;
    }
    else {
        
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

        SetWindowPlacement(ghwnd, &wpPrev);

        SetWindowPos(ghwnd, 
            HWND_TOP,
            0, 0, 0, 0,
            SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE);    

        ShowCursor(TRUE);
        gbFullScreen = false;
    }
}

int initialize(void) {

    //fun & var
    void resize(int,int);
    void uninitialize(void);

    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;
    GLenum result;
    //For Shader Comiple
    GLint iShaderCompileStatus = 0;
    GLint iShaderInfoLogLen = 0;
    GLchar* szInfoLog = NULL;

    //For inCircle Calcus!
    GLfloat fx1, fx2, fx3, fy1, fy2, fy3;
	GLfloat fArea, fRad, fPer;
	GLfloat fdAB, fdBC, fdAC;
	GLfloat fxCord, fyCord;
	static GLfloat fAngle = 0.0f;
    static bool bWrite = false;

    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize =  sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.cColorBits = 32;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cRedBits = 8;
    pfd.cBlueBits = 8;
    pfd.cGreenBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;

    ghdc = GetDC(ghwnd);

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0){
        return(-1);
    }
    else {
        fprintf(fptr, "Choose Pixel Format Successful!!..\t{Index = %d}\n", iPixelFormatIndex);
    }

    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE){
        return(-2);
    }
    else {
        fprintf(fptr, "SetPixelFormat Successful!\n");
    }

    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL){
        return(-3);
    }
    else {
        fprintf(fptr, "wglCreateContext Successsful!!..\n");
    }

    if(wglMakeCurrent(ghdc, ghrc) == FALSE) {
        return(-4);
    }
    else {
        fprintf(fptr, "wglMakeCurrent Successful!.\n");
    }

    result = glewInit();
    if(result != GLEW_OK){
        return(-5);
    }
    else {
        fprintf(fptr, "glewInit Successful!..\n");
    }

    gVertexShaderObj = glCreateShader(GL_VERTEX_SHADER);

    const GLchar *vertexShaderSourceCode = 
                    "#version 460 core" \
                    "\n" \
                    "in vec4 vPosition;" \
                    "in vec4 vColor;" \
                    "out vec4 out_Color;"   \
                    "uniform mat4 u_mvp_matrix;" \
                    "void main(void)" \
                    "{" \
                    "gl_Position = vPosition * u_mvp_matrix;" \
                    "out_Color = vColor;" \
                    "}";

    glShaderSource(gVertexShaderObj, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

    glCompileShader(gVertexShaderObj);

    glGetShaderiv(gVertexShaderObj, GL_COMPILE_STATUS, &iShaderCompileStatus);

    if(iShaderCompileStatus == GL_FALSE) {
        glGetShaderiv(gVertexShaderObj, GL_INFO_LOG_LENGTH, &iShaderInfoLogLen);

        if(iShaderInfoLogLen > 0) {
            szInfoLog = (GLchar*)malloc(iShaderInfoLogLen);

            if(szInfoLog != NULL) {
                GLsizei written;

                glGetShaderInfoLog(gVertexShaderObj,
                    iShaderInfoLogLen,
                    &written,
                    szInfoLog);

                fprintf(fptr, "Vertex Shader Log::\n %s\n", szInfoLog);

                free(szInfoLog);
                uninitialize();
                DestroyWindow(ghwnd);
                exit(0);
            }
        }
    }
    else {
        fprintf(fptr, "Vertex Shader Compiled Successfully!!..\n");
    }

    //Fragment Shader : create Shader Objet!
    iShaderCompileStatus = 0;
    iShaderInfoLogLen = 0;
    szInfoLog = NULL;
    gFragmentShaderObj = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar *fragmentShaderSourceCode = 
                    "#version 460 core" \
                    "\n" \
                    "in vec4 out_Color;" \
                    "out vec4 FragColor;" \
                    "void main(void)" \
                    "{" \
                    "FragColor = out_Color;" \
                    "}";

    glShaderSource(gFragmentShaderObj, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);

    glCompileShader(gFragmentShaderObj);

    glGetShaderiv(gFragmentShaderObj, GL_COMPILE_STATUS, &iShaderCompileStatus);

    if(iShaderCompileStatus == GL_FALSE) {
        glGetShaderiv(gFragmentShaderObj, GL_INFO_LOG_LENGTH, &iShaderInfoLogLen);

        if(iShaderInfoLogLen > 0) {
            szInfoLog = (GLchar*)malloc(iShaderInfoLogLen);
            if(szInfoLog != NULL) {
                GLint written;

                glGetShaderInfoLog(gVertexShaderObj,
                                iShaderInfoLogLen,
                                &written,
                                szInfoLog);

                fprintf(fptr,"Fragment Shader Log :\n %s\n",szInfoLog);
                free(szInfoLog);
                uninitialize();
                DestroyWindow(ghwnd);
                exit(0);
            }
        }
    }
    else {
        fprintf(fptr, "Fragement ShaderCompiled Successfully!!..\n");
    }

    //Noe Create Program
    GLint iProgLinkStatus = 0;
    GLint iProgLogLen = 0;
    GLchar* szProgLog = NULL;

    gProgramShaderObj = glCreateProgram();
    glAttachShader(gProgramShaderObj, gVertexShaderObj);
    glAttachShader(gProgramShaderObj, gFragmentShaderObj);

    glBindAttribLocation(gProgramShaderObj, AMC_ATTRIBUTE_POSITION, "vPosition");
    glLinkProgram(gProgramShaderObj);

    glGetProgramiv(gProgramShaderObj, GL_LINK_STATUS, &iProgLinkStatus);

    if(iProgLinkStatus == GL_FALSE) {
        glGetProgramiv(gProgramShaderObj, GL_INFO_LOG_LENGTH, &iProgLogLen);

        if(iProgLogLen > 0) {
            szProgLog = (GLchar*)malloc(iProgLogLen);

            if(szProgLog != NULL) {
                GLint written;

                glGetProgramInfoLog(gProgramShaderObj, iProgLogLen, &written, szProgLog);

                fprintf(fptr,"Program Link Log :\n %s\n",szProgLog);

                uninitialize();
                DestroyWindow(ghwnd);
                exit(0);
            }
        }
    }
    else {
        fprintf(fptr, "Program Linkage Successful!!\n");
    }

    mvpUniform = glGetUniformLocation(gProgramShaderObj, "u_mvp_matrix");

    GLfloat axesVert[] = {1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f};

    GLfloat horLinesVert[80];
    float fSteps = 0.05;
    for(int i = 0; i< 20; i++) {
        for(int j = 0; j < 4; j++) {
            if(j == 0) {
                horLinesVert[(i*4) + j] = 1.5f;
            }
            else if(j == 2) {
                horLinesVert[(i * 4) + j] = -1.5f;
            }
            else {
                horLinesVert[(i * 4) + j] = fSteps;
            }
        }
        fSteps += 0.05f;
    }
    GLfloat horLinesVertDwn[80];
    fSteps = 0.05f;
    for(int i = 0; i< 20; i++) {
        for(int j = 0; j < 4; j++) {
            if(j == 0) {
                horLinesVertDwn[(i*4) + j] = 1.5f;
            }
            else if(j == 2) {
                horLinesVertDwn[(i * 4) + j] = -1.5f;
            }
            else {
                horLinesVertDwn[(i * 4) + j] = -fSteps;
            }
        }
        fSteps += 0.05f;
    }
    GLfloat vertLinesVert[120];
    fSteps = 0.05f;
    for(int i = 0; i< 30; i++) {
        for(int j = 0; j < 4; j++) {
            if(j == 1) {
                vertLinesVert[(i*4) + j] = 1.0f;
            }
            else if(j == 3) {
                vertLinesVert[(i * 4) + j] = -1.0f;
            }
            else {
                vertLinesVert[(i * 4) + j] = fSteps;
            }
        }
        fSteps += 0.05f;
    }
    GLfloat vertLinesVertDwn[120];
    fSteps = 0.05f;
    for(int i = 0; i< 30; i++) {
        for(int j = 0; j < 4; j++) {
            if(j == 1) {
                vertLinesVertDwn[(i*4) + j] = 1.0f;
            }
            else if(j == 3) {
                vertLinesVertDwn[(i * 4) + j] = -1.0f;
            }
            else {
                vertLinesVertDwn[(i * 4) + j] = -fSteps;
            }
        }
        fSteps += 0.05f;
    }
    GLfloat axesCol[] = {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.1f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
    //Axes
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vboAxes);
    glBindBuffer(GL_ARRAY_BUFFER, vboAxes);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesVert), axesVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glGenBuffers(1, &vboCol1);
    glBindBuffer(GL_ARRAY_BUFFER, vboCol1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesCol), axesCol, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);   
    //Up Horizontal!
    glGenVertexArrays(1, &vaoHor);
    glBindVertexArray(vaoHor);
    glGenBuffers(1, &vboLinesHor);
    glBindBuffer(GL_ARRAY_BUFFER, vboLinesHor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(horLinesVert), horLinesVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);       
    glBindVertexArray(0); 

    glGenVertexArrays(1, &vaoHorDwn);
    glBindVertexArray(vaoHorDwn);
    glGenBuffers(1, &vboLinesHorDwn);
    glBindBuffer(GL_ARRAY_BUFFER, vboLinesHorDwn);
    glBufferData(GL_ARRAY_BUFFER, sizeof(horLinesVertDwn), horLinesVertDwn, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);       
    glBindVertexArray(0);

    glGenVertexArrays(1, &vaoVert);
    glBindVertexArray(vaoVert);
    glGenBuffers(1, &vboLinesVert);
    glBindBuffer(GL_ARRAY_BUFFER, vboLinesVert);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertLinesVert), vertLinesVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);        
    glBindVertexArray(0); 

    glGenVertexArrays(1, &vaoVertDwn);
    glBindVertexArray(vaoVertDwn);
    glGenBuffers(1, &vboLinesVertDwn);
    glBindBuffer(GL_ARRAY_BUFFER, vboLinesVertDwn);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertLinesVertDwn), vertLinesVertDwn, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);       
    glBindVertexArray(0); 

    //Related TO Triangle
    fx1 = 0.0f;
	fy1 = 1.0f;
	fx2 = -1.0f;
	fy2 = -1.0f;
	fx3 = 1.0f;
	fy3 = -1.0f;
    
    GLfloat TriangleVert[] = {fx1, fy1, 0.0f, fx2, fy2, 0.0f, fx3, fy3, 0.0f, fx1, fy1, 0.0f};
    GLfloat TriangleColor[] = {1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f};

    glGenVertexArrays(1, &vaoTriangle);
    glBindVertexArray(vaoTriangle);
    glGenBuffers(1, &vboTrianglePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboTrianglePos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleVert), TriangleVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glGenBuffers(1, &vboTriangleCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboTriangleCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleColor), TriangleColor, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);   
    
    //Rectangle!.
    GLfloat rectVert[] = {fx3, fy1, 0.0f, fx2, fy1, 0.0f, fx2, fy2, 0.0f, fx3, fy3, 0.0f, fx3, fy1, 0.0f};
    GLfloat rectCol[] = {1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f};

    glGenVertexArrays(1, &vaoRectangle);
    glBindVertexArray(vaoRectangle);
    glGenBuffers(1, &vboRectanglePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboRectanglePos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVert), rectVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glGenBuffers(1, &vboRectangleCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboRectangleCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectCol), rectCol, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);  

    //InCircle cha Rada!.
    //diatances of each side of triangle by distance formula
	fdAB = sqrtf(((fx2 - fx1)*(fx2 - fx1)) + ((fy2 - fy1)*(fy2 - fy1)));
	fdBC = sqrtf(((fx3 - fx2)*(fx3 - fx2)) + ((fy3 - fy2)*(fy3 - fy2)));
	fdAC = sqrtf(((fx3 - fx1)*(fx3 - fx1)) + ((fy3 - fy1)*(fy3 - fy1)));

	//perimeter of triangle >> A+B+C and we need half of it for area 
	fPer = ((fdAB + fdAB + fdBC) / 2);

	//are of T = sqrt(P(P-A)(P-B)(P-C))
	fArea = sqrtf(fPer*(fPer - fdAB)*(fPer - fdBC)*(fPer - fdAC));

	//Radius of inCircle = AreaOf T/Perimete Of T
	fRad = (fArea / fPer);

	if (!bWrite) {
		fprintf(fptr, "fdAB = %f\nfdBC = %f\nfdAC = %f\nfPer =  %f\nfArea = %f\nfRad = %f\n", fdAB, fdBC, fdAC, fPer, fArea, fRad);
	}

	fxCord = (GLfloat)(((fdBC*fx1) + (fx2*fdAC) + (fx3*fdAB)) / (fPer * 2));
	fyCord = (GLfloat)(((fdBC*fy1) + (fy2*fdAC) + (fy3*fdAB)) / (fPer * 2));

	if (!bWrite) {
		fprintf(fptr, "\n\nfx1 = %f\tfy1 = %f\nfx2 = %f\tfy2 = %f\nfx3 = %f\tfy3 = %f\n\nfxCoord = %f\tfyCord = %f\n", fx1, fy1, fx2, fy2, fx3, fy3, fxCord, fyCord);
        fflush(fptr);
	}
    GLfloat circleVert[12580];
    GLfloat circleCol[18870];
    int i, j;
    float circleSteps = 0.0f;
    for(i = 0; i < 6290; i++) {
        for(j = 0; j < 2; j++) {
            if(j==0)
                circleVert[ (i*2) + j] =  fxCord + cosf(circleSteps)*fRad;
            else
                circleVert[ (i*2) + j] =  fyCord + sinf(circleSteps)*fRad;
        }
        circleSteps += 0.001f;
        circleCol[(i*2) + 0] = 1.0f;
        circleCol[(i*2) + 1] = 1.0f;
        circleCol[(i*2) + 2] = 0.0f;
    }
    circleLen = sizeof(circleVert);
    
    glGenVertexArrays(1, &vaoCircle);
    glBindVertexArray(vaoCircle);
    glGenBuffers(1, &vboCirclePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboCirclePos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVert), circleVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glGenBuffers(1, &vboCircleCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboCircleCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleCol), circleCol, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);     
    glBindVertexArray(0);

    GLfloat outCircVert[12580];
    GLfloat outCircColor[18870];
    circleSteps = 0.0f;
    for(i = 0; i< 6280; i++) {
        for(j = 0; j < 2; j++) {
            if(j == 0)
                outCircVert[ (i*2) + j] = 1.42f*cosf(circleSteps);
            else
                outCircVert[ (i*2) + j] = 1.42f*sinf(circleSteps);
        }
        circleSteps += 0.001f;
        circleSteps += 0.001f;
        outCircColor[(i*2) + 0] = 1.0f;
        outCircColor[(i*2) + 1] = 1.0f;
        outCircColor[(i*2) + 2] = 0.0f;
    }

    glGenVertexArrays(1, &vaoOutCirc);
    glBindVertexArray(vaoOutCirc);
    glGenBuffers(1, &vboOutCircPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboOutCircPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(outCircVert), outCircVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glGenBuffers(1, &vboOutCircCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboOutCircCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(outCircColor), outCircColor, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);     
    glBindVertexArray(0);


    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    perspectiveProjMat = mat4::identity();

    resize(WIN_WIDTH, WIN_HEIGHT);

    return(0);
}

void resize(int width, int height){

    if(height == 0) {
        height = 1;
    }

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    perspectiveProjMat = perspective(45.0f, 
                            ((GLfloat)width/(GLfloat)height),
                            0.1f,
                            100.0f);    
}

void display(void) {

    mat4 modelViewMat;
    mat4 modelviewProjMat;

    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gProgramShaderObj);

    modelViewMat = translate(0.0f, 0.0f, -1.0f);

    modelviewProjMat = perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);

    glLineWidth(3.0f);
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glBindVertexArray(0);

    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();
    modelViewMat = translate(0.0f, 0.0f, -2.0f);
    modelviewProjMat = perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);

    glLineWidth(0.8f);
    glBindVertexArray(vaoHor);
    for(int i = 0; i< 40; i += 2) {
        glDrawArrays(GL_LINES, i, 2);
    }
    glBindVertexArray(0);

    glBindVertexArray(vaoHorDwn);
    for(int i = 0; i< 40; i += 2) {
        glDrawArrays(GL_LINES, i, 2);
    }
    glBindVertexArray(0);

    glBindVertexArray(vaoVert);
    for(int i = 0; i< 60; i += 2) {
        glDrawArrays(GL_LINES, i, 2);
    }
    glBindVertexArray(0);

    glBindVertexArray(vaoVertDwn);
    for(int i = 0; i< 60; i += 2) {
        glDrawArrays(GL_LINES, i, 2);
    }
    glBindVertexArray(0);

    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();
    modelViewMat = translate(0.0f, 0.0f, -3.5f);
    modelviewProjMat = perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);

    glLineWidth(2.9f);
    glBindVertexArray(vaoTriangle);
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 1, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glBindVertexArray(0);

    glBindVertexArray(vaoRectangle);
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 1, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glDrawArrays(GL_LINES, 3, 2);
    glBindVertexArray(0);

    glBindVertexArray(vaoCircle);
    glPointSize(3.0f);
    glDrawArrays(GL_POINTS, 0, circleLen);  
    glBindVertexArray(0);

    glBindVertexArray(vaoOutCirc);
    glPointSize(3.0f);
    glDrawArrays(GL_POINTS, 0, circleLen);  
    glBindVertexArray(0);

    glUseProgram(0);

    SwapBuffers(ghdc);
}


void uninitialize(void) {

    if(gbFullScreen) {
        SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

        SetWindowPlacement(ghwnd, &wpPrev);

        SetWindowPos(ghwnd, 
            HWND_TOP,
            0, 0, 0, 0,
            SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOSIZE);    

        ShowCursor(TRUE);
        gbFullScreen = false;
    }

    glUseProgram(gProgramShaderObj);
    if(vboAxes) {
        glDeleteBuffers(1, &vboAxes);
        vboAxes = 0;
    }
    if(vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    // Detach Fragment Shader First!.
    glDetachShader(gProgramShaderObj, gFragmentShaderObj);
    //Detach Vertex Shader
    glDetachShader(gProgramShaderObj, gVertexShaderObj);
    glDeleteShader(gFragmentShaderObj);
    gFragmentShaderObj = 0;

    glDeleteShader(gVertexShaderObj);
    gVertexShaderObj = 0;

    glDeleteProgram(gProgramShaderObj);
    gProgramShaderObj= 0;

    glUseProgram(0);

    if(ghrc == wglGetCurrentContext()) {
        wglMakeCurrent(NULL, NULL);
    }

    if(ghrc) {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    if(ghdc) {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }

    if(fptr) {
        fprintf(fptr, "\nFile Closed Successfully!!..\n");
        fclose(fptr);
        fptr = NULL;
    }
}