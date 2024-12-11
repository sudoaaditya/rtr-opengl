#include<windows.h>
#include<stdio.h>
#include<gl/glew.h>
#include<gl/GL.h>
#include "./glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3
#include "glm/vec4.hpp" // glm::vec4
#include "glm/mat4x4.hpp" // glm::mat4
#include "glm/gtc/matrix_transform.hpp" // glm::translate, glm::rotate, glm::scale, glm::perspective
#include "glm/gtc/type_ptr.hpp"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define MY_PI  3.14159265358979323846f

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
GLuint vao, vboVert, vboNorm, vboEle;
GLuint mvpUniform;
glm::mat4 perspectiveProjMat;
GLfloat fRotAngle = 0.0f;

//Sphere
int sphereVertices = 0;

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
                        TEXT("PP_PerspectiveTriangle"),
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
    void Sphere(GLfloat fRadius, GLint iSlices, GLint iStacks);

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
                    "uniform mat4 u_mvp_matrix;" \
                    "void main(void)" \
                    "{" \
                    "gl_Position = u_mvp_matrix * vPosition;" \
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
                    "void main(void)" \
                    "{" \
                    "FragColor = vec4(1.0, 1.0, 1.0, 1.0);" \
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

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    Sphere(2.0f, 100, 100);

    perspectiveProjMat = glm::mat4(1.0f);

    resize(WIN_WIDTH, WIN_HEIGHT);

    return(0);
}

void resize(int width, int height){

    if(height == 0) {
        height = 1;
    }

    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    perspectiveProjMat = glm::perspective(glm::radians(45.0f),(GLfloat)width / (GLfloat)height,0.1f,1000.0f);   
}

void display(void) {


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gProgramShaderObj);

    glm::mat4 modelViewMatrix = glm::mat4( 1.0f );
	glm::mat4 modelViewProjectionMatrix = glm::mat4( 1.0f );
	glm::mat4 TranslationMatrix=glm::mat4( 1.0f );
    glm::mat4 rotateMat = glm::mat4(1.0f);

	TranslationMatrix = glm::translate( glm::mat4( 1.0f ), glm::vec3(0.0f, 0.0f, -3.0f));
    //rotateMat = glm::rotate(glm::mat4(1.0f), fRotAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	modelViewMatrix = modelViewMatrix * TranslationMatrix;
    //modelViewMatrix = modelViewMatrix * rotateMat;
	modelViewProjectionMatrix = perspectiveProjMat * modelViewMatrix;

	glUniformMatrix4fv(mvpUniform, 1, GL_FALSE,  glm::value_ptr(modelViewProjectionMatrix));

	glBindVertexArray(vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboEle);
	glDrawElements(GL_TRIANGLES, 202, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);

    glUseProgram(0);

    SwapBuffers(ghdc);
}

