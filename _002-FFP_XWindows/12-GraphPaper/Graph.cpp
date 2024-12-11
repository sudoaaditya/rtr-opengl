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

using namespace std;

// Global Variables
bool bFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXViual = NULL;
Colormap gColormap;
Window gWindow;
int gWinWidth = 800;
int gWinHeight = 600;
float giCord = 0.0f;

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
    giCord = 0.05f;
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -1.0f);

	glLineWidth(3.0f);

	glBegin(GL_LINES);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);
	glVertex2f(-1.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);
	glVertex2f(0.0f, -1.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glLineWidth(2.0f);

	for (; giCord <= 1.0f; giCord += 0.05f) {

		glVertex2f(1.0f, giCord);
		glVertex2f(-1.0f, giCord);

		glVertex2f(1.0f, -giCord);
		glVertex2f(-1.0f, -giCord);
		
		glVertex2f(giCord, 1.0f);
		glVertex2f(giCord, -1.0f);

		glVertex2f(-giCord, 1.0f);
		glVertex2f(-giCord, -1.0f);
		
	}

	glEnd();




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
