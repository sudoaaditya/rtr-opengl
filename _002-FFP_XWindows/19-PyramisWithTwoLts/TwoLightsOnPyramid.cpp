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

using namespace std;

// Global Variables
bool bFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXViual = NULL;
Colormap gColormap;
Window gWindow;
int gWinWidth = 800;
int gWinHeight = 600;

static GLXContext gGlxContext;

GLfloat frotPyrmd = 0.0f;

//Light Parameters
bool bLight = false;

//Left Wing
GLfloat lightAmbientZero[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightDiffuseZero[] = { 1.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightSpecularZero[] = { 1.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightPositionZero[] = { -2.0f, 0.0f, 0.0f, 1.0f };

//Right Wing
GLfloat lightAmbientOne[] = { 0.0f, 0.0f, 0.0f, 1.0f }; 
GLfloat lightDiffuseOne[] = { 0.0f, 0.0f, 1.0f, 1.0f };
GLfloat lightSpecularOne[] = { 0.0f, 0.0f, 1.0f, 1.0f };
GLfloat lightPositionOne[] = { 2.0f, 0.0f, 0.0f, 1.0f };


//Material
GLfloat materialAmbient[] = { 0.0f, 0.0f, 0.0f };
GLfloat materialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat materialShininess[] = { /*50.0f*/ 128.0f };


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
                        case 'l':
                        case 'L':
                            if(!bLight) {
                                glEnable(GL_LIGHTING);
                                bLight = true;
                            }
                            else {
                                glDisable(GL_LIGHTING);
                                bLight = false;
                            }

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

	gpXViual = glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttributes);
	if(gpXViual == NULL) {
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
					      RootWindow(gpDisplay, gpXViual->screen),
					      gpXViual->visual,
					      AllocNone);
	gColormap = winAttribs.colormap;
	winAttribs.event_mask = ExposureMask | VisibilityChangeMask |
				ButtonPressMask | KeyPressMask |
				PointerMotionMask | StructureNotifyMask;
	
	styleMask = CWBorderPixmap | CWBackPixel | CWEventMask | CWColormap;

	
	gWindow = XCreateWindow(gpDisplay,
				RootWindow(gpDisplay, gpXViual->screen),
				0, 0,
				gWinWidth, gWinHeight,
				0,
				gpXViual->depth,
				InputOutput,
				gpXViual->visual,
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

	XStoreName(gpDisplay, gWindow, "Two Lights on Sphere!");

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
		   RootWindow(gpDisplay, gpXViual->screen),
		   False,
		   StructureNotifyMask,
		   &xeve);

}

int initialize(void) {

	void uninitialize(void);
	void resize(int, int);
			
	gGlxContext = glXCreateContext(gpDisplay, 
				       gpXViual,
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


    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbientZero);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuseZero);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecularZero);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPositionZero);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbientOne);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuseOne);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecularOne);
    glLightfv(GL_LIGHT1, GL_POSITION, lightPositionOne);
    glEnable(GL_LIGHT1);
 
    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);
 
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	resize(gWinWidth, gWinHeight);

	return(0);

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
    void drawPyramid(void);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -5.0f);

    glRotatef(frotPyrmd, 0.0f, 1.0f, 0.0f);

    drawPyramid();

	glXSwapBuffers(gpDisplay, gWindow);

}


void drawPyramid(void) {

    glBegin(GL_TRIANGLES);

    //Front face
    glNormal3f(0.0f, 0.447214f, 0.894427f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    //Right Face
    glNormal3f(0.894427f, 0.447214f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);

    //Back face
    glNormal3f(0.0f, 0.447214f, -0.894427f);    
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);

    //Left Face
    glNormal3f(-0.894427f, 0.447214f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.00f, -1.0f, 1.0f);

    glEnd();
}

void update(void) {

    frotPyrmd += 0.8f;
    if(frotPyrmd >= 360.0f) {
        frotPyrmd = 0.0f;
    }
}

void uninitialize(void) {

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

	if(gpXViual) {
		free(gpXViual);
		gpXViual = NULL;
	}

	if(gpDisplay) {
		XCloseDisplay(gpDisplay);
		gpDisplay = NULL;
	}
	
	printf("\n SUCCESS : Code Termination Successfully!!..\n\n");
}