void update(void) {
    fRotAngle += 0.2f;
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


/* Sphere Concerned CODE!!. */

void Sphere(GLfloat fRadius, GLint iSlices, GLint iStacks) {

    void generateSphereData(GLfloat, GLint, GLint, GLfloat**, GLfloat**, GLint*);
    void makeSphereBuffersToDraw(GLfloat *vertices, GLfloat *normals, GLushort *vertIndx, int numVert, int, int);

    int i, j, numVertices, indx;
    GLfloat *vertices, *normals;

    fprintf(fptr, "Called Sphere!");

    generateSphereData(fRadius, iSlices, iStacks, &vertices, &normals, &numVertices);

    if(numVertices ==0) {
        return;
    }

    GLushort *stripIndex;
    GLushort offset;
    int elecount = 0;

    stripIndex = (GLushort*)malloc((iSlices+1)*2*(iStacks)*sizeof(GLushort));
    if(!(stripIndex)){
        free(stripIndex);
        return;
    }

    //create array for vertex index so that we can draw using draw elements.
    for(j=0, indx=0; j<iSlices; j++, indx +=2) {
        stripIndex[indx ] = j+1;
        stripIndex[indx+1] = 0;
        elecount++;
    }
    stripIndex[indx ] = 1;
    stripIndex[indx+1] = 0;
    indx += 2;
    elecount+=2;


    for (i=0; i<-2; i++, indx+=2)
        {
            offset = 1+i*iSlices;                    
            for (j=0; j<iSlices; j++, indx+=2)
            {
                stripIndex[indx  ] = offset+j+iSlices;
                stripIndex[indx+1] = offset+j;
                elecount++;
            }
            stripIndex[indx  ] = offset+iSlices;        
            stripIndex[indx+1] = offset;
            elecount +=2;
        }

        offset = 1+(iStacks-2)*iSlices;   
        for (j=0; j<iSlices; j++, indx+=2)
        {
            stripIndex[indx  ] = numVertices-1;              
            stripIndex[indx+1] = offset+j;
            elecount++;
        }
        stripIndex[indx  ] = numVertices-1;        
        stripIndex[indx+1] = offset;
        elecount += 2;

    makeSphereBuffersToDraw(vertices, normals, stripIndex, numVertices, iStacks, (iSlices+1*2));

}

void generateSphereData(GLfloat radius, GLint slices, GLint stacks, GLfloat** vertices, GLfloat** normals, GLint* numVert) {

    void calculateCircles(GLfloat**, GLfloat**, const int, const GLboolean);

    fprintf(fptr, "Called Sphere Data!");
    int i,j, indx = 0;
    GLfloat x,y,z;

    GLfloat *sint1, *sint2, *cost1, *cost2;

    if (slices==0 || stacks<2)
    {
        *numVert = 0;
        return;
    }
    *numVert = slices*(stacks-1)+2;

    calculateCircles(&sint1,&cost1,-slices,GL_FALSE);
    calculateCircles(&sint2,&cost2, stacks,GL_TRUE);

    *vertices = (GLfloat*)malloc((*numVert)*3*sizeof(GLfloat));
    *normals  = (GLfloat*)malloc((*numVert)*3*sizeof(GLfloat));
    
    if (!(*vertices) || !(*normals))
    {
        free(*vertices);
        free(*normals);
    }

    /* top */
    (*vertices)[0] = 0.f;
    (*vertices)[1] = 0.f;
    (*vertices)[2] = radius;
    (*normals )[0] = 0.f;
    (*normals )[1] = 0.f;
    (*normals )[2] = 1.f;
    indx = 3;

    /* each stack */
    for( i=1; i<stacks; i++ )
    {
        for(j=0; j<slices; j++, indx+=3)
        {
            x = cost1[j]*sint2[i];
            y = sint1[j]*sint2[i];
            z = cost2[i];

            fprintf(fptr, "%f, %f, %f\n", x,y,z);

            (*vertices)[indx  ] = x*radius;
            (*vertices)[indx+1] = y*radius;
            (*vertices)[indx+2] = z*radius;
            (*normals )[indx  ] = x;
            (*normals )[indx+1] = y;
            (*normals )[indx+2] = z;
        }
    }

    /* bottom */
    (*vertices)[indx  ] =  0.f;
    (*vertices)[indx+1] =  0.f;
    (*vertices)[indx+2] = -radius;
    (*normals )[indx  ] =  0.f;
    (*normals )[indx+1] =  0.f;
    (*normals )[indx+2] = -1.f;

    free(sint1);
    free(cost1);
    free(sint2);
    free(cost2);
}

void calculateCircles(GLfloat** sint, GLfloat** cost, const int n , const GLboolean halfCircle) {

    int i;
    const int size = glm::abs(n);

    const GLfloat angle = (halfCircle?1:2)*(GLfloat)MY_PI/(GLfloat)( ( n == 0 ) ? 1 : n );

    *sint = (GLfloat*)malloc(sizeof(GLfloat) * (size+1));
    *cost = (GLfloat*)malloc(sizeof(GLfloat) * (size+1));
    if (!(*sint) || !(*cost))
    {
        free(*sint);
        free(*cost);
        return;
    }

    (*sint)[0] = 0.0;
    (*cost)[0] = 1.0;

    for (i=1; i<size; i++)
    {
        (*sint)[i] = (GLfloat)glm::sin(angle*i);
        (*cost)[i] = (GLfloat)glm::cos(angle*i);
    }
    
    if (halfCircle)
    {
        (*sint)[size] =  0.0f;  
        (*cost)[size] = -1.0f;  
    }
    else
    {
        (*sint)[size] = (*sint)[0];
        (*cost)[size] = (*cost)[0];
    }
}

void makeSphereBuffersToDraw(GLfloat *vertices, GLfloat *normals, GLushort *vertIndx, int numVert, int elements) {

    sphereVertices = numVert;
    
    fprintf(fptr, "Called Prepare Sphere!");
    fprintf(fptr, "\nVerNo: %d",numVert);
    fprintf(fptr, "\nVerEle No: %d",elements);
    

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vboVert);
    glBindBuffer(GL_ARRAY_BUFFER, vboVert);
    glBufferData(GL_ARRAY_BUFFER, (sphereVertices*3*4), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);       
    glBindVertexArray(0);

    glGenBuffers(1, &vboNorm);
    glBindBuffer(GL_ARRAY_BUFFER, vboNorm);
    glBufferData(GL_ARRAY_BUFFER, (sphereVertices*3*4), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);       
    glBindVertexArray(0);

    glGenBuffers(1, &vboEle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboEle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (elements*3*sizeof(GLushort)), vertIndx, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    
}
