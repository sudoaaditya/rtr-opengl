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
	static int winWidth = iWinWidth;
	static int winHeight = iWinHeight;
	
	//Var For Text
	static XFontStruct *pXFontStruct = NULL;
	static GC gc;
	XGCValues gcValues;
	XColor textColor;
	char str[] = "Hello World!!..";
	int strLen, textWidth, fontHeight;

	//code : Window Creation 
	CreateWindow();

	//Message Loop
	XEvent event;
	KeySym keysym;
	char keys[26];	//Case Sensitive KeyPress

	while(1) {

		XNextEvent(gpDisplay, &event);

		switch(event.type) {

			case MapNotify:		//WM_CREATE
				pXFontStruct = XLoadQueryFont(gpDisplay, "-schumacher-clean-medium-r-normal--0-0-75-75-c-0-iso8859-1");
				break;

			case KeyPress: 	//WM_KEYDOWN
				keysym = XkbKeycodeToKeysym(gpDisplay, event.xkey.keycode, 0, 0);
				switch(keysym) {
					
					case XK_Escape:
						XFreeGC(gpDisplay, gc);
						XUnloadFont(gpDisplay,  pXFontStruct->fid);
						uninitialize();
						exit(0);
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

				gc = XCreateGC(gpDisplay, gWindow, 0, &gcValues);
				XSetFont(gpDisplay, gc, pXFontStruct->fid);
				XAllocNamedColor(gpDisplay, gColorMap, "green", &textColor, &textColor);
				XSetForeground(gpDisplay, gc, textColor.pixel);
				
				strLen = strlen(str);
				textWidth = XTextWidth(pXFontStruct, str, strLen);
				fontHeight = pXFontStruct->ascent - pXFontStruct->descent;

				XDrawString(gpDisplay,
					    gWindow,
					    gc,
					    (winWidth/2 - textWidth/2),
					    (winHeight/2 - fontHeight/2),
					    str,
					    strLen);
				break;

			case DestroyNotify:	//Combo of WM_DESTROIY & WM_CLOSE
				break;

			case 33:	//Close Button assigned key
				XFreeGC(gpDisplay, gc);
				XUnloadFont(gpDisplay,  pXFontStruct->fid);
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

	gpDisplay = XOpenDisplay(NULL);
	if(gpDisplay == NULL) {
		printf("ERROR :: Ubnable To Open X Display!..\nExiting Now!!..\n");
		uninitialize();
		exit(0);
	}

	defaultScreen = XDefaultScreen(gpDisplay);


	defaultDepth = XDefaultDepth(gpDisplay, defaultScreen);

 
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


	winAttribs.border_pixel = 0;
	winAttribs.border_pixmap = 0;
	winAttribs.background_pixel = BlackPixel(gpDisplay, defaultScreen);
	winAttribs.background_pixmap = 0;

	winAttribs.colormap = XCreateColormap(gpDisplay, 
				RootWindow(gpDisplay, gpXVisualInfo->screen),
				gpXVisualInfo->visual,
				AllocNone);
	gColorMap = winAttribs.colormap;
	winAttribs.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | 
				StructureNotifyMask | VisibilityChangeMask | PointerMotionMask;


	styleMask = CWBorderPixel | CWBackPixel | CWEventMask | CWColormap;


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
	

	XStoreName(gpDisplay, gWindow, "Hello World");


	Atom DeleteWindowManager = XInternAtom(gpDisplay, "WM_DELETE_WINDOW", True);
	XSetWMProtocols(gpDisplay, gWindow, &DeleteWindowManager, 1);

	XMapWindow(gpDisplay, gWindow);

}


void ToggleFullScreen(void) {

	Atom wm_state;
	Atom fullscreen;
	XEvent xEve = {0};

	wm_state = XInternAtom(gpDisplay, "_NET_WM_STATE", False);
	memset(&xEve, 0, sizeof(XEvent));

	xEve.type = ClientMessage;	
	xEve.xclient.window = gWindow; 	
	xEve.xclient.message_type = wm_state; 
	xEve.xclient.format = 32; 	
	xEve.xclient.data.l[0] = bFullScreen ? 0 : 1;	

	fullscreen = XInternAtom(gpDisplay, "_NET_WM_STATE_FULLSCREEN", False);
	xEve.xclient.data.l[1] = fullscreen; 

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
