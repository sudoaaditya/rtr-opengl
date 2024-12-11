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
#include<GL/gl.h>
#include<GL/glu.h>
#include<GL/glx.h>

#define CheckWidth 64
#define CheckHeight 64

using namespace std;

// Global Variables
bool bFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXVisual = NULL;
Colormap gColormap;
Window gWindow;
int gWinWidth = 800;
int gWinHeight = 600;

static GLXContext gGlxContext;

//Texture Parameters
GLuint iCheckerBoard = 0;
GLubyte checkImgData[CheckWidth][CheckHeight][4];


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
					 ToggleFullScreen();
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
	
					 	default:
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

	//OGL Frame Buffer attributes
	static int frameBufferAttributes[] = { GLX_RGBA,
					       GLX_DOUBLEBUFFER, True,
					       GLX_RED_SIZE, 8,
				      	       GLX_GREEN_SIZE, 8,
					       GLX_BLUE_SIZE, 8,
					       GLX_ALPHA_SIZE, 8, 
					       GLX_DEPTH_SIZE, 24,
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

	gpXVisual = glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttributes);
	if(gpXVisual == NULL) {
		printf(" ERROR : Unable To Get XVisualInfo.\n Exiting Now!!..");
		uninitialize();
		exit(0);
	}
	else {
		printf(" SUCCESS : Alotted With XVisualInfo!.\n");
	}
	
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
				100, 100,
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

	XStoreName(gpDisplay, gWindow, "OGL_CheckerBoard");

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
	void loadTexture(void);
			
	gGlxContext = glXCreateContext(gpDisplay, 
				       gpXVisual,
				       NULL,
				       GL_TRUE);

	if(!gGlxContext) {
		printf(" ERROR : Failed To Get GLXContext.\n Exiting Now!!..");
		uninitialize();
		exit(0);
	}
	else {
		printf(" SUCCESS : Alotted With GLXContext!..\n");
	}

	if(glXMakeCurrent(gpDisplay, gWindow, gGlxContext) != True) {
		return(-1);
	}else {
		printf(" SUCCESS : Set as Current GLXContext!..\n");
	}


	glShadeModel(GL_SMOOTH);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

	glEnable(GL_TEXTURE_2D);

	loadTexture();

	resize(gWinWidth, gWinHeight);

	return(0);

}

void loadTexture(void) {

	void makeCheckImage(void);

	makeCheckImage();

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &iCheckerBoard);
	glBindTexture(GL_TEXTURE_2D, iCheckerBoard);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D,
		     0,
		     GL_RGBA,
		     CheckWidth,
		     CheckHeight,
		     0,
		     GL_RGBA,
		     GL_UNSIGNED_BYTE,
		     checkImgData);

	glTexEnvf(GL_TEXTURE_ENV,
		  GL_TEXTURE_ENV_MODE,
		  GL_REPLACE);
}

void makeCheckImage(void) {
	int i, j, c;

	for(i = 0; i < CheckHeight; i++) {
		for( j = 0; j < CheckHeight; j++) {

			c = ( ((i & 0x8 ) == 0) ^ (( j & 0x8) == 0) ) * 255;

			checkImgData[i][j][0] = (GLubyte)c;
			checkImgData[i][j][1] = (GLubyte)c;
			checkImgData[i][j][2] = (GLubyte)c;
			checkImgData[i][j][3] = (GLubyte)255;
		}
	}
}

void resize(int width, int height) {

	if(height == 0) {
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	gluPerspective(45.0f, 
		       ((GLfloat)width/(GLfloat)height),
		       0.1f,
		       100.0f);
}

void display(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -3.6f);
	
	glBindTexture(GL_TEXTURE_2D, iCheckerBoard);

	glBegin(GL_QUADS);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-2.0f, -1.0f, 0.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-2.0f, 1.0f, 0.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(0.0f, -1.0f, 0.0f);

	//Second Tilted!.
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(2.41421f, 1.0f, -1.41421f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(2.41421f, -1.0f, -1.41421f);

	glEnd();

	glXSwapBuffers(gpDisplay, gWindow);

}



void uninitialize(void) {

	if(iCheckerBoard) {
		glDeleteTextures(1, &iCheckerBoard);
		iCheckerBoard = 0;
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
	
	printf("\n SUCCESS : Code Termination Successful!!..\n\n");
}

