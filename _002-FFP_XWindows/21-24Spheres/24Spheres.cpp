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

//Lights Vars
bool bLights = false;

GLfloat lightAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat lightPosition[] = { 0.0f, 0.0f, 0.0f, 1.0f }; 

GLfloat light_model_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
GLfloat light_model_local_viewer[] = { 0.0f };

GLUquadric *Spheres[24];

GLfloat angleOfXRot = 0.0f;
GLfloat angleOfYRot = 0.0f;
GLfloat angleOfZRot = 0.0f;

GLint KeyPressed = 0;

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
                        case 'L':
                        case 'l':
                            if(!bLights) {
                                bLights = true;
                                glEnable(GL_LIGHTING);
                            }
                            else {
                                bLights = false;
                                glDisable(GL_LIGHTING);
                            }
                            break;

                        case 'X':
                        case 'x':
                            KeyPressed = 1;
                            angleOfXRot = 0.0f;
                            break;
                        
                        case 'Y':
                        case 'y':
                            KeyPressed = 2;
                            angleOfYRot = 0.0f;
                            break;

                        case 'Z':
                        case 'z':
                            KeyPressed = 3;
                            angleOfZRot = 0.0f;
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

	XStoreName(gpDisplay, gWindow, "24 Spheres");

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

    glEnable(GL_AUTO_NORMAL);   //TO Let OGL calculate and Take Care Of Normals
    glEnable(GL_NORMALIZE);     // To Rescale & rESIZE nOMALS Mathematically.

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0,  GL_POSITION, lightPosition);
    
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, light_model_local_viewer);
    glEnable(GL_LIGHT0);

    for(int i = 0; i < 24; i++) {
        Spheres[i] = gluNewQuadric();
    }

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
	
    if(width < height) {
        glOrtho(0.0f,
            15.5f,
            0.0f,
            15.5f * (GLfloat)height/(GLfloat)width,
            -10.0f,
            10.0f);
    }
    else {
        glOrtho(0.0f,
            15.5f * (GLfloat)width/(GLfloat)height,
            0.0f,
            15.5f,
            -10.0f,
            10.0f);
    }
}

