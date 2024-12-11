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
#include"Sphere.h"

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
GLuint vaoSphere, vboSpherePos, vboSphereNorm, vboSphereEle;
GLuint mvUniform;
GLuint projMatUniform;
GLuint ldUniform;
GLuint kdUniform;
GLuint lightPosUniform;
GLuint lPressedUniform;
mat4 perspectiveProjMat;

//Sphere Vert!.
float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textutres[764];
unsigned short sphere_elements[2280];
int gNumVert, gNumEle;

// Animation Variables!.
bool bLighting = false;
bool bAnimation = false;
GLfloat fRotAngle = 0.0f;
GLfloat lightPos[] = {2.0f, 2.0f, 2.0f, 1.0f};


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
     void update(void);

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

                        case 'L':
                        case 'l':
                            if(!bLighting){
                                bLighting = true;
                            }
                            else {
                                bLighting = false;
                            }
                        break;

                        case 'A':
                        case 'a':
                            if(!bAnimation) {
                                bAnimation = true;
                            }
                            else {
                                bAnimation =  false;
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
        if(bAnimation){
            update();
        }

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

	XStoreName(gpDisplay, gWindow, "Diffuse Light On Sphere!");

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
	
    //LoadSphereData
    getSphereVertexData(sphere_vertices, sphere_normals, sphere_textutres, sphere_elements);
    gNumVert = getNumberOfSphereVertices();
    gNumEle = getNumberOfSphereElements();


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
                    "in vec3 vNormal;" \
                    "uniform mat4 u_mvMatrix;" \
                    "uniform mat4 u_projMatrix;" \
                    "uniform vec3 u_ld;" \
                    "uniform vec3 u_kd;" \
                    "uniform int u_lKeyPressed;" \
                    "uniform vec4 u_lightPosition;" \
                    "out vec3 diffuseColor;" \
                    "void main(void) {" \
                    "if(u_lKeyPressed == 1 ) {" \
                    "   vec4 eyeCoords = u_mvMatrix * vPosition;" \
                    "   mat3 normMatrix = mat3(transpose(inverse(u_mvMatrix)));" \
                    "   vec3 tNorm = normalize(normMatrix * vNormal);" \
                    "   vec3 s = vec3(u_lightPosition - eyeCoords);" \
                    "   diffuseColor = u_ld * u_kd * max(dot(s, tNorm), 0.0);" \
                    "}" \
                    "gl_Position = u_projMatrix * u_mvMatrix * vPosition;" \
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
                    "out vec4 FragColor;" \
                    "uniform int u_lKeyPressed;" \
                    "in vec3 diffuseColor;" \
                    "void main(void) {" \
                    "if(u_lKeyPressed == 1) {" \
                    "   FragColor = vec4(diffuseColor, 1.0);" \
                    "}" \
                    "else {" \
                    "   FragColor = vec4(1.0, 1.0, 1.0, 1.0);" \
                    "}"\
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
    glBindAttribLocation(gShaderProgObj, AMC_ATTRIBUTE_NORMAL, "vNormal");

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
    mvUniform = glGetUniformLocation(gShaderProgObj, "u_mvMatrix");
    projMatUniform = glGetUniformLocation(gShaderProgObj, "u_projMatrix");
    ldUniform = glGetUniformLocation(gShaderProgObj, "u_ld");
    kdUniform = glGetUniformLocation(gShaderProgObj, "u_kd");
    lPressedUniform = glGetUniformLocation(gShaderProgObj, "u_lKeyPressed");
    lightPosUniform = glGetUniformLocation(gShaderProgObj, "u_lightPosition");


    glGenVertexArrays(1, &vaoSphere);
    glBindVertexArray(vaoSphere);

    glGenBuffers(1, &vboSpherePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboSpherePos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);        // unbind VBO

    glGenBuffers(1, &vboSphereNorm);
    glBindBuffer(GL_ARRAY_BUFFER, vboSphereNorm);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vboSphereEle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereEle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(0);   //unbind VAO

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
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

       //Initialization of matrices
    mat4 modelViewMat = mat4::identity();
    mat4 modelviewProjMat = mat4::identity();
    mat4 translateMat = mat4::identity();
    mat4 rotateMat = mat4::identity();


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gShaderProgObj);
    translateMat = translate(0.0f, 0.0f, -3.0f);
    modelViewMat *= translateMat;
    rotateMat = rotate(fRotAngle, 1.0f, 0.0f, 0.0f);
    rotateMat *= rotate(fRotAngle, 0.0f, 1.0f, 0.0f);
    rotateMat *= rotate(fRotAngle, 0.0f, 0.0f, 1.0f);
    modelViewMat *= rotateMat;
    glUniformMatrix4fv(mvUniform, 1, GL_FALSE, modelViewMat);
    glUniformMatrix4fv(projMatUniform, 1, GL_FALSE, perspectiveProjMat);

    if(bLighting) {
        glUniform1i(lPressedUniform, 1);
        glUniform3f(ldUniform, 1.0f, 1.0f, 1.0f);
        glUniform3f(kdUniform, 0.5f, 0.5f, 0.5f);
        glUniform4fv(lightPosUniform, 1, lightPos);
    }
    else {
        glUniform1i(lPressedUniform, 0);
    }

    glBindVertexArray(vaoSphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereEle);
    glDrawElements(GL_TRIANGLES, gNumEle, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
    glUseProgram(0);

	glXSwapBuffers(gpDisplay, gWindow);

}
void update(void) {

    fRotAngle += 0.2f;
    if(fRotAngle >= 360.0f) {
        fRotAngle = 0.0f;
    }
}


void uninitialize(void) {

    if(vboSpherePos) {
        glDeleteBuffers(1, &vboSpherePos);
        vboSpherePos = 0;
    }
    if(vboSphereNorm) {
        glDeleteBuffers(1, &vboSphereNorm);
        vboSphereNorm = 0;
    }
    if(vboSphereEle) {
        glDeleteBuffers(1, &vboSphereEle);
        vboSphereEle = 0;
    }
    //DeleteVAO
    if(vaoSphere) {
        glDeleteVertexArrays(1, &vaoSphere);
        vaoSphere = 0;
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

