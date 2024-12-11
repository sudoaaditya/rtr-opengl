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

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

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
GLuint gVertexShaderObj = 0;
GLuint gFragmentShaderObj = 0;
GLuint gProgramShaderObj = 0;
GLuint mvpUniform = 0;
mat4 perspectiveProjMat;
GLfloat fAngleTriangle = 0.0f;
GLfloat fAngleRectangle = 0.0f;

GLuint vao_triangle, vbo_triangle_position, vbo_triangle_color;
GLuint vao_rectangle, vbo_rectangle_position, vbo_rectangle_color;

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
    void update(void);

    if(fopen_s(&fptr, "_PersTringleShaderLog.txt", "w") != 0) {
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
                          TEXT("PP_PerspectiveTriangleReactangleColored"),
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
                update();
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

    //Shader Code : Define Vertex Shader Object
    gVertexShaderObj = glCreateShader(GL_VERTEX_SHADER);

    // Write Vertex Shader Code!.
    const GLchar *vertexShaderSourceCode = 
                    "#version 460 core" \
                    "\n" \
                    "in vec4 vPosition;" \
                    "in vec4 vColor;" \
                    "out vec4 out_color;" \
                    "uniform mat4 u_mvp_matrix;" \
                    "void main(void)" \
                    "{" \
                    "gl_Position = u_mvp_matrix * vPosition;" \
                    "out_color = vColor;" \
                    "}";

    // Specify above code to VertexShader Object.
    glShaderSource(gVertexShaderObj, 1, (const GLchar**)&vertexShaderSourceCode, NULL);

    //Compile The Vertex Shader.
    glCompileShader(gVertexShaderObj);

    //Error Checking.
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
        "in vec4 out_color;" \
        "out vec4 FragColor;" \
        "void main(void)" \
        "{" \
        "FragColor = out_color;" \
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
    //Create Shader Program Shader Object!
    gProgramShaderObj = glCreateProgram();

    //Attach VS to Shader Prog
    glAttachShader(gProgramShaderObj, gVertexShaderObj);

    //Attach FS to Shader Prog
    glAttachShader(gProgramShaderObj, gFragmentShaderObj);

    //NOW BEFORE LINK : Prelinking Binding with Vertex Attribute
    glBindAttribLocation(gProgramShaderObj, AMC_ATTRIBUTE_POSITION, "vPosition");
    glBindAttribLocation(gProgramShaderObj, AMC_ATTRIBUTE_COLOR, "vColor");

    //Now Link The Program
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

    //POST LINKING :: Retriving Uniform Location from GPU and Bind them with a var on CPU
    mvpUniform = glGetUniformLocation(gProgramShaderObj, "u_mvp_matrix");


    //Create Array of vertices to be sent over GPU
    const GLfloat triangleVert[] = {0.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f};
    const GLfloat triColors[] = {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    const GLfloat rectangleVert[] = {1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f};
    const GLfloat rectColors[] = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

    glGenVertexArrays(1, &vao_triangle);
    glBindVertexArray(vao_triangle);

    glGenBuffers(1, &vbo_triangle_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle_position);

    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVert), triangleVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_triangle_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle_color);

    glBufferData(GL_ARRAY_BUFFER, sizeof(triColors), triColors, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenVertexArrays(1, &vao_rectangle);
    glBindVertexArray(vao_rectangle);

    glGenBuffers(1, &vbo_rectangle_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_rectangle_position);

    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVert), rectangleVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_rectangle_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_rectangle_color);

    glBufferData(GL_ARRAY_BUFFER, sizeof(rectColors), rectColors, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    perspectiveProjMat = mat4::identity();

    resize(WIN_WIDTH, WIN_HEIGHT);

    return(0);
}

void resize(int width, int height){

   if(height == 0) {
        height = 1;
    }

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    perspectiveProjMat = perspective(45.0f, ((GLfloat)width/(GLfloat)height), 0.1f, 100.0f);   
    }

void display(void) {

    //Declare MAtrices
    mat4 modelViewMat;
    mat4 modelviewProjMat;
    mat4 translateMat;
    mat4 rotationMat;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gProgramShaderObj);

    //Triangle!!.
    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();
    translateMat = mat4::identity();
    rotationMat = mat4::identity();

    translateMat = translate(-1.5f, 0.0f, -6.0f);
    rotationMat = rotate(fAngleTriangle, 0.0f, 1.0f, 0.0f);
    
    modelViewMat = modelViewMat * translateMat;
    modelViewMat = modelViewMat * rotationMat;

    modelviewProjMat = perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);

    glBindVertexArray(vao_triangle);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    glBindVertexArray(0); 

    //Rectangle!
    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();
    translateMat = mat4::identity();
    rotationMat = mat4::identity();

    translateMat = translate(1.5f, 0.0f, -6.0f);
    rotationMat = rotate(fAngleRectangle, 1.0f, 0.0f, 0.0f);

    modelViewMat = modelViewMat * translateMat;
    modelViewMat = modelViewMat * rotationMat;

    modelviewProjMat = perspectiveProjMat * modelViewMat;

    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);

    glBindVertexArray(vao_rectangle);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glBindVertexArray(0);

	glUseProgram(0);
    
    SwapBuffers(ghdc);
}

void update(void) {

    fAngleTriangle += 0.8f;
    if(fAngleTriangle >= 360.0f) {
        fAngleTriangle = 0.0f;
    }

    fAngleRectangle += 0.8f;
    if(fAngleRectangle >= 360.0f) {
        fAngleRectangle = 0.0f;
    }
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

    //VAo & vbo
    if(vbo_triangle_position) {
        glDeleteBuffers(1, &vbo_triangle_position);
        vbo_triangle_position = 0;
    }
    if(vbo_triangle_color) {
        glDeleteBuffers(1, &vbo_triangle_color);
        vbo_triangle_color = 0;
    }
    if(vao_triangle) {
        glDeleteVertexArrays(1, &vao_triangle);
        vao_triangle = 0;
    }

    if(vbo_rectangle_position) {
        glDeleteBuffers(1, &vbo_rectangle_position);
        vbo_rectangle_position = 0;
    }
    if(vbo_rectangle_color) {
        glDeleteBuffers(1, &vbo_rectangle_color);
        vbo_rectangle_color = 0;
    }
    if(vao_rectangle) {
        glDeleteVertexArrays(1, &vao_rectangle);
        vao_rectangle = 0;
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
