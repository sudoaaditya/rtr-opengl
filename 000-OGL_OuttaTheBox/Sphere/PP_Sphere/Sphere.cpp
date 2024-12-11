#include<windows.h>
#include<stdio.h>
#include<gl/glew.h>
#include<gl/GL.h>
#include"vmath.h"

#define _USE_MATH_DEFINES 1
#include<math.h>

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

//Global Vars
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
DWORD dwStyle;
bool gbFullScreen = false;
bool gbActiveWindow = false;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};
FILE *fptr = NULL;

//Shader Vars
GLuint gVertexShaderObj = 0;
GLuint gFragmentShaderObj = 0;
GLuint gShaderProgramObj = 0;
GLuint vao, vbo;
GLuint mvpUniform;
mat4 perspectiveProjMat;

//Sphere vars.
int totLat = 100, totLon = 100;
GLfloat sphereVerts[100 * 100 * 3];
GLfloat fAngle = 0.0f;

LRESULT CALLBACK MyCallBack(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow) {

    //Func & Vars
    int initialize(void);
    void display(void);
    void update(void);
    void ToggleFullScreen(void);

    WNDCLASSEX wndclass;
    HWND hwnd = NULL;
    MSG msg;
    TCHAR szAppName[] = TEXT("PP_OGL");
    bool bDone = false;
    int iRet = 0;

    if(fopen_s(&fptr, "_SphereLog.txt", "w") != 0) {
        MessageBox(NULL, TEXT("Cannot Create Log!!.."), TEXT("ErrMsg"), MB_OK);
        exit(0);
    }
    else {
        fprintf(fptr, "Log File Created Successfully!!!..\n\n");
    }

    //WNDCLASS
    wndclass.cbSize = sizeof(WNDCLASSEX);
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.lpszClassName =  szAppName;
    wndclass.lpszMenuName = NULL;
    wndclass.hInstance = hInstance;
    wndclass.lpfnWndProc = MyCallBack;
    wndclass.hbrBackground = CreateSolidBrush(RGB(0, 0, 0));
    wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassEx(&wndclass);

    hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                            szAppName,
                            TEXT("Sphere_PP"),
                            WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
                            100, 100,
                            WIN_WIDTH, WIN_HEIGHT,
                            NULL, NULL,
                            hInstance, 
                            NULL);

    ghwnd = hwnd;

    iRet = initialize();
    if(iRet == -1) {
        fprintf(fptr, "ChoosePixelFormat Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -2) {
        fprintf(fptr, "SetPixelFormat Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -3) {
        fprintf(fptr, "wglCreateContext Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -4) {
        fprintf(fptr, "wglMakeCurrent Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else if(iRet == -5) {
        fprintf(fptr,"glewInit Failed!!..\n");
        DestroyWindow(hwnd);
    }
    else {
        fprintf(fptr, "Initialization Successful!..\n");
    }

    ToggleFullScreen();
    ShowWindow(hwnd, iCmdShow);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    //Message Loop!.
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

LRESULT CALLBACK MyCallBack (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam) {

    //Function!.
    void uninitialize(void);
    void resize(int, int);
    void ToggleFullScreen(void);

    switch(iMsg) {
        
        case WM_SETFOCUS :
            gbActiveWindow = true;
            break;

        case WM_KILLFOCUS:
            gbActiveWindow = false;
            break;

        case WM_SIZE:
            resize(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_KEYDOWN :
            switch (wParam) {

                case 'F':
                case 'f':
                    ToggleFullScreen();
                    break;

                case VK_ESCAPE:
                    DestroyWindow(hwnd);
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

    return(DefWindowProc(hwnd, iMsg, wParam ,lParam));
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
    //calls
    void resize(int, int);
    void makeSphereVertices(void);
    GLuint createVertexShader();
    GLuint createFragmentShader();
    GLuint createShaderProgram(GLuint, GLuint);

    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;
    GLenum result; 

    ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cBlueBits = 8;
    pfd.cGreenBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32;

    ghdc = GetDC(ghwnd);

    iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
    if(iPixelFormatIndex == 0) {
        return(-1);
    }
    else {
        fprintf(fptr, "ChoosePixelFormat Successful!!..\n");
    }

    if(SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE) {
        return(-2);
    }
    else {
        fprintf(fptr, "SetPixelFormat Successful!!..\n");
    }

    ghrc = wglCreateContext(ghdc);
    if(ghrc == NULL) {
        return(-3);
    }
    else {
        fprintf(fptr, "wglCreateContext Successful!!..\n");
    }

    if(wglMakeCurrent(ghdc, ghrc) == FALSE) {
        return(-4);
    }
    else {
        fprintf(fptr, "wglMakeCurrent Successful!!..\n");
    }

    result = glewInit();
    if(result != GLEW_OK) {
        return(-5);
    }
    else {
        fprintf(fptr, "GLEW Initialization Successful!!..\n");
    }

    //here we get successfully compiled ShaderObj!.
    gVertexShaderObj = createVertexShader();
    gFragmentShaderObj = createFragmentShader();

    gShaderProgramObj = createShaderProgram(gVertexShaderObj, gFragmentShaderObj);

    //Now Vao vbo
    //makeSphereVertices
    makeSphereVertices();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVerts), sphereVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArray(0);

    
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    perspectiveProjMat = mat4::identity();
    
    resize(WIN_WIDTH, WIN_HEIGHT);
    return(0);
}

GLuint createVertexShader() {

    void checkShaderCompilation(GLuint);
    GLuint shaderObj;

    shaderObj = glCreateShader(GL_VERTEX_SHADER);

    const GLchar *shaderSource = 
        "#version 460 core" \
        "\n"    \
        "in vec4 vPosition;" \
        "uniform mat4 u_mvp_matrix;" \
        "void main(void) {" \
        " gl_Position = u_mvp_matrix * vPosition;" \
        "}";

    glShaderSource(shaderObj,1 ,(const GLchar **)&shaderSource, NULL);
    glCompileShader(shaderObj);

    checkShaderCompilation(shaderObj);
    return(shaderObj);
}

GLuint createFragmentShader() {

    void checkShaderCompilation(GLuint);
    GLuint shaderObj;

    shaderObj = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar *shaderSource = 
        "#version 460 core" \
        "\n"    \
        "out vec4 FragColor;" \
        "void main(void) {" \
        " FragColor = vec4(1.0, 0.0, 1.0, 1.0);" \
        "}";

    glShaderSource(shaderObj,1 ,(const GLchar **)&shaderSource, NULL);
    glCompileShader(shaderObj);
    
    checkShaderCompilation(shaderObj);
    return(shaderObj);
}

GLuint createShaderProgram(GLuint Vert, GLuint Frag) {

    void checkProgramLinkage(GLuint);
    GLuint progObj = 0;

    progObj = glCreateProgram();

    glAttachShader(progObj, Vert);
    glAttachShader(progObj, Frag);

    //prelink Binding!
    glBindAttribLocation(progObj, AMC_ATTRIBUTE_POSITION, "vPosition");

    glLinkProgram(progObj);
    
    checkProgramLinkage(progObj);

    //Post Link Uniform
    mvpUniform = glGetUniformLocation(progObj, "u_mvp_matrix"); 

    return(progObj);

}

void checkProgramLinkage(GLuint progObj) {
    GLint iLinkStat = 0, iLogLen = 0;
    GLchar* szInfoLog = NULL;

    void uninitialize(void);

    glGetProgramiv(progObj, GL_LINK_STATUS, &iLinkStat);

    if(iLinkStat == GL_FALSE) {
        glGetShaderiv(progObj, GL_INFO_LOG_LENGTH, &iLogLen);

        if(iLogLen > 0) {
            szInfoLog = (GLchar*)malloc(iLogLen);
            GLint written;

            glGetProgramInfoLog(progObj, iLogLen, &written, szInfoLog);
            fprintf(fptr, "%s\n",szInfoLog);

            uninitialize();
            DestroyWindow(ghwnd);
            exit(0);
        }
    }
    else {
        fprintf(fptr, "Program Linked Successfully!!..\n");
    }
}

void checkShaderCompilation(GLuint shaderObj) {
    GLint iCompileStat = 0, iLogLen = 0;
    GLchar* szInfoLog = NULL;

    void uninitialize(void);

    glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &iCompileStat);

    if(iCompileStat == GL_FALSE) {
        glGetShaderiv(shaderObj, GL_INFO_LOG_LENGTH, &iLogLen);
        if(iLogLen > 0) {
            szInfoLog = (GLchar*)malloc(iLogLen);
            GLint written;

            glGetShaderInfoLog(shaderObj, iLogLen, &written, szInfoLog);
            fprintf(fptr, "%s\n",szInfoLog);

            uninitialize();
            DestroyWindow(ghwnd);
            exit(0);
        }
    }
    else {
        fprintf(fptr, "Shader Compiled Successfully!!..\n");
    }
}

void makeSphereVertices(void) {
    
    float mapValues(float, float, float, float, float);
    int i, j;
    float lat, lon;
    float r = 2.0f;
    
    for(i=0; i < totLat; i++) {
        lat = mapValues(i, 0, totLat, -M_PI, M_PI);
        for(j=0; j < totLon; j++) {
            lon = mapValues(j, 0, totLon, -2*M_PI, 2*M_PI);
            sphereVerts[ (i * totLat) + j * 3 + 0 ] = r * sin(lat) * cos(lon);
            sphereVerts[ (i * totLat) + j * 3 + 1 ] = r * sin(lat) * sin(lon);
            sphereVerts[ (i * totLat) + j * 3 + 2 ] = r * cos(lat);
            //fprintf(fptr, "%0.3f %0.3f %0.3f\n", sphereVerts[ (i * totLat) + j * 3 + 0],sphereVerts[ (i * totLat) + j * 3 + 1],sphereVerts[ (i * totLat) + j * 3 + 2]);
        }
    }

}

float mapValues(float index, float _start, float _end, float _mapStart, float _mapEnd) {

    return(_mapStart + (_mapEnd - _mapStart) * ( (index - _start) / (_end - _start)));
}

void resize(int width, int height) {
    
    if(height == 0) {
        height = 1;
    }
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    perspectiveProjMat = perspective(45.0f, (GLfloat)width/height, 0.1f, 100.0f);
}


void display(void) {

    mat4 modelViewMat = mat4::identity();
    mat4 modelViewProjMat = mat4::identity();
    mat4 translateMat = mat4::identity();
    mat4 rotateMat = mat4::identity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gShaderProgramObj);

    translateMat = translate(0.0f, 0.0f, -6.0f);
    rotateMat = rotate(fAngle, 1.0f, 0.0f, 0.0f);

    glPointSize(2.0f);

    modelViewMat *= translateMat * rotateMat;
    modelViewProjMat = perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelViewProjMat);

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, (sizeof(sphereVerts)/sizeof(GLfloat)));
    glBindVertexArray(0);

    glUseProgram(0);    
    SwapBuffers(ghdc);
}

void update(void) {
    fAngle += 0.2f;
    if(fAngle >= 360.0f) {
        fAngle = 0.0f;
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

    glUseProgram(gShaderProgramObj);
    if(vbo) {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }
    if(vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    
    glDetachShader(gShaderProgramObj, gFragmentShaderObj);
    glDetachShader(gShaderProgramObj, gVertexShaderObj);
    glDeleteShader(gFragmentShaderObj);
    gFragmentShaderObj = 0;

    glDeleteShader(gVertexShaderObj);
    gVertexShaderObj = 0;

    glDeleteProgram(gShaderProgramObj);
    gShaderProgramObj= 0;

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
