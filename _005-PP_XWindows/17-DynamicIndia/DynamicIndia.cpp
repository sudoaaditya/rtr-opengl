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

//for AUDIO
#include<AL/al.h>
#include<AL/alc.h>
#include<AL/alut.h>

using namespace std;
using namespace vmath;

//Color MAcros
//Saffron 255 153 51
#define SAFFRON_R 1.0f
#define SAFFRON_G 0.600f
#define SAFFRON_B 0.20f
//White 255 255 255
#define WHITE_R 1.0f
#define WHITE_G 1.0f
#define WHITE_B 1.0f
//Green 18 136 7
#define GREEN_R 0.070f
#define GREEN_G 0.533f
#define GREEN_B 0.027f
//Blue 0 0 136
#define BLUE_R 0.0f
#define BLUE_G 0.0f
#define BLUE_B 0.533f


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
GLuint mvpUniform;
mat4 perspectiveProjMat;
GLuint vaoI1, vaoN, vaoD, vaoI2, vaoA, vaoDArc;
GLuint vboI1Pos, vboI1Col, vboNPos, vboNCol, vboDPos, vboDCol, vboI2Pos, vboI2Col, vboAPos, vboACol, vboDArcPos, vboDArcCol;

//Declare MAtrices
mat4 modelViewMat;    
mat4 modelviewProjMat;
mat4 translateMat;

//Animation Parameter
bool bI1Reached = false, bNReached = false, bDReached = false,bI2Reached = false, bAReached = false, bPlaneReached = false;
static float fTransSpeed = 0.003f;
GLfloat dArcCol[1260];

