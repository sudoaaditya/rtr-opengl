//System Files
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

//XWindows Header File
#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>

// OGL Header File
#include<GL/glew.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include"./../vmath/vmath.h"

using namespace std;
using namespace vmath;

//enum
enum {
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXTURE0
};

// Global Variables
bool bFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisual = NULL;
Colormap gColormap;
Window gWindow;
int gWinWidth = 800;
int gWinHeight = 600;

//FBConfig Variables
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

glXCreateContextAttribsARBProc glXCreateContextAttribsARB = NULL;
GLXFBConfig gGLXFBConfig;
GLXContext gGlxContext;

//Vars for shaders
GLuint gShaderProgObj;
GLuint gVertexShaderObj;
GLuint gFragmentShaderObj;
GLuint vao, vboAxes, vaoHor, vaoVert, vboLinesHor, vboLinesVert, vaoHorDwn, vboLinesHorDwn, vaoVertDwn, vboLinesVertDwn;
GLuint vboCol1;
GLuint vaoTriangle, vboTrianglePos, vboTriangleCol;
GLuint vaoRectangle, vboRectanglePos, vboRectangleCol;
GLuint vaoCircle, vboCirclePos, vboCircleCol;
GLuint vaoOutCirc, vboOutCircPos, vboOutCircCol;
GLuint mvpUniform;
mat4 perspectiveProjMat;
int circleLen;


//MAIN
 int main(void) {


	 printf(" SUCCESS : Code Initiated!!..\n\n");
	 
	 //func
	 int initialize(void);
	 void uninitialize(void);
	 void display(void);
	 void ToggleFullScreen(void);
	 void CreateWindow(void);
	 void resize(int, int);

	 // Vars
	 static int iWinWidth = gWinWidth;
	 static int iWinHeight = gWinHeight;
	 bool bDone = false;

	 // Create Window
	 CreateWindow();
    
	 // Now Initialize Window
	 if(initialize() == 0) {
		 printf(" SUCCESS : Initialization Successful!.\n");
	 }
	 else {
		 printf(" ERROR : Initialization Failed!!..\n");
		 uninitialize();
		 exit(0);
	 }

    ToggleFullScreen();
    
	 // Message Loop
	 XEvent event;
	 KeySym keysym;
	 char keys[26];

	 while(!bDone) {

		 while(XPending(gpDisplay)) {

			 XNextEvent(gpDisplay, &event);

			 switch(event.type) {
				 case MapNotify:
					 printf(" MessageLoop: Map Notify called!.\n");
					 break;

				 case KeyPress:
					 keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
					 switch(keysym) {
						 case XK_Escape:
							 bDone = true;
							 break;

						  default:
							 break;
					 }

					 XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);
					 switch(keys[0]) {
						 case 'F':
						 case 'f':
							 if(!bFullScreen) {
								 ToggleFullScreen();
								 bFullScreen = true;
							 }
							 else {
								 ToggleFullScreen();
								 bFullScreen = false;
							 }
							 break;
					 }
					 break;

				 case ButtonPress:
					 switch(event.xbutton.button) {

						 case 1:
							 break;
						 case 2:
							 break;
						 case 3:
							 break;
						 default:
							 break;
					 }
					 break;

				 case MotionNotify:
					 break;

				case ConfigureNotify:
					 iWinWidth = event.xconfigure.width;
					 iWinHeight = event.xconfigure.height;

					 resize(iWinWidth, iWinHeight);
					 break;

				 case Expose:
					 break;

				 case DestroyNotify:
					 break;

				 case 33:
					 bDone = true;
					 break;

				 default:
					 break;

			 }
		 }
		display();
	 }
	uninitialize();
	return(0);
}

