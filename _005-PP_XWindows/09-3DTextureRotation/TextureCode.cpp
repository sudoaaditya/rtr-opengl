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
#include<SOIL/SOIL.h>

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
GLuint mvpUniform;
GLuint samplerUniform;
mat4 perspectiveProjMat;

GLfloat fAnglePyramid = 0.0f;
GLfloat fAngleCube = 0.0f;

GLuint vao_pyramid, vbo_pyramid_position, vbo_pyramid_texture;
GLuint vao_cube, vbo_cube_position, vbo_cube_texture;

//Texture Parameters
GLuint iPyramidTex = 0;
GLuint iCubeTex = 0;
const char *imgPathPyramid = "Stone.bmp";
const char *imgPathCube  = "Vijay_Kundali.bmp";


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

     ToggleFullScreen();

	 // Now Initialize Window
	 if(initialize() == 0) {
		 printf(" SUCCESS : Initialization Successful!.\n");
	 }
	 else {
		 printf(" ERROR : Initialization Failed!!..\n");
		 uninitialize();
		 exit(0);
	 }

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
        update();
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

	XStoreName(gpDisplay, gWindow, "3D Rotations");

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


    void uninitialize(void);
	void resize(int, int);
    bool loadTexture(GLuint*, const char*);

	GLenum result;

    //shader log lines..
    GLint iShaderCompileStat = 0;
    GLint iShaderInfoLogLen = 0;
    GLchar* shaderInfoLog = NULL; 

	
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
            "in vec2 vTexCoord;" \
            "uniform mat4 u_mvp_matrix;" \
            "out vec2 out_texCoord;" \
            "void main(void)" \
            "{" \
            "gl_Position = u_mvp_matrix * vPosition;" \
            "out_texCoord = vTexCoord;" \
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
            "in vec2 out_texCoord;" \
            "uniform sampler2D u_sampler;"  \
            "out vec4 FragColor;" \
            "void main(void)" \
            "{" \
            "FragColor = texture(u_sampler, out_texCoord);" \
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
    glBindAttribLocation(gShaderProgObj, AMC_ATTRIBUTE_TEXTURE0, "vTexCoord");

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
    samplerUniform = glGetUniformLocation(gShaderProgObj, "u_sampler");

	    //Create Array of vertices to be sent over GPU
    const GLfloat pyramidVert[] = { 0.0f, 1.0f, 0.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
                                    0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
                                    0.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
                                    0.0f, 1.0f, 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f};

    const GLfloat pyramidTexCoords[] = { 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
                                         0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                                         0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
                                         0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };


    glGenVertexArrays(1, &vao_pyramid);
    glBindVertexArray(vao_pyramid);

    glGenBuffers(1, &vbo_pyramid_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_position);

    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVert), pyramidVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_pyramid_texture);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_pyramid_texture);

    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidTexCoords), pyramidTexCoords, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXTURE0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);       
    glBindVertexArray(0);

    //Cube array & COlor
    const GLfloat cubeVert[] = { 
        -1.0f, 1.0f, 1.0f,-1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f,//Front
        1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,//Right
		1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f,//Back
		-1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f,//Left
		-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f,//Top
		-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f//Bottom
    };

    const GLfloat cubeTexCoord[] = { 
            1.0f,1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f,1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f,1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f,1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f,1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f,1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f
        };


    glGenVertexArrays(1, &vao_cube);
    glBindVertexArray(vao_cube);

    glGenBuffers(1, &vbo_cube_position);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_position);

    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVert), cubeVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vbo_cube_texture);

    glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_texture);

    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeTexCoord), cubeTexCoord, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXTURE0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
        
    glEnable(GL_CULL_FACE);
    glDisable(GL_FRONT_AND_BACK);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    if(loadTexture(&iPyramidTex, imgPathPyramid) == false) {
        printf(" ERROR : Failed To load Texture!.\n Exiting Now!!..");
		uninitialize();
		exit(0);
	}
	else {
		printf(" SUCCESS : Stone Texture Loaded Successfully!..\n");
	}

    if(loadTexture(&iCubeTex, imgPathCube) == false) {
        printf(" ERROR : Failed To load Texture!.\n Exiting Now!!..");
		uninitialize();
		exit(0);
	}
	else {
		printf(" SUCCESS : Cube Texture Loaded Successfully!..\n");
	}

    glEnable(GL_TEXTURE_2D);

    perspectiveProjMat = mat4::identity();

	resize(gWinWidth, gWinHeight);

	return(0);

}

