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
GLuint vaoPyramid, vboPyramidPos, vboPyramidNormal;
mat4 perspectiveProjMat;
//Uniforms
GLuint modelUniform;
GLuint viewUniform;
GLuint projUnifrom;

GLuint laOneUniform;
GLuint ldOneUniform;
GLuint lsOneUniform;
GLuint lightPosOneUniform;

GLuint laTwoUniform;
GLuint ldTwoUniform;
GLuint lsTwoUniform;
GLuint lightPosTwoUniform;

GLuint kaUniform;
GLuint kdUniform;
GLuint ksUniform;
GLuint matShineUniform;
GLuint lPressedUnifrom;

//Light Parameters Right Pars:: Red Light!.
GLfloat lightAmbientRed[] = {0.0f, 0.0f, 0.0f, 0.0f};
GLfloat lightDiffuseRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightSpecularRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightPositionRed[] = {2.0f, 0.0f, 0.0f, 1.0f};
//Light For Left Pars:: Blue Light
GLfloat lightAmbientBlue[] = {0.0f, 0.0f, 0.0f, 0.0f};
GLfloat lightDiffuseBlue[] = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightSpecularBlue[] = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightPositionBlue[] = {-2.0f, 0.0f, 0.0f, 1.0f};

GLfloat materialAmbient[] = {0.0f, 0.0f, 0.0f, 0.0f};
GLfloat materialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialShininess = 128.0f;
bool bLighting = false;
GLfloat fRotAngle = 0.0f;



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

    if(fopen_s(&fptr, "_ADS_PerVertexLight.txt", "w") != 0) {
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
                        TEXT("PP_ADS_LIGHT!"),
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

                case 'L':
                case 'l':
                    if(!bLighting){
                        bLighting = true;
                    }
                    else {
                        bLighting = false;
                    }
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

    gVertexShaderObj = glCreateShader(GL_VERTEX_SHADER);

    const GLchar *vertexShaderSourceCode = 
                    "#version 460 core" \
                    "\n" \
                    "in vec4 vPosition;" \
                    "in vec3 vNormal;" \
                    "uniform mat4 u_modelMat;" \
                    "uniform mat4 u_viewMat;" \
                    "uniform mat4 u_projMat;" \
                    "uniform int u_lPressed;" \
                    "uniform vec3 u_laOne;" \
                    "uniform vec3 u_ldOne;" \
                    "uniform vec3 u_lsOne;" \
                    "uniform vec4 u_lightPosOne;" \
                    "uniform vec3 u_laTwo;" \
                    "uniform vec3 u_ldTwo;" \
                    "uniform vec3 u_lsTwo;" \
                    "uniform vec4 u_lightPosTwo;" \
                    "uniform vec3 u_ka;" \
                    "uniform vec3 u_kd;" \
                    "uniform vec3 u_ks;" \
                    "uniform float u_matShine;" \
                    "out vec3 phong_ads_light;" \
                    "void main(void) {" \
                    "   if(u_lPressed == 1) {" \
                    "       vec4 eyeCoords = u_viewMat * u_modelMat * vPosition;" \
                    "       vec3 t_normal = normalize(mat3(u_viewMat * u_modelMat) * vNormal);" \

                    "       vec3 lightDirectionOne = normalize(vec3(u_lightPosOne - eyeCoords));" \
                    "       float tn_dot_ldOne = max(dot(lightDirectionOne, t_normal), 0.0);" \
                    "       vec3 reflectionVectorOne = reflect(-lightDirectionOne, t_normal);" \

                    "       vec3 lightDirectionTwo = normalize(vec3(u_lightPosTwo - eyeCoords));" \
                    "       float tn_dot_ldTwo = max(dot(lightDirectionTwo, t_normal), 0.0);" \
                    "       vec3 reflectionVectorTwo = reflect(-lightDirectionTwo, t_normal);" \

                    "       vec3 viewerVector = normalize(vec3(-eyeCoords.xyz));" \

                    "       vec3 ambientOne = u_laOne * u_ka;" \
                    "       vec3 diffuseOne = u_ldOne * u_kd * tn_dot_ldOne;" \
                    "       vec3 specularOne = u_lsOne * u_ks * pow(max(dot(reflectionVectorOne, viewerVector), 0.0), u_matShine);" \

                    "       vec3 ambientTwo = u_laTwo * u_ka;" \
                    "       vec3 diffuseTwo = u_ldTwo * u_kd * tn_dot_ldTwo;" \
                    "       vec3 specularTwo = u_lsTwo * u_ks * pow(max(dot(reflectionVectorTwo, viewerVector), 0.0), u_matShine);" \

                    "       phong_ads_light = ambientOne + ambientTwo +diffuseOne + diffuseTwo +specularOne + specularTwo;" \
                    "   }" \
                    "   else {" \
                    "       phong_ads_light = vec3(1.0, 1.0, 1.0);" \
                    "   }" \
                    "   gl_Position = u_projMat * u_viewMat * u_modelMat * vPosition;"  \
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
                    "out vec4 FragColor;" \
                    "in vec3 phong_ads_light;"
                    "void main(void)" \
                    "{" \
                    "FragColor = vec4(phong_ads_light, 1.0);" \
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
    glBindAttribLocation(gProgramShaderObj, AMC_ATTRIBUTE_NORMAL, "vNormal");
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

    modelUniform = glGetUniformLocation(gProgramShaderObj, "u_modelMat");
    viewUniform = glGetUniformLocation(gProgramShaderObj, "u_viewMat");
    projUnifrom = glGetUniformLocation(gProgramShaderObj, "u_projMat");
    laOneUniform = glGetUniformLocation(gProgramShaderObj, "u_laOne");
    ldOneUniform = glGetUniformLocation(gProgramShaderObj, "u_ldOne");
    lsOneUniform = glGetUniformLocation(gProgramShaderObj, "u_ls");
    lightPosOneUniform = glGetUniformLocation(gProgramShaderObj, "u_lightPosOne");

    laTwoUniform = glGetUniformLocation(gProgramShaderObj, "u_laTwo");
    ldTwoUniform = glGetUniformLocation(gProgramShaderObj, "u_ldTwo");
    lsTwoUniform = glGetUniformLocation(gProgramShaderObj, "u_lsTwo");
    lightPosTwoUniform = glGetUniformLocation(gProgramShaderObj, "u_lightPosTwo");
    
    kaUniform = glGetUniformLocation(gProgramShaderObj, "u_ka");
    kdUniform = glGetUniformLocation(gProgramShaderObj, "u_kd");
    ksUniform = glGetUniformLocation(gProgramShaderObj, "u_ks");
    matShineUniform = glGetUniformLocation(gProgramShaderObj, "u_matShine");
    lPressedUnifrom = glGetUniformLocation(gProgramShaderObj, "u_lPressed");
    


    GLfloat pyramidVertices[] = { 
                0.0f, 1.0f, 0.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
                0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
                0.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
                0.0f, 1.0f, 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f
            };

    GLfloat pyramidNormals[] = {
        0.0f, 0.447214f, 0.894427f, 0.0f, 0.447214f, 0.894427f, 0.0f, 0.447214f, 0.894427f, 
        0.894427f, 0.447214f, 0.0f, 0.894427f, 0.447214f, 0.0f, 0.894427f, 0.447214f, 0.0f, 
        0.0f, 0.447214f, -0.894427f, 0.0f, 0.447214f, -0.894427f, 0.0f, 0.447214f, -0.894427f, 
        -0.894427f, 0.447214f, 0.0f, -0.894427f, 0.447214f, 0.0f, -0.894427f, 0.447214f, 0.0f
    };

    glGenVertexArrays(1, &vaoPyramid);
    glBindVertexArray(vaoPyramid);

    glGenBuffers(1, &vboPyramidPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPyramidPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);        // unbind VBO!.

    glGenBuffers(1, &vboPyramidNormal);
    glBindBuffer(GL_ARRAY_BUFFER, vboPyramidNormal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);   //unbind VAO!.

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

    //Declare MAtrices
    mat4 modelMat;
    mat4 viewMat;
    mat4 rotateMat;
    mat4 translateMat;

    //Initialization of matrices
    modelMat = mat4::identity();
    viewMat = mat4::identity();
    rotateMat = mat4::identity();
    translateMat = mat4::identity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gProgramShaderObj);

    translateMat = translate(0.0f, 0.0f, -5.0f);
    rotateMat= rotate(fRotAngle, 0.0f, 1.0f, 0.0f);
    modelMat = translateMat * rotateMat;
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMat);
    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMat);
    glUniformMatrix4fv(projUnifrom, 1, GL_FALSE, perspectiveProjMat);

    if(bLighting == true) {
        glUniform3fv(laOneUniform, 1, lightAmbientRed);
        glUniform3fv(ldOneUniform, 1, lightDiffuseRed);
        glUniform3fv(lsOneUniform, 1, lightSpecularRed);
        glUniform4fv(lightPosOneUniform, 1, lightPositionRed);

        glUniform3fv(laTwoUniform, 1, lightAmbientBlue);
        glUniform3fv(ldTwoUniform, 1, lightDiffuseBlue);
        glUniform3fv(lsTwoUniform, 1, lightSpecularRed);
        glUniform4fv(lightPosTwoUniform, 1, lightPositionBlue);

        glUniform3fv(kaUniform, 1, materialAmbient);
        glUniform3fv(kdUniform, 1, materialDiffuse);
        glUniform3fv(ksUniform, 1, materialSpecular);
        glUniform1f(matShineUniform, materialShininess);
        glUniform1i(lPressedUnifrom, 1);
    }
    else {
        glUniform1i(lPressedUnifrom, 0);
    }

    glBindVertexArray(vaoPyramid);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    glBindVertexArray(0);
    
    glUseProgram(0);

    SwapBuffers(ghdc);
}

void update(void) {
    fRotAngle += 0.1f;
    if(fRotAngle >= 360.0f) {
        fRotAngle = 0.0f;
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

    //UnSetting All We Did For Shaders
    //let make use of shader obj!.
    glUseProgram(gProgramShaderObj);
    //Delete VBO
    if(vboPyramidPos) {
        glDeleteBuffers(1, &vboPyramidPos);
        vboPyramidPos = 0;
    }
    if(vboPyramidNormal) {
        glDeleteBuffers(1, &vboPyramidNormal);
        vboPyramidNormal = 0;
    }
    //DeleteVAO
    if(vaoPyramid) {
        glDeleteVertexArrays(1, &vaoPyramid);
        vaoPyramid = 0;
    }
    
    glDetachShader(gProgramShaderObj, gFragmentShaderObj);
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