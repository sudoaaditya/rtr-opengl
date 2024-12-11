#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/Xutil.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>

using namespace std;

//global variables

bool bFullScreen = false;
XVisualInfo *gpXVisualInfo = NULL;
Display *gpDisplay = NULL;
Window gWindow;
Colormap gColorMap;

int iWinWidth = 800;
int iWinHeight = 600;


int main(void) {

	//Function Declaration
	void CreateWindow(void);
	void ToggleFullScreen(void);
	void uninitialize(void);

	//variables
	int winWidth = iWinWidth;
	int winHeight = iWinHeight;

	//code : Window Creation 
	CreateWindow();

	//Message Loop
	XEvent event;
	KeySym keysym;

	while(1) {

		XNextEvent(gpDisplay, &event);

		switch(event.type) {

			case MapNotify:		//WM_CREATE
				break;

			case KeyPress: 	//WM_KEYDOWN
				keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
				switch(keysym) {
					
					case XK_Escape:
						uninitialize();
						exit(0);
						break;

					case XK_F:
					case XK_f:
						if(bFullScreen == false) {
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

			case ButtonPress: //WM_*BUTTON*
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

			case MotionNotify:	//WM_MOUSEMOVE
					break;
			
					
			case ConfigureNotify:	//WM_SIZE
					winWidth = event.xconfigure.width;
					winHeight = event.xconfigure.height;
					break;

			case Expose:	//Paint
				break;

			case DestroyNotify:	//Combo of WM_DESTROIY & WM_CLOSE
				break;

			case 33:	//Close Button assigned key
				uninitialize();
				exit(0);
				break;

			}
	}

	uninitialize();
	exit(0);

}


void CreateWindow(void) {

	//func
	void uninitialize(void);

	//vars
	XSetWindowAttributes winAttribs;
	int defaultScreen, defaultDepth, styleMask;

	// 1: pen Connection & Get Display
	gpDisplay = XOpenDisplay(NULL);
	if(gpDisplay == NULL) {
		printf("ERROR :: Ubnable To Open X Display!..\nExiting Now!!..\n");
		uninitialize();
		exit(0);
	}

	// 2: Get Default Screen needed for XVisual : Acquiring default Screen i.e Primary one
	defaultScreen = XDefaultScreen(gpDisplay);

	// 3: Get default Bit Depth using above two things
	defaultDepth = XDefaultDepth(gpDisplay, defaultScreen);

	//4: Get Matching XVisualInfo from registered one based on Screen and bitDepth :: Actual Graphic HW ch SW Reprsentation 
	gpXVisualInfo = (XVisualInfo *) malloc(sizeof(XVisualInfo));
	if(gpXVisualInfo == NULL) {
		printf("ERROR :: Ubnable To Allocate Memory to XVisualInfo!..\nExiting Now!!..\n");
		uninitialize();
		exit(0);
	}

	XMatchVisualInfo(gpDisplay, defaultScreen, defaultDepth, TrueColor, gpXVisualInfo);
	if(gpXVisualInfo == NULL) {
		printf("ERROR :: Ubnable To Get Matching XVisualInfo!..\nExiting Now!!..\n");
		uninitialize();
		exit(0);
	}

	// 5 : Fill Window Attributes : 
	winAttribs.border_pixel = 0;	//no color to border give default.
	winAttribs.border_pixmap = 0;	//No Image to border
	winAttribs.background_pixel = BlackPixel(gpDisplay, defaultScreen);
	winAttribs.background_pixmap = 0;

	winAttribs.colormap = XCreateColormap(gpDisplay, 
				RootWindow(gpDisplay, gpXVisualInfo->screen),
				gpXVisualInfo->visual,
				AllocNone);
	gColorMap = winAttribs.colormap;
	winAttribs.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | 
				StructureNotifyMask | VisibilityChangeMask | PointerMotionMask;

	// 6 : Styles Set Karane
	styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;

	// 7 : Actaul Createion Of Window
	gWindow = XCreateWindow(gpDisplay,
				RootWindow(gpDisplay, gpXVisualInfo->screen),
				0, 0,
				iWinWidth, iWinHeight,
				0,
				gpXVisualInfo->depth,
				InputOutput,
				gpXVisualInfo->visual,
				styleMask,
				&winAttribs);

	if( !gWindow ) {
		printf("ERROR :: Ubnable To Create Window!!..\nExiting Now!!..\n");
		uninitialize();
		exit(0);
	}
	
	// 8 : Baptism of Window
	XStoreName(gpDisplay, gWindow, "First XWindows Window");

	// 9 : Close Button & Menu Close Button Chi Soy Lavane
	Atom DeleteWindowManager = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(gpDisplay, gWindow, &DeleteWindowManager, 1);

	// 10 : MApping of default Window with Expected window
	XMapWindow(gpDisplay, gWindow);

}


void ToggleFullScreen(void) {

	Atom wm_state;
	Atom fullscreen;
	XEvent xEve = {0};

	wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
	memset(&xEve, 0, sizeof(XEvent));

	xEve.type = ClientMessage;	//Message Type: Kon Message Pathvnar
	xEve.xclient.window = gWindow; 	// Kontya Window la
	xEve.xclient.message_type = wm_state; // Konta Message
	xEve.xclient.format = 32; 	//Size of Message
	xEve.xclient.data.l[0] = bFullScreen ? 0 : 1;	//Setting Nature whether the mode is Full or not

	fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	xEve.xclient.data.l[1] = fullscreen; // Detting actual Event is above is set to 0 then Event will not trigger and converse

	XSendEvent(gpDisplay,
	  	   RootWindow(gpDisplay, gpXVisualInfo->screen),
		   False,
		   StructureNotifyMask,
		   &xEve);
}


void uninitialize(void) {

	if(gWindow) {
		XDestroyWindow(gpDisplay, gWindow);
		gWindow = 0;
	}

	if(gColorMap) {
		XFreeColormap(gpDisplay, gColorMap);
		gColorMap = 0;

	}

	if(gpXVisualInfo) {
		free(gpXVisualInfo);
		gpXVisualInfo = NULL;

	}

	if(gpDisplay) {
		XCloseDisplay(gpDisplay);
		gpDisplay = NULL;
	}
}
