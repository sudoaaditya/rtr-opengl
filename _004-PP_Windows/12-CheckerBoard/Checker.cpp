#include<windows.h>
#include<stdio.h>
#include<gl/glew.h>
#include<gl/GL.h>
#include"./../vmath/vmath.h"

using namespace vmath;

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define Checker_Width 64
#define Checker_Height 64

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
GLuint vao_Rect, vbo_RectPos;
GLuint mvpUniform;
int colorUniform;
mat4 perspectiveProjMat;
GLuint vbo_RectTex;
GLuint samplerUniform;

//Checker Board Vars
GLubyte CheckerBoradData[Checker_Width][Checker_Height][4];
GLuint iCheckerTex;

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

    if(fopen_s(&fptr, "_PPCheckerBoardLog.txt", "w") != 0) {
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
                        TEXT("CheckerBoard"),
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
    void loadTexture(void);

    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;
    GLenum result;
    //For Shader Comiple
    GLint iShaderCompileStatus = 0;
    GLint iShaderInfoLogLen = 0;
    char* szInfoLog = NULL;

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

    //Shader Code 

    gVertexShaderObj = glCreateShader(GL_VERTEX_SHADER);

    const GLchar *vertexShaderSourceCode = 
                    "#version 460 core" \
                    "\n" \
                    "in vec4 vPosition;" \
                    "in vec2 vTexCoord;" \
                    "uniform mat4 u_mvp_matrix;" \
                    "out vec2 out_texCoord;"    \
                    "void main(void)" \
                    "{" \
                    "gl_Position =  u_mvp_matrix * vPosition ;" \
                    "out_texCoord = vTexCoord;" \
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

    iShaderCompileStatus = 0;
    iShaderInfoLogLen = 0;
    szInfoLog = NULL;
    gFragmentShaderObj = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar *fragmentShaderSourceCode = 
        "#version 460 core" \
        "\n" \
        "in vec2 out_texCoord;" \
        "uniform sampler2D u_sampler;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
        "FragColor = texture(u_sampler, out_texCoord);" \
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

    GLint iProgLinkStatus = 0;
    GLint iProgLogLen = 0;
    GLchar* szProgLog = NULL;

    gProgramShaderObj = glCreateProgram();

    glAttachShader(gProgramShaderObj, gVertexShaderObj);
    glAttachShader(gProgramShaderObj, gFragmentShaderObj);

    glBindAttribLocation(gProgramShaderObj, AMC_ATTRIBUTE_POSITION, "vPosition");
    glBindAttribLocation(gProgramShaderObj, AMC_ATTRIBUTE_TEXCOORD0, "vTexCoord");

    glLinkProgram(gProgramShaderObj);

    glGetProgramiv(gProgramShaderObj, GL_LINK_STATUS, &iProgLinkStatus);

    if(iProgLinkStatus == GL_FALSE) {
        glGetProgramiv(gProgramShaderObj, GL_INFO_LOG_LENGTH, &iProgLogLen);

        if(iProgLogLen > 0) {
            szProgLog = (GLchar*)malloc(iProgLogLen);

            if(szProgLog != NULL) {
                GLint written;

                glGetProgramInfoLog(gProgramShaderObj, iProgLogLen, &written, szProgLog);

                fprintf(fptr,"Program Link Log :\n %s \n",szProgLog);

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
    samplerUniform = glGetUniformLocation(gProgramShaderObj, "u_sampler");

    /* const GLfloat rectVert[] = {1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f}; */
    const GLfloat texCoords[] = { 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };

    glGenVertexArrays(1, &vao_Rect);
    glBindVertexArray(vao_Rect);

    glGenBuffers(1, &vbo_RectPos);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_RectPos);

    glBufferData(GL_ARRAY_BUFFER, (4*3*sizeof(GLfloat)), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_RectTex);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_RectTex);

    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);       
    glBindVertexArray(0);   

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    perspectiveProjMat = mat4::identity();

    glEnable(GL_TEXTURE_2D);

    loadTexture();

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


void loadTexture(void) {

    void makeCheckImage(void);

    makeCheckImage();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &iCheckerTex);
    glBindTexture(GL_TEXTURE_2D, iCheckerTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                Checker_Width, Checker_Height,
                0, GL_RGBA, GL_UNSIGNED_BYTE,
                CheckerBoradData);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void makeCheckImage(void) {
    int i, j, c;

    for(i = 0; i < Checker_Height; i++) {
        for(j = 0; j < Checker_Width; j++) {
            c = ((( i & 0x8 ) == 0) ^ (( j & 0x8 ) == 0)) * 255;

            CheckerBoradData[i][j][0] = (GLubyte)c;
            CheckerBoradData[i][j][1] = (GLubyte)c;
            CheckerBoradData[i][j][2] = (GLubyte)c;
            CheckerBoradData[i][j][3] = (GLubyte)255;
        }
    }
}

void display(void) {

    mat4 modelViewMat;
    mat4 modelviewProjMat;
    GLfloat rectVertArr[12];
    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(gProgramShaderObj);
    
    modelViewMat = translate(0.0f, 0.0f, -3.0f);
    
    modelviewProjMat = perspectiveProjMat * modelViewMat;
    
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, iCheckerTex);
    glUniform1i(samplerUniform, 0);
    glBindVertexArray(vao_Rect);

    //ONE
    rectVertArr[0] = 0.0f;
    rectVertArr[1] = 1.0f;
    rectVertArr[2] = 0.0f;
    rectVertArr[3] = -2.0f;
    rectVertArr[4] = 1.0f;
    rectVertArr[5] = 0.0f;
    rectVertArr[6] = -2.0f;
    rectVertArr[7] = -1.0f;
    rectVertArr[8] = 0.0f;
    rectVertArr[9] = 0.0f;
    rectVertArr[10] = -1.0f;
    rectVertArr[11] = 0.0f;

    glBindBuffer(GL_ARRAY_BUFFER, vbo_RectPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertArr), rectVertArr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    //Two  
    rectVertArr[0] = 2.41421f;
    rectVertArr[1] = 1.0f;
    rectVertArr[2] = -1.41421f;
    rectVertArr[3] = 1.0f;
    rectVertArr[4] = 1.0f;
    rectVertArr[5] = 0.0f;
    rectVertArr[6] = 1.0f;
    rectVertArr[7] = -1.0f;
    rectVertArr[8] = 0.0f;
    rectVertArr[9] = 2.41421f;
    rectVertArr[10] = -1.0f;
    rectVertArr[11] = -1.41421f;

    glBindBuffer(GL_ARRAY_BUFFER, vbo_RectPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVertArr), rectVertArr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
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

    if(vbo_RectTex) {
        glDeleteBuffers(1, &vbo_RectTex);
        vbo_RectTex = 0;
    }
    if(vbo_RectPos) {
        glDeleteBuffers(1, &vbo_RectPos);
        vbo_RectPos = 0;
    }
    if(vao_Rect) {
        glDeleteVertexArrays(1, &vao_Rect);
        vao_Rect = 0;
    }

    //Safe Release to Shader!!.
    if(gProgramShaderObj) {
        GLsizei iShaderCnt = 0;
        GLsizei iShaderNo = 0;

        glUseProgram(gProgramShaderObj);

        //Ask prog how many shader are attached to it
        glGetProgramiv(gProgramShaderObj, 
                        GL_ATTACHED_SHADERS, 
                        &iShaderCnt);

        GLuint *pShaders = (GLuint*)malloc(iShaderCnt * sizeof(GLuint));

        if(pShaders) {
            glGetAttachedShaders(gProgramShaderObj,
                                iShaderCnt,
                                &iShaderCnt,
                                pShaders);

            for(iShaderNo = 0; iShaderNo < iShaderCnt; iShaderNo++) {

                glDetachShader(gProgramShaderObj, pShaders[iShaderNo]);

                fprintf(fptr, "Shader: %ld\n", pShaders[iShaderNo]);

                pShaders[iShaderNo] = 0;
            }
            free(pShaders);
        }
        glDeleteProgram(gProgramShaderObj);
        gProgramShaderObj = 0;
        glUseProgram(0);
    }

    if(iCheckerTex) {
        glDeleteTextures(1, &iCheckerTex);
        iCheckerTex = 0;
    }

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