//Audio Pars
ALCcontext *context;
ALCdevice *device;
char*     alBuffer;             //data for the buffer
ALenum alFormatBuffer;    //buffer format
ALsizei   alFreqBuffer;       //frequency
long       alBufferLen;        //bit depth
ALboolean    alLoop;         //loop
unsigned int alSource;      //source
unsigned int alSampleSet;



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
    void createAudioContext(void);
    void PlaySound();
    void deleteAudioContext();

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
     createAudioContext();
        
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

	XStoreName(gpDisplay, gWindow, "PP_DynamicIndia");

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

    //I
    GLfloat I1Vert[] = {-1.5f, 0.5f, 0.0f, -1.5f, -0.5f, 0.0f};
    GLfloat I1Col[] = {SAFFRON_R, SAFFRON_G, SAFFRON_B, GREEN_R, GREEN_G, GREEN_B};
    glGenVertexArrays(1, &vaoI1);
    glBindVertexArray(vaoI1);
    glGenBuffers(1, &vboI1Pos);
    glBindBuffer(GL_ARRAY_BUFFER, vboI1Pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(I1Vert), I1Vert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &vboI1Col);
    glBindBuffer(GL_ARRAY_BUFFER, vboI1Col);
    glBufferData(GL_ARRAY_BUFFER, sizeof(I1Col), I1Col, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //N
    GLfloat NVert[] = {-1.1f, -0.5f, 0.0f, -1.1f, 0.5f, 0.0f, -0.6f, -0.5f, 0.0f, -0.6f, 0.5f, 0.0f};
    GLfloat NCol [] = {GREEN_R,GREEN_G,GREEN_B,SAFFRON_R,SAFFRON_G,SAFFRON_B,GREEN_R,GREEN_G,GREEN_B,SAFFRON_R,SAFFRON_G,SAFFRON_B};
    glGenVertexArrays(1, &vaoN);
    glBindVertexArray(vaoN);
    glGenBuffers(1, &vboNPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboNPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(NVert), NVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &vboNCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboNCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(NCol), NCol, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //D
    //D ARC
    GLfloat dArcVert[945];
    GLfloat R = GREEN_R, G = GREEN_G, B = GREEN_B; 
    int i , j;
    GLfloat angle = 3 * M_PI/2;
    for(i=0; i < 315; i++) {
        for(j = 0; j < 3; j++) {
            if(j == 0)
                dArcVert[(i*3)+j] = -0.1+cosf(angle)*0.5f;
            else if(j == 1) 
                dArcVert[(i*3)+j] = sinf(angle)*0.5f;
            else 
                dArcVert[(i*3) +j] = 0.0f;
            
            dArcCol[(i*4)+0] = R;
            dArcCol[(i*4)+1] = G;
            dArcCol[(i*4)+2] = B;
            dArcCol[(i*4)+3] = 0.0f;
        }
        G += 0.00021f;
		if(G == SAFFRON_G){
			G = SAFFRON_G;
		}
		R += 0.0029f;
		if( R == SAFFRON_R){
			R = SAFFRON_R;
		}
		B += 0.00054f;
		if(B == SAFFRON_B) {
			B = SAFFRON_B;
		}
        angle += 0.01f;
    }
    glGenVertexArrays(1, &vaoDArc);
    glBindVertexArray(vaoDArc);
    glGenBuffers(1, &vboDArcPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboDArcPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dArcVert), dArcVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &vboDArcCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboDArcCol);
    glBufferData(GL_ARRAY_BUFFER, 1260 * 4, NULL, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //D Line
    GLfloat dLineVert[] = {-0.1f, 0.5f, 0.0f, -0.21f, 0.5f, 0.0f, -0.2f, 0.5f, 0.0f, -0.2f, -0.5f, 0.0f, -0.21f, -0.5f, 0.0f, -0.1f, -0.5f, 0.0f};
    //GLfloat dLineCol[] = {SAFFRON_R, SAFFRON_G, SAFFRON_B, SAFFRON_R, SAFFRON_G, SAFFRON_B, SAFFRON_R, SAFFRON_G, SAFFRON_B, GREEN_R, GREEN_G, GREEN_B, GREEN_R, GREEN_G, GREEN_B, GREEN_R, GREEN_G, GREEN_B};
    glGenVertexArrays(1, &vaoD);
    glBindVertexArray(vaoD);
    glGenBuffers(1, &vboDPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboDPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dLineVert), dLineVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &vboDCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboDCol);
    glBufferData(GL_ARRAY_BUFFER, 3*6*4, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //I2
    GLfloat I2Vert[] = {0.7f,0.5f,0.0f,0.7f,-0.5f,0.0f};
    glGenVertexArrays(1, &vaoI2);
    glBindVertexArray(vaoI2);
    glGenBuffers(1, &vboI2Pos);
    glBindBuffer(GL_ARRAY_BUFFER, vboI2Pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(I2Vert), I2Vert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &vboI2Col);
    glBindBuffer(GL_ARRAY_BUFFER, vboI2Col);
    glBufferData(GL_ARRAY_BUFFER, sizeof(I1Col), I1Col, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //A
    GLfloat aVert[] = {1.0f, -0.5f, 0.0f, 1.3f, 0.5f, 0.0f, 1.6f, -0.5f, 0.0f, 1.15f,0.025f,0.0f, 1.45f,0.025f,0.0f, 1.15f,0.0f,0.0f,1.45f,0.0f,0.0f, 1.15f,-0.02f,0.0f,1.45f,-0.02f,0.0f};
    GLfloat aCol[] = {GREEN_R,GREEN_G,GREEN_B, SAFFRON_R,SAFFRON_G,SAFFRON_B, GREEN_R,GREEN_G,GREEN_B, SAFFRON_R,SAFFRON_G,SAFFRON_B, SAFFRON_R,SAFFRON_G,SAFFRON_B, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, GREEN_R,GREEN_G,GREEN_B,GREEN_R,GREEN_G,GREEN_B};
    glGenVertexArrays(1, &vaoA);
    glBindVertexArray(vaoA);
    glGenBuffers(1, &vboAPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboAPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(aVert), aVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &vboACol);
    glBindBuffer(GL_ARRAY_BUFFER, vboACol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(aCol), aCol, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

    //Functions!!
    void drawI1(void);
    void drawN(void);
    void drawD(void);
    void drawI2(void);
    void drawA(void);

    static bool bPlaySong = false;
    void PlaySound(void);
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(gShaderProgObj);
    
    glLineWidth(10.0f);
    glPointSize(9.0f);

	if(!bPlaySong) {
        PlaySound();
    }

    drawI1();
	
	if(bAReached == true) {
		drawN();
	}
	if(bI2Reached == true) {
		drawD();
	}
	if(bNReached == true) {
		drawI2();
	}
    if(bI1Reached == true) {
		drawA();
	}

    glUseProgram(0);

	glXSwapBuffers(gpDisplay, gWindow);

}
void drawI1(void) {
    static float bXTrans = -0.8f;

    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();

    modelViewMat  = translate(bXTrans, 0.0f, -3.0f);
    modelviewProjMat =  perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);
    
    //Draw I
    glBindVertexArray(vaoI1);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);

    bXTrans += fTransSpeed;
	if(bXTrans >= 0.0f){
		bXTrans = 0.0f;
		bI1Reached = true;
	}


}

void drawN(void) {

    static GLfloat bYTrans = 2.0f;

    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();

    modelViewMat  = translate(0.0f, bYTrans, -3.0f);
    modelviewProjMat =  perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);
    
    //Draw N
    glBindVertexArray(vaoN);
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 1, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glBindVertexArray(0);

    bYTrans -= fTransSpeed;
	if(bYTrans <= 0.0f) {
		bYTrans = 0.0f;
		bNReached = true;
	}
	
}