void CreateWindow(void) {

	void uninitialize(void);

	//vars
	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int styleMask;
    //FBConfig Vars
    GLXFBConfig *pGLXFBConfig = NULL;
    GLXFBConfig bestGLXFBConfig;
    XVisualInfo *pTempXVisual = NULL;
    int iNuFBConfig = 0;

	//OGL Frame Buffer attributes
	static int frameBufferAttributes[] = { 
                    GLX_X_RENDERABLE,   True,
                    GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
                    GLX_RENDER_TYPE,    GLX_RGBA_BIT,
                    GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
                    GLX_RED_SIZE,       8,
                    GLX_GREEN_SIZE,     8,
                    GLX_BLUE_SIZE,      8,
                    GLX_ALPHA_SIZE,     8,
                    GLX_DEPTH_SIZE,     24,
                    GLX_STENCIL_SIZE,   8,
                    GLX_DOUBLEBUFFER,   True,
                    None };


	gpDisplay = XOpenDisplay(NULL);
	if(gpDisplay == NULL) {
		printf(" ERROR : Unable To Open X Display.\n Exiting Now!!..");
		uninitialize();
		exit(0);
	}
	else {
		printf(" SUCCESS : Opened Display!..\n");
	}

	defaultScreen = XDefaultScreen(gpDisplay);

    //..!! FBConfig !!..
    pGLXFBConfig = glXGetFBConfigs(gpDisplay, defaultScreen, &iNuFBConfig);

    int bestFrameBufferConfig = -1;
    int bestNumberOfSamples = -1;
    int worstFrameBufferConfig = -1;
    int worstNumberOfSamples = 999;

    for(int i = 0; i< iNuFBConfig; i++) {

        pTempXVisual = glXGetVisualFromFBConfig(gpDisplay, pGLXFBConfig[i]);

        if(pTempXVisual) {
            int iSampleBuff, iSamples;

            glXGetFBConfigAttrib(gpDisplay, pGLXFBConfig[i], GLX_SAMPLE_BUFFERS, &iSampleBuff);
            
            glXGetFBConfigAttrib(gpDisplay, pGLXFBConfig[i], GLX_SAMPLES, &iSamples);

            if(bestFrameBufferConfig < 0 || iSampleBuff && iSamples > bestNumberOfSamples) {
                bestFrameBufferConfig = i;
                bestNumberOfSamples = iSamples;
            }

            if(worstFrameBufferConfig < 0 || !iSampleBuff || iSamples < worstNumberOfSamples) {
                worstFrameBufferConfig = i;
                worstNumberOfSamples = iSamples;
            }
        }
        XFree(pTempXVisual);
        pTempXVisual = NULL;
    }

    bestGLXFBConfig = pGLXFBConfig[bestFrameBufferConfig];
    gGLXFBConfig = bestGLXFBConfig;

    XFree(pGLXFBConfig);

    gpXVisual = glXGetVisualFromFBConfig(gpDisplay, bestGLXFBConfig);

	winAttribs.border_pixel = 0;
	winAttribs.border_pixmap = 0;
	winAttribs.background_pixmap = 0;
	winAttribs.backing_pixel = BlackPixel(gpDisplay, defaultScreen);
	winAttribs.colormap = XCreateColormap(gpDisplay,
					      RootWindow(gpDisplay, gpXVisual->screen),
					      gpXVisual->visual,
					      AllocNone);
	gColormap = winAttribs.colormap;
	winAttribs.event_mask = ExposureMask | VisibilityChangeMask |
				ButtonPressMask | KeyPressMask |
				PointerMotionMask | StructureNotifyMask;
	
	styleMask = CWBorderPixmap | CWBackPixel | CWEventMask | CWColormap;

	
	gWindow = XCreateWindow(gpDisplay,
				RootWindow(gpDisplay, gpXVisual->screen),
				0, 0,
				gWinWidth, gWinHeight,
				0,
				gpXVisual->depth,
				InputOutput,
				gpXVisual->visual,
				styleMask,
				&winAttribs);

	if(!gWindow) {
		printf(" ERROR : Failed To CreateWinodw.\n Exiting Now!!..");
		uninitialize();
		exit(0);
	}
	else {
		printf(" SUCCESS : Window Created Successfully!..\n");
	}

	XStoreName(gpDisplay, gWindow, "Mixed Geometry");

	Atom wndMgrDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(gpDisplay, gWindow, &wndMgrDelete, 1);

	XMapWindow(gpDisplay, gWindow);

}

void ToggleFullScreen(void) {

	Atom wm_state;
	Atom fullscreen;
	XEvent xeve = {0};

	//code
	wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
	memset(&xeve, 0, sizeof(XEvent));

	xeve.type = ClientMessage;
	xeve.xclient.window = gWindow;
	xeve.xclient.message_type = wm_state;
	xeve.xclient.format = 32;
	xeve.xclient.data.l[0] = bFullScreen ? 0 : 1;

	fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	xeve.xclient.data.l[1] = fullscreen;

	XSendEvent(gpDisplay,
		   RootWindow(gpDisplay, gpXVisual->screen),
		   False,
		   StructureNotifyMask,
		   &xeve);

}