bool loadTexture(GLuint *texture, const char *path) {

	bool bStatus = false;
	unsigned char *imgData = NULL;
	int imgWidth, imgHeight;

	imgData = SOIL_load_image(path, &imgWidth, &imgHeight, 0, SOIL_LOAD_RGB);
	
	if(imgData == NULL) {
		bStatus = false;
		return(bStatus);
	}
	else {
		bStatus = true;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		glGenTextures(1, texture);	
		glBindTexture(GL_TEXTURE_2D, *texture);
	
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imgData);

        glGenerateMipmap(GL_TEXTURE_2D);

		SOIL_free_image_data(imgData);
		imgData = NULL;
	}
    return(bStatus);
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
    mat4 translateMat;
    mat4 rotationMat;
    mat4 scaleMat;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gShaderProgObj);

    //Pyramid!!.
    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();
    translateMat = mat4::identity();
    rotationMat = mat4::identity();

    translateMat = translate(-2.0f, 0.0f, -6.0f);
    rotationMat = rotate(fAnglePyramid, 0.0f, 1.0f, 0.0f);
    
    modelViewMat = modelViewMat * translateMat;
    modelViewMat = modelViewMat * rotationMat;

    modelviewProjMat = perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, iPyramidTex);
    glUniform1i(samplerUniform, 0);

    glBindVertexArray(vao_pyramid);
    
    glDrawArrays(GL_TRIANGLES, 0, 12);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0); 

    //Cube!!..
    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();
    translateMat = mat4::identity();
    rotationMat = mat4::identity();
    scaleMat = mat4::identity();

    translateMat = translate(2.0f, 0.0f, -6.0f);
    scaleMat = scale(0.75f, 0.75f, 0.75f);
    rotationMat = rotate(fAngleCube, 1.0f, 0.0f, 0.0f);
    rotationMat *= rotate(fAngleCube, 0.0f, 1.0f, 0.0f);
    rotationMat *= rotate(fAngleCube, 0.0f, 0.0f, 1.0f);

    modelViewMat = modelViewMat * translateMat;
    modelViewMat = modelViewMat * scaleMat;
    modelViewMat = modelViewMat * rotationMat;

    modelviewProjMat = perspectiveProjMat * modelViewMat;

    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, iCubeTex);
    glUniform1i(samplerUniform, 0);

    glBindVertexArray(vao_cube);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 4, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 8, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 12, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 16, 4);
    glDrawArrays(GL_TRIANGLE_FAN, 20, 4);

    glBindVertexArray(0);

	glUseProgram(0);

	glXSwapBuffers(gpDisplay, gWindow);

}

void update(void) {

    fAnglePyramid += 0.5f;
    if(fAnglePyramid >= 360.0f) {
        fAnglePyramid = 0.0f;
    }

    fAngleCube += 0.5f;
    if(fAngleCube >= 360.0f) {
        fAngleCube = 0.0f;
    }
}

void uninitialize(void) {

	//vao & vbos
    if(vbo_pyramid_position) {
        glDeleteBuffers(1, &vbo_pyramid_position);
        vbo_pyramid_position = 0;
    }
    if(vbo_pyramid_texture) {
        glDeleteBuffers(1, &vbo_pyramid_texture);
        vbo_pyramid_texture = 0;
    }
    if(vao_pyramid) {
        glDeleteVertexArrays(1, &vao_pyramid);
        vao_pyramid = 0;
    }

    if(vbo_cube_position) {
        glDeleteBuffers(1, &vbo_cube_position);
        vbo_cube_position = 0;
    }
    if(vbo_cube_texture) {
        glDeleteBuffers(1, &vbo_cube_texture);
        vbo_cube_texture = 0;
    }
    if(vao_cube) {
        glDeleteVertexArrays(1, &vao_cube);
        vao_cube = 0;
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