void drawD(void) {
    //D
	static GLfloat A = 0.0f;
	static GLfloat Sr=0.0f, Sg = 0.0f, Sb = 0.0f;
	static GLfloat Gr=0.0f, Gg = 0.0f, Gb = 0.0f;
	static int iCntStat = 0;
    GLfloat colorArray[] = {Sr, Sg, Sb, Sr, Sg, Sb, Sr, Sg, Sb, Gr, Gg, Gb, Gr, Gg, Gb, Gr, Gg, Gb};

    glBindVertexArray(vaoD);
    glBindBuffer(GL_ARRAY_BUFFER, vboDCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colorArray), colorArray, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glDrawArrays(GL_LINES, 4, 2);
    glBindVertexArray(0);
    for(int i = 0; i<315; i++) {
        dArcCol[((i*4)+3)] = A;
        
    }
    glBindVertexArray(vaoDArc);
    glBindBuffer(GL_ARRAY_BUFFER, vboDArcCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dArcCol), dArcCol, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_POINTS, 0, sizeof(dArcCol)/sizeof(GLfloat));
    glBindVertexArray(0);

    if(iCntStat >= 5) {
	//fprintf(fptr,"Here Val SR =%d\n",iCntStat);

		Sr += 0.001f;
		Sg += 0.0006f;
		Sb += 0.0002f; 
		if(Sr >= SAFFRON_R) {
			//fprintf(fptr,"Here Val itr =%d\n",itr);
			bDReached = true;
			Sr = SAFFRON_R;
		}
		if(Sg >= SAFFRON_G) {
			Sg = SAFFRON_G;
		}
		if(Sb >= SAFFRON_B) {
			Sb = SAFFRON_B;
		}

		Gr += 0.00007f;
		Gg += 0.0006f;
		Gb += 0.000027f;

		if(Gr >= GREEN_R){
			Gr = GREEN_R;
		}
		if(Gg >= GREEN_G){
			Gg = GREEN_G;
		}
		if(Gb >= GREEN_B){
			Gb = GREEN_B;
		}
		
		A += 0.00057f;
		if(A >= 1.0f){
			A = 1.0f;
		}
		iCntStat =0;
	}
	else {
		iCntStat++;
	}
}

void drawI2(void) {
    static GLfloat bYTrans = -2.0f;
        //Draw I
    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();

    modelViewMat  = translate(0.0f, bYTrans, -3.0f);
    modelviewProjMat =  perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);
    
    glBindVertexArray(vaoI2);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);

    bYTrans += fTransSpeed;
	if(bYTrans >= 0.0f) {
		bYTrans = 0.0f;
		bI2Reached = true;
	}
}

void drawA(void) {
    static GLfloat bXTrans = 0.8f;

    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();

    modelViewMat  = translate(bXTrans, 0.0f, -3.0f);
    modelviewProjMat =  perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);

    //Draw  A
    glBindVertexArray(vaoA);
    //Middle Strip
    glDrawArrays(GL_LINES, 3, 2);
    glDrawArrays(GL_LINES, 5, 2);
    glDrawArrays(GL_LINES, 7, 2);
    //Lines of A
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 1, 2);
    glBindVertexArray(0);

    bXTrans -= fTransSpeed;
	if(bXTrans <= 0.0f){
	    bXTrans = 0.0f;
		bAReached = true;
	}

}