int initialize(void) {

	GLenum result;

    //shader log lines..
    GLint iShaderCompileStat = 0;
    GLint iShaderInfoLogLen = 0;
    GLchar* shaderInfoLog = NULL; 
    
    //For inCircle Calcus!
    GLfloat fx1, fx2, fx3, fy1, fy2, fy3;
	GLfloat fArea, fRad, fPer;
	GLfloat fdAB, fdBC, fdAC;
	GLfloat fxCord, fyCord;
	static GLfloat fAngle = 0.0f;
    
	void uninitialize(void);
	void resize(int, int);
	
    // Getting Renderable Context
    glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte *)"glXCreateContextAttribsARB");
	if(glXCreateContextAttribsARB == NULL) {
		printf(" ERROR : Failed to get address of glXCreateContextAttribsARB\n Exiting Now!.. ");
		uninitialize();
		exit(0);
	}

	const int attrib[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
		GLX_CONTEXT_MINOR_VERSION_ARB, 5,
		GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None};

	gGlxContext = glXCreateContextAttribsARB(gpDisplay, gGLXFBConfig, 0, True, attrib);
	
	if(!gGlxContext) {
		const int attrib[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 1,
		GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		None};

		gGlxContext = glXCreateContextAttribsARB(gpDisplay, gGLXFBConfig, 0, True, attrib);
	}

	if(!glXIsDirect(gpDisplay, gGlxContext)) {
		printf(" WARNING : Context is not Hardware Rendering!..\n");
	}
	else {
		printf(" SUCCESS : Obtained context is Hardware Rendering.\n");
	}

	glXMakeCurrent(gpDisplay, gWindow, gGlxContext);


	if(glXMakeCurrent(gpDisplay, gWindow, gGlxContext) != True) {
		return(-1);
	}else {
		printf(" SUCCESS : Set as Current GLXContext!..\n");
	}

	result = glewInit();
	if(result != GLEW_OK) {
		printf(" ERROR : Failed To Initialize GLEW.\n Exiting Now!!..");
		uninitialize();
		exit(0);
	}
	else {
		printf(" SUCCESS : GLEW Initialization successful..\n");
	}

    //shaders

    gVertexShaderObj = glCreateShader(GL_VERTEX_SHADER);

    const GLchar *vertexShaderSource = 
                    "#version 450 core" \
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
    

    glShaderSource(gVertexShaderObj, 1, (const GLchar**)&vertexShaderSource, NULL);

    glCompileShader(gVertexShaderObj);

    glGetShaderiv(gVertexShaderObj, GL_COMPILE_STATUS, &iShaderCompileStat);

    if(iShaderCompileStat == GL_FALSE) {

        glGetShaderiv(gVertexShaderObj, GL_INFO_LOG_LENGTH, &iShaderInfoLogLen);

        if(iShaderInfoLogLen > 0) {

            GLint written;

            shaderInfoLog = (GLchar*)malloc(iShaderInfoLogLen);
            if(shaderInfoLog) {

                glGetShaderInfoLog(gVertexShaderObj, iShaderInfoLogLen, &written, shaderInfoLog);

                printf(" ERROR : Vertex Shader Log\n\t %s", shaderInfoLog);

                free(shaderInfoLog);
                uninitialize();
                exit(0);
            }
        }
    }
    else {
        printf(" SUCCESS : Vertex Shader Compiled Successfully!!..\n");
    }

    iShaderCompileStat = 0;
    iShaderInfoLogLen = 0;
    shaderInfoLog = NULL;

    gFragmentShaderObj = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar* fragmentShaderSource =
                "#version 450 core" \
                "\n" \
                "in vec4 out_color;" \
                "out vec4 FragColor;" \
                "void main(void)" \
                "{" \
                "FragColor = out_color;" \
                "}";   

    glShaderSource(gFragmentShaderObj, 1, (const GLchar**)&fragmentShaderSource, NULL);

    glCompileShader(gFragmentShaderObj);
    glGetShaderiv(gFragmentShaderObj, GL_COMPILE_STATUS, &iShaderCompileStat);

    if(iShaderCompileStat == GL_FALSE) {

        glGetShaderiv(gFragmentShaderObj, GL_INFO_LOG_LENGTH, &iShaderInfoLogLen);

        if(iShaderInfoLogLen > 0) {

            GLint written;

            shaderInfoLog = (GLchar*)malloc(iShaderInfoLogLen);
            if(shaderInfoLog) {

                glGetShaderInfoLog(gFragmentShaderObj, iShaderInfoLogLen, &written, shaderInfoLog);

                printf(" ERROR : Fragment Shader Log\n\t %s", shaderInfoLog);

                free(shaderInfoLog);
                uninitialize();
                exit(0);
            }
        }
    }
    else {
        printf(" SUCCESS : Fragment Shader Compiled Successfully!!..\n");
    }

    GLint iProgLinkStat = 0;
    GLint iProgLogLen = 0;
    GLchar* progInfoLog = NULL;

    gShaderProgObj = glCreateProgram();

    glAttachShader(gShaderProgObj, gVertexShaderObj);
    glAttachShader(gShaderProgObj, gFragmentShaderObj);

    //PreLink Bind

    glBindAttribLocation(gShaderProgObj, AMC_ATTRIBUTE_POSITION, "vPosition");
    glBindAttribLocation(gShaderProgObj, AMC_ATTRIBUTE_COLOR, "vColor");

    glLinkProgram(gShaderProgObj);

    glGetProgramiv(gShaderProgObj, GL_LINK_STATUS, &iProgLinkStat);

    if(iProgLinkStat == GL_FALSE) {

        glGetProgramiv(gShaderProgObj, GL_INFO_LOG_LENGTH, &iProgLogLen);

        if(iProgLogLen > 0) {

            GLint written;

            progInfoLog = (GLchar*)malloc(iProgLogLen);
            if(progInfoLog) {

                glGetProgramInfoLog(gShaderProgObj, iProgLogLen, &written, progInfoLog);

                printf(" ERROR : Fragment Shader Log\n\t %s", progInfoLog);

                free(progInfoLog);
                uninitialize();
                exit(0);
            }
        }
    }
    else {
        printf(" SUCCESS : Prograam Linked Successfully!!..\n");
    }

    //Post Link Uniform Binding
    mvpUniform = glGetUniformLocation(gShaderProgObj, "u_mvp_matrix");

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

	printf("CALCUS :: \nfdAB = %f\nfdBC = %f\nfdAC = %f\nfPer =  %f\nfArea = %f\nfRad = %f\n", fdAB, fdBC, fdAC, fPer, fArea, fRad);
	

	fxCord = (GLfloat)(((fdBC*fx1) + (fx2*fdAC) + (fx3*fdAB)) / (fPer * 2));
	fyCord = (GLfloat)(((fdBC*fy1) + (fy2*fdAC) + (fy3*fdAB)) / (fPer * 2));

	printf("\n\n CASCUS :: \nfx1 = %f\tfy1 = %f\nfx2 = %f\tfy2 = %f\nfx3 = %f\tfy3 = %f\n\nfxCoord = %f\tfyCord = %f\n", fx1, fy1, fx2, fy2, fx3, fy3, fxCord, fyCord);
        
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

	resize(gWinWidth, gWinHeight);

	return(0);

}

