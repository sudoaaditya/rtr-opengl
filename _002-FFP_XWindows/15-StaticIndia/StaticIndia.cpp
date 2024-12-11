//System Files
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

//XWindows Header File
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>

// OGL Header File
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#define _USE_MATH_DEFINES 1
#include<math.h>

using namespace std;


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



// Global Variables
bool bFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXViual = NULL;
Colormap gColormap;
Window gWindow;
int gWinWidth = 800;
int gWinHeight = 600;


static GLXContext gGlxContext;

//MAIN
int main(void)
{

	printf(" SUCCESS : Code Initiated!!..\n\n");

	//func
	void initialize(void);
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
	initialize();

	// Message Loop
	XEvent event;
	KeySym keysym;
	char keys[26];

	while (!bDone)
	{

		while (XPending(gpDisplay))
		{

			XNextEvent(gpDisplay, &event);

			switch (event.type)
			{
			case MapNotify:
				break;

			case KeyPress:
				keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
				switch (keysym)
				{
				case XK_Escape:
					bDone = true;
					break;

				default:
					break;
				}
				XLookupString(&event.xkey, keys, sizeof(keys), NULL, NULL);
				switch (keys[0])
				{
				case 'F':
				case 'f':
					if (!bFullScreen)
					{
						ToggleFullScreen();
						bFullScreen = true;
					}
					else
					{
						ToggleFullScreen();
						bFullScreen = false;
					}
					break;
				}
				break;

			case ButtonPress:
				switch (event.xbutton.button)
				{

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
	return (0);
}

void CreateWindow(void)
{

	void uninitialize(void);

	//vars
	XSetWindowAttributes winAttribs;
	int defaultScreen;
	int styleMask;

	//OGL Frame Buffer attributes
	static int frameBufferAttributes[] = {GLX_RGBA,
										  GLX_DOUBLEBUFFER, True,
										  GLX_RED_SIZE, 8,
										  GLX_GREEN_SIZE, 8,
										  GLX_BLUE_SIZE, 8,
										  GLX_ALPHA_SIZE, 8,
										  None};

	gpDisplay = XOpenDisplay(NULL);
	if (gpDisplay == NULL)
	{
		printf(" ERROR : Unable To Open X Display.\n Exiting Now!!..");
		uninitialize();
		exit(0);
	}
	else
	{
		printf(" SUCCESS : Opened Display!..\n");
	}

	defaultScreen = XDefaultScreen(gpDisplay);

	gpXViual = glXChooseVisual(gpDisplay, defaultScreen, frameBufferAttributes);
	if (gpXViual == NULL)
	{
		printf(" ERROR : Unable To Get XVisualInfo.\n Exiting Now!!..");
		uninitialize();
		exit(0);
	}
	else
	{
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

	if (!gWindow)
	{
		printf(" ERROR : Failed To CreateWinodw.\n Exiting Now!!..");
		uninitialize();
		exit(0);
	}
	else
	{
		printf(" SUCCESS : Window Created Successfully!..\n");
	}

	XStoreName(gpDisplay, gWindow, "Static India");

	Atom wndMgrDelete = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(gpDisplay, gWindow, &wndMgrDelete, 1);

	XMapWindow(gpDisplay, gWindow);
}

void ToggleFullScreen(void)
{

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

void initialize(void)
{

	void uninitialize(void);
	void resize(int, int);

	gGlxContext = glXCreateContext(gpDisplay,
								   gpXViual,
								   NULL,
								   GL_TRUE);

	if (!gGlxContext)
	{
		printf(" ERROR : Failed To Get GLXContext.\n Exiting Now!!..");
		uninitialize();
		exit(0);
	}
	else
	{
		printf(" SUCCESS : Alotted With GLXContext!..\n");
	}

	glXMakeCurrent(gpDisplay, gWindow, gGlxContext);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	resize(gWinWidth, gWinHeight);
}

void resize(int width, int height)
{

	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f,
				   ((GLfloat)width / (GLfloat)height),
				   0.1f,
				   100.0f);
}

void display(void)
{
	//func
	void drawI1(void);
	void drawN(void);
	void drawD(void);
	void drawI2(void);
	void drawA(void);

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
    gluLookAt(0,0,3,0,0,0,0,1,0);
    
	glLineWidth(9.0f);

	drawI1();
	drawN();
	drawD();
	drawI2();
	drawA();

    glXSwapBuffers(gpDisplay, gWindow);
}

void drawI1() {

    //I
    glBegin(GL_LINES);
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(-1.5f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(-1.5f,-0.5f,0.0f);
    glEnd();


}

void drawN(){

    //N
    glBegin(GL_LINES);
    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(-1.1f,-0.5f,0.0f);

    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(-1.1f,0.5f,0.0f);

    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(-1.1f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(-0.6f,-0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(-0.6f,-0.5f,0.0f);

    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(-0.6f,0.5f,0.0f);
    glEnd();
}
void drawD(){
    //D
    glPointSize(6.4f);
	
	GLfloat G = GREEN_G, R = GREEN_R, B = GREEN_B;
	//int itr =0;
    glBegin(GL_POINTS);
    for(GLfloat angle = 3*M_PI/2; angle <= 5*M_PI/2 ; angle += 0.01f) {

		glColor3f(R,G,B);
        glVertex3f(-0.1+cosf(angle)*0.5f,sinf(angle)*0.5f,0.0f);

		//calcus for increment G = SAFFRON_G - GREEN_G /itr tya alelya avlue na increase kel only!!
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
		//itr++;
    }

    glEnd();
	
	
	glBegin(GL_LINES);
	glVertex3f(-0.21f,0.5f,0.0f);
	glVertex3f(-0.1f,0.5f,0.0f);

    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(-0.2f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(-0.2f,-0.5f,0.0f);

	glVertex3f(-0.21f,-0.5f,0.0f);
	glVertex3f(-0.1f,-0.5f,0.0f);
    glEnd();
}

void drawI2() {

	//I
    glBegin(GL_LINES);
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(0.7f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(0.7f,-0.5f,0.0f);
    glEnd();

}

void drawA(){
	//A
	glBegin(GL_LINES);

	//MID
	//Orange Strip
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
	glVertex3f(1.15f,0.025f,0.0f);
	glVertex3f(1.45f,0.025f,0.0f);
	//WHITE Strip
	glColor3f(1.0f,1.0f,1.0f);
	glVertex3f(1.15f,0.0f,0.0f);
	glVertex3f(1.45f,0.0f,0.0f);
	//Green Strip
    glColor3f(GREEN_R,GREEN_G,GREEN_B);
	glVertex3f(1.15f,-0.02f,0.0f);
	glVertex3f(1.45f,-0.02f,0.0f);

	//1st inCline
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
	glVertex3f(1.3f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
	glVertex3f(1.0f,-0.5f,0.0f);

	//2nd One
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
	glVertex3f(1.3f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
	glVertex3f(1.6f,-0.5f,0.0f);
	glEnd();

	glBegin(GL_POINTS);
	glColor3f(BLUE_R,BLUE_G,BLUE_B);
	glVertex3f(1.3f,0.0f,0.0f);
	glEnd();
}


void uninitialize(void)
{

	GLXContext current = glXGetCurrentContext();

	if (current != NULL && current == gGlxContext)
	{

		glXMakeCurrent(gpDisplay, 0, 0);
	}

	if (gGlxContext)
	{
		glXDestroyContext(gpDisplay, gGlxContext);
		gGlxContext = NULL;
	}

	if (gWindow)
	{
		XDestroyWindow(gpDisplay, gWindow);
	}

	if (gColormap)
	{
		XFreeColormap(gpDisplay, gColormap);
	}

	if (gpXViual)
	{
		free(gpXViual);
		gpXViual = NULL;
	}

	if (gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay = NULL;
	}

	printf("\n SUCCESS : Code Termination Successfully!!..\n\n");
}