void createAudioContext(void) {
    void uninitialize(void);

    device = alcOpenDevice(NULL);
    if (device == NULL)
    {
        printf(" ERROR : Cannot Open Audio Context../n Exiting NOw!\n");
        uninitialize();
        exit(0);
    }
 
    //Create a context
    context=alcCreateContext(device,NULL);
 
    //Set active context
    alcMakeContextCurrent(context);
    
        //load the wave file
    alutLoadWAVFile((ALbyte*)"VandeMataram.wav",&alFormatBuffer, (void **) &alBuffer,(ALsizei*)&alBufferLen, &alFreqBuffer, &alLoop);
    
    //create a source
    alGenSources(1, &alSource);
    
    //create  buffer
    alGenBuffers(1, &alSampleSet);
    
    //put the data into our sampleset buffer
    alBufferData(alSampleSet, alFormatBuffer, alBuffer, alBufferLen, alFreqBuffer);
    
    //assign the buffer to this source
    alSourcei(alSource, AL_BUFFER, alSampleSet);
    
    //release the data
    alutUnloadWAV(alFormatBuffer, alBuffer, alBufferLen, alFreqBuffer);
}

void PlaySound() {

    alSourcei(alSource,AL_LOOPING,AL_FALSE);    
    //play
    alSourcePlay(alSource);
}

void StopSound() {
    alSourceStop(alSource);
}

void deleteAudioContext() {

    alDeleteSources(1,&alSource);
    
    //delete our buffer
    alDeleteBuffers(1,&alSampleSet);
    
    context=alcGetCurrentContext();
    
    //Get device for active context
    device=alcGetContextsDevice(context);
    
    //Disable context
    alcMakeContextCurrent(NULL);
    
    //Release context(s)
    alcDestroyContext(context);
    
    //Close device
    alcCloseDevice(device);
}

void uninitialize(void) {

    deleteAudioContext();

    //UnSetting All We Did For Shaders
    //let make use of shader obj!.
    //Delete VBO
    if(vboAPos) {
        glDeleteBuffers(1, &vboAPos);
        vboAPos = 0;
    }
    if(vboACol) {
        glDeleteBuffers(1, &vboACol);
        vboACol = 0;
    }
    if(vaoA) {
        glDeleteVertexArrays(1, &vaoA);
        vaoA = 0;
    }

    if(vboNCol) {
        glDeleteBuffers(1, &vboNCol);
        vboNCol = 0;
    }
    if(vboNCol) {
        glDeleteBuffers(1, &vboNCol);
        vboNCol = 0;
    }
    if(vaoN) {
        glDeleteVertexArrays(1, &vaoN);
        vaoN = 0;
    }

    if(vboDCol) {
        glDeleteBuffers(1, &vboDCol);
        vboDCol = 0;
    }
    if(vboDCol) {
        glDeleteBuffers(1, &vboDCol);
        vboDCol = 0;
    }
    if(vaoD) {
        glDeleteVertexArrays(1, &vaoD);
        vaoD = 0;
    }

    if(vboDArcCol) {
        glDeleteBuffers(1, &vboDArcCol);
        vboDArcCol = 0;
    }
    if(vboDArcCol) {
        glDeleteBuffers(1, &vboDArcCol);
        vboDArcCol = 0;
    }
    if(vaoDArc) {
        glDeleteVertexArrays(1, &vaoDArc);
        vaoDArc = 0;
    }

    if(vboI1Col) {
        glDeleteBuffers(1, &vboI1Col);
        vboI1Col = 0;
    }
    if(vboI1Col) {
        glDeleteBuffers(1, &vboI1Col);
        vboI1Col = 0;
    }
    if(vaoI1) {
        glDeleteVertexArrays(1, &vaoI1);
        vaoI1 = 0;
    }

    if(vboI2Col) {
        glDeleteBuffers(1, &vboI2Col);
        vboI2Col = 0;
    }
    if(vboI2Col) {
        glDeleteBuffers(1, &vboI2Col);
        vboI2Col = 0;
    }
    if(vaoI2) {
        glDeleteVertexArrays(1, &vaoI2);
        vaoI2 = 0;
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