void resize(int width, int height) {

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

    glUseProgram(gShaderProgObj);

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

	glXSwapBuffers(gpDisplay, gWindow);

}


void uninitialize(void) {
    
    if(vboAxes) {
        glDeleteBuffers(1, &vboAxes);
        vboAxes = 0;
    }
    if(vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    //Safe Release to Shader!!.
    if(gShaderProgObj) {
        GLsizei iShaderCnt = 0;
        GLsizei iShaderNo = 0;

        glUseProgram(gShaderProgObj);

        glGetProgramiv(gShaderProgObj, 
                        GL_ATTACHED_SHADERS, 
                        &iShaderCnt);

        GLuint *pShaders = (GLuint*)malloc(iShaderCnt * sizeof(GLuint));

        if(pShaders) {
            glGetAttachedShaders(gShaderProgObj,
                                 iShaderCnt,
                                 &iShaderCnt,
                                 pShaders);

            for(iShaderNo = 0; iShaderNo < iShaderCnt; iShaderNo++) {

                glDetachShader(gShaderProgObj, pShaders[iShaderNo]);
                pShaders[iShaderNo] = 0;
            }
            free(pShaders);
        }
        glDeleteProgram(gShaderProgObj);
        gShaderProgObj = 0;

        glUseProgram(0);
    }

    GLXContext current = glXGetCurrentContext();

	if(current != NULL && current == gGlxContext) {

		glXMakeCurrent(gpDisplay, 0, 0);
	}

	if(gGlxContext) {
		glXDestroyContext(gpDisplay, gGlxContext);
		gGlxContext = NULL;
	}

	if(gWindow) {
		XDestroyWindow(gpDisplay, gWindow);
	
	}

	if(gColormap) {
		XFreeColormap(gpDisplay, gColormap);
	
	}

	if(gpXVisual) {
		free(gpXVisual);
		gpXVisual = NULL;
	}

	if(gpDisplay) {
		XCloseDisplay(gpDisplay);
		gpDisplay = NULL;
	}
	
	printf("\n SUCCESS : Code Termination Successfully!!..\n\n");
}

