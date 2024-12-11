//System Files
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

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
using namespace std;

// Global Variables
bool bFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXViual = NULL;
Colormap gColormap;
Window gWindow;
int gWinWidth = 800;
int gWinHeight = 600;
FILE *fptr;

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
    bool createLogFile(const char* );


	// Vars
	static int iWinWidth = gWinWidth;
	static int iWinHeight = gWinHeight;
	bool bDone = false;

    if(createLogFile("InnerCircLog.txt")) {
        printf(" SUCCESS : Log File Created!!..\n");
    }
    else {
        printf(" ERROR : Unable To Create Log File.\n Exiting Now!!..");
		uninitialize();
		exit(0);
    }

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


bool createLogFile(const char* fName) {
    
    fptr = fopen(fName, "w");
    if(fptr != NULL) {
        return(true);
    }  
    return(false); 

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

	XStoreName(gpDisplay, gWindow, "OGL_Graph");

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
	GLfloat fx1, fx2, fx3, fy1, fy2, fy3;
	GLfloat fArea, fRad, fPer;
	GLfloat fdAB, fdBC, fdAC;
	GLfloat fxCord, fyCord;
	static bool bToWrite = false;

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -1.0f);
	glLineWidth(2.0f);
	glBegin(GL_LINES);

	//X-axis
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(2.2f, 0.0f, 0.0f);
	glVertex3f(-2.2f, 0.0f, 0.0f);

	//Y-axis
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 2.0f, 0.0f);
	glVertex3f(0.0f, -2.0f, 0.0f);

	glEnd();

	//Graph
	float gfCoord = 0.05f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLineWidth(0.5f);
	glTranslatef(0.0f, 0.0f, -2.0f);
	glBegin(GL_LINES);
	glColor3f(0.0f, 0.0f, 1.0f);

	for (; gfCoord <= 2.0f; gfCoord += 0.06f) {

		//Horizontal Lines
		glVertex3f(2.0f, gfCoord, 0.0f);
		glVertex3f(-2.0f, gfCoord, 0.0f);

		glVertex3f(2.0f, -gfCoord, 0.0f);
		glVertex3f(-2.0f, -gfCoord, 0.0f);

		//Vertical lines
		glVertex3f(-gfCoord, 2.0f, 0.0f);
		glVertex3f(-gfCoord, -2.0f, 0.0f);

		glVertex3f(gfCoord, 2.0f, 0.0f);
		glVertex3f(gfCoord, -2.0f, 0.0f);

	}
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -4.0f);
	glLineWidth(2.0f);
	//coord of triangles
	fx1 = 0.0f;
	fy1 = 1.0f;
	fx2 = -1.0f;
	fy2 = -1.0f;
	fx3 = 1.0f;
	fy3 = -1.0f;

	glBegin(GL_LINES);
	glColor3f(1.0f, 1.0f, 0.0f);
	glVertex3f(fx1, fy1, 0.0f);
	glVertex3f(fx2, fy2, 0.0f);

	glVertex3f(fx2, fy2, 0.0f);
	glVertex3f(fx3, fy3, 0.0f);

	glVertex3f(fx3, fy3, 0.0f);
	glVertex3f(fx1, fy1, 0.0f);

	glEnd();

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

	if (!bToWrite) {
		fprintf(fptr, "fdAB = %f\nfdBC = %f\nfdAC = %f\nfPer =  %f\nfArea = %f\nfRad = %f\n", fdAB, fdBC, fdAC, fPer, fArea, fRad);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -4.0f);

	fxCord = (GLfloat)(((fdBC*fx1) + (fx2*fdAC) + (fx3*fdAB)) / (fPer * 2));
	fyCord = (GLfloat)(((fdBC*fy1) + (fy2*fdAC) + (fy3*fdAB)) / (fPer * 2));

	if (!bToWrite) {
		fprintf(fptr, "\n\nfx1 = %f\tfy1 = %f\nfx2 = %f\tfy2 = %f\nfx3 = %f\tfy3 = %f\n\nfxCoord = %f\tfyCord = %f\n", fx1, fy1, fx2, fy2, fx3, fy3, fxCord, fyCord);
	}

	glPointSize(2.0f);
	glBegin(GL_POINTS);
	glColor3f(1.0f, 1.0f, 0.0f);
	for (GLfloat i = 0; i < 2.0f*M_PI; i += 0.001f) {
		glVertex2f(fxCord + cosf(i)*fRad, fyCord + sinf(i)*fRad);
	}
	glEnd();

	//Rectangle 
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -4.0f);
	
	glBegin(GL_LINE_LOOP);

	glVertex3f(fx3, fy1, 0.0f);
	glVertex3f(fx2, fy1, 0.0f);

	glVertex3f(fx2, fy2, 0.0f);
	glVertex3f(fx3, fy3, 0.0f);

	glEnd();

	//Circle
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -4.0f);
	glBegin(GL_LINE_LOOP);
	for (GLfloat angle = 0.0f; angle < 2.0f*M_PI; angle += 0.001f) {
		glVertex2f(1.42f*cosf(angle), 1.42f*sinf(angle));
	}
	glEnd();

	bToWrite = true;
	glXSwapBuffers(gpDisplay, gWindow);
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