void display(void) {

    void draw24Spheres(void);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if(KeyPressed == 1) {
        glRotatef(angleOfXRot, 1.0f, 0.0f, 0.0f);
        lightPosition[1] = angleOfXRot;
    }
    else if(KeyPressed == 2) {
        glRotatef(angleOfYRot, 0.0f, 1.0f, 0.0f);
        lightPosition[2] = angleOfYRot;
    }
    else if(KeyPressed == 3) {
        glRotatef(angleOfZRot, 0.0f, 0.0f, 1.0f);
        lightPosition[0] = angleOfZRot;
    }
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    draw24Spheres();
	
	glXSwapBuffers(gpDisplay, gWindow);

}
void draw24Spheres(void) {

    //Local to all
    GLfloat matAmb[4];
    GLfloat matDiff[4];
    GLfloat matSpec[4];
    GLfloat matShine;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //for Spere 1 in col 1 : Emerald
    matAmb[0] = 0.0215f;
    matAmb[1] = 0.1745;
    matAmb[2] = 0.0215;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.07568f;
    matDiff[1] = 0.61424f;
    matDiff[2] = 0.07568f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.633f;
    matSpec[1] = 0.727811f;
    matSpec[2] = 0.633f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.6f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.5f, 14.0f, 0.0f);
    gluSphere(Spheres[0], 1.0f, 30, 30);


    //for Spere 2 in col 1 : Jade
    matAmb[0] = 0.135f;
    matAmb[1] = 0.2225f;
    matAmb[2] = 0.1575f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.54f;
    matDiff[1] = 0.89f;
    matDiff[2] = 0.63f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.316228f;
    matSpec[1] = 0.316228f;
    matSpec[2] = 0.316228f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.1f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.5f, 11.5f, 0.0f);
    gluSphere(Spheres[1], 1.0f, 30, 30);
    
    
    //for Spere 3 in col 1 : Obsidian
    matAmb[0] = 0.05375f;
    matAmb[1] = 0.05f;
    matAmb[2] = 0.06652f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.18275f;
    matDiff[1] = 0.17f;
    matDiff[2] = 0.22525f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.332741f;
    matSpec[1] = 0.328634f;
    matSpec[2] = 0.346435f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.3f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.5f, 9.0f, 0.0f);
    gluSphere(Spheres[2], 1.0f, 30, 30);
    
    
    //for Spere 4 in col 1 : Pearl
    matAmb[0] = 0.25f;
    matAmb[1] = 0.20725f;
    matAmb[2] = 0.20725f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 1.0f;
    matDiff[1] = 0.829f;
    matDiff[2] = 0.829f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.296648f;
    matSpec[1] = 0.296648f;
    matSpec[2] = 0.296648f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.088f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.5f, 6.5f, 0.0f);
    gluSphere(Spheres[3], 1.0f, 30, 30);
    
    //for Spere 5 in col 1 : RUBY
    matAmb[0] = 0.1745f;
    matAmb[1] = 0.01175f;
    matAmb[2] = 0.01175f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.61424f;
    matDiff[1] = 0.04136f;
    matDiff[2] = 0.04136f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.727811f;
    matSpec[1] = 0.626959f;
    matSpec[2] = 0.626959f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.6f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.5f, 4.0f, 0.0f);
    gluSphere(Spheres[4], 1.0f, 30, 30);
    
    //for Spere 6 in col 1 : TurQuoise
    matAmb[0] = 0.1f;
    matAmb[1] = 0.18725f;
    matAmb[2] = 0.1754f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.396f;
    matDiff[1] = 0.74151f;
    matDiff[2] = 0.69102f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.297254f;
    matSpec[1] = 0.30829f;
    matSpec[2] = 0.306678f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.1f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(5.5f, 1.5f, 0.0f);
    gluSphere(Spheres[5], 1.0f, 30, 30);
    
    
    //for Spere 1 in col 2 : Brass
    matAmb[0] = 0.329412f;
    matAmb[1] = 0.223529f;
    matAmb[2] = 0.027451f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.780392f;
    matDiff[1] = 0.568627f;
    matDiff[2] = 0.113725f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.992157f;
    matSpec[1] = 0.941176f;
    matSpec[2] = 0.807843f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.21794872f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(11.0f, 14.0f, 0.0f);
    gluSphere(Spheres[6], 1.0f, 30, 30);
    
    //for Spere 2 in col 2 : Bronze
    matAmb[0] = 0.2125f;
    matAmb[1] = 0.1275f;
    matAmb[2] = 0.054f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.714f; 
    matDiff[1] = 0.4284f;
    matDiff[2] = 0.18144f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.393548f;
    matSpec[1] = 0.271906f;
    matSpec[2] = 0.166721f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.2f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(11.0f, 11.5f, 0.0f);
    gluSphere(Spheres[7], 1.0f, 30, 30);
    
    //for Spere 3 in col 2 : Chrome
    matAmb[0] = 0.25f;
    matAmb[1] = 0.25f;
    matAmb[2] = 0.25f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.4f;
    matDiff[1] = 0.4f;
    matDiff[2] = 0.4f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.774597f;
    matSpec[1] = 0.774597f;
    matSpec[2] = 0.774597f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.6f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(11.0f, 9.0f, 0.0f);
    gluSphere(Spheres[8], 1.0f, 30, 30);
    
    //for Spere 4 in col 2 : Copper
    matAmb[0] = 0.19125f;
    matAmb[1] = 0.0735f;
    matAmb[2] = 0.0225f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.7038f;
    matDiff[1] = 0.27048f;
    matDiff[2] = 0.0828f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.256777f;
    matSpec[1] = 0.137622f;
    matSpec[2] = 0.0828f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.1f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(11.0f, 6.5f, 0.0f);
    gluSphere(Spheres[9], 1.0f, 30, 30);
    
    //for Spere 5 in col 2 : Gold
    matAmb[0] = 0.24725f;
    matAmb[1] = 0.1995f;
    matAmb[2] = 0.0745f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.75164f;
    matDiff[1] = 0.60648f;
    matDiff[2] = 0.22648f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.628281f;
    matSpec[1] = 0.555802f;
    matSpec[2] = 0.366065f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.4f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(11.0f, 4.0f, 0.0f);
    gluSphere(Spheres[10], 1.0f, 30, 30);
    
    //for Spere 6 in col 2 : Silver
    matAmb[0] = 0.19225f;
    matAmb[1] = 0.19225f;
    matAmb[2] = 0.19225f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.50754f;
    matDiff[1] = 0.50754f;
    matDiff[2] = 0.50754f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.508273f;
    matSpec[1] = 0.508273f;
    matSpec[2] = 0.508273f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.4f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(11.0f, 1.5f, 0.0f);
    gluSphere(Spheres[11], 1.0f, 30, 30);
    
    
    //for Spere 1 in col 3 : Black Plastic
    matAmb[0] = 0.0f;
    matAmb[1] = 0.0f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.01f;
    matDiff[1] = 0.01f;
    matDiff[2] = 0.01f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.50f;
    matSpec[1] = 0.50f;
    matSpec[2] = 0.50f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(16.5f, 14.0f, 0.0f);
    gluSphere(Spheres[12], 1.0f, 30, 30);
    
    //for Spere 2 in col 3 : Cyan Plastic
    matAmb[0] = 0.0f;
    matAmb[1] = 0.1f;
    matAmb[2] = 0.06f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.0f;
    matDiff[1] = 0.50980392f;
    matDiff[2] = 0.50980392f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.50196078;
    matSpec[1] = 0.50196078;
    matSpec[2] = 0.50196078;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(16.5f, 11.5f, 0.0f);
    gluSphere(Spheres[13], 1.0f, 30, 30);
    
    //for Spere 3 in col 3 : Green Plastic
    matAmb[0] = 0.0f;
    matAmb[1] = 0.0f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.1f;
    matDiff[1] = 0.35f;
    matDiff[2] = 0.1f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.45f;
    matSpec[1] = 0.55f;
    matSpec[2] = 0.45f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(16.5f, 9.0f, 0.0f);
    gluSphere(Spheres[14], 1.0f, 30, 30);
    
    //for Spere 4 in col 3 : Red Plastic
    matAmb[0] = 0.0f;
    matAmb[1] = 0.0f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.5f;
    matDiff[1] = 0.0f;
    matDiff[2] = 0.0f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);;

    matSpec[0] = 0.7f;
    matSpec[1] = 0.6f;
    matSpec[2] = 0.6f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(16.5f, 6.5f, 0.0f);
    gluSphere(Spheres[15], 1.0f, 30, 30);
    
    //for Spere 5 in col 3 : White Plastic
    matAmb[0] = 0.0f;
    matAmb[1] = 0.0f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.55f;
    matDiff[1] = 0.55f;
    matDiff[2] = 0.55f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.70f;
    matSpec[1] = 0.70f;
    matSpec[2] = 0.70f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(16.5f, 4.0f, 0.0f);
    gluSphere(Spheres[16], 1.0f, 30, 30);
    
    //for Spere 6 in col 3 : Yellow Plastic
    matAmb[0] = 0.0f;
    matAmb[1] = 0.0f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.5f;
    matDiff[1] = 0.5f;
    matDiff[2] = 0.0f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.60f;
    matSpec[1] = 0.60f;
    matSpec[2] = 0.50f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.25f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(16.5f, 1.5f, 0.0f);
    gluSphere(Spheres[17], 1.0f, 30, 30);
    
    
    //for Spere 1 in col 4 : Black Rubber
    matAmb[0] = 0.02f;
    matAmb[1] = 0.02f;
    matAmb[2] = 0.02f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.01f;
    matDiff[1] = 0.01f;
    matDiff[2] = 0.01f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.4f;
    matSpec[1] = 0.4f;
    matSpec[2] = 0.4f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(22.0f, 14.0f, 0.0f);
    gluSphere(Spheres[18], 1.0f, 30, 30);
    
    //for Spere 2 in col 4 : CYAN Rubber
    matAmb[0] = 0.0f;
    matAmb[1] = 0.05f;
    matAmb[2] = 0.05f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.4f;
    matDiff[1] = 0.5f;
    matDiff[2] = 0.5f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.04f;
    matSpec[1] = 0.7f;
    matSpec[2] = 0.7f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(22.0f, 11.5f, 0.0f);
    gluSphere(Spheres[19], 1.0f, 30, 30);
    
    //for Spere 3 in col 4 : Green Rubber
    matAmb[0] = 0.0f;
    matAmb[1] = 0.05f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.4f;
    matDiff[1] = 0.5f;
    matDiff[2] = 0.4f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.04f;
    matSpec[1] = 0.7f;
    matSpec[2] = 0.04f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(22.0f, 9.0f, 0.0f);
    gluSphere(Spheres[20], 1.0f, 30, 30);
    
    //for Spere 4 in col 5 : Red Rubber
    matAmb[0] = 0.05f;
    matAmb[1] = 0.0f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.5f;
    matDiff[1] = 0.4f;
    matDiff[2] = 0.4f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.7f;
    matSpec[1] = 0.04f;
    matSpec[2] = 0.04f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(22.0f, 6.5f, 0.0f);
    gluSphere(Spheres[21], 1.0f, 30, 30);
    
    //for Spere 5 in col 4 : White Rubber
    matAmb[0] = 0.05f;
    matAmb[1] = 0.05f;
    matAmb[2] = 0.05f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.5f;
    matDiff[1] = 0.5f;
    matDiff[2] = 0.5f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.7f;
    matSpec[1] = 0.7f;
    matSpec[2] = 0.7f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(22.0f, 4.0f, 0.0f);
    gluSphere(Spheres[22], 1.0f, 30, 30);
    
    //for Spere 6 in col 4 : Yellow Rubber
    matAmb[0] = 0.05f;
    matAmb[1] = 0.05f;
    matAmb[2] = 0.0f;
    matAmb[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmb);

    matDiff[0] = 0.5f;
    matDiff[1] = 0.5f;
    matDiff[2] = 0.4f;
    matDiff[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiff);

    matSpec[0] = 0.7f;
    matSpec[1] = 0.7f;
    matSpec[2] = 0.04f;
    matSpec[3] = 1.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpec);

    matShine = 0.078125f * 128;
    glMaterialf(GL_FRONT, GL_SHININESS, matShine);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(22.0f, 1.5f, 0.0f);
    gluSphere(Spheres[23], 1.0f, 30, 30);
    

}

void update(void) {
    
    angleOfXRot += 0.9f;
    if(angleOfXRot >= 360.0f) {
        angleOfXRot = 0.0f;
    }

    angleOfYRot += 0.9f;
    if(angleOfYRot >= 360.0f) {
        angleOfYRot = 0.0f;
    }

    angleOfZRot += 0.9f;
    if(angleOfZRot >= 360.0f) {
        angleOfZRot = 0.0f;
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

