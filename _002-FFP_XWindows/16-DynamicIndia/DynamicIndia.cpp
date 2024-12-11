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

//for AUDIO
#include<AL/al.h>
#include<AL/alc.h>
#include<AL/alut.h>

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

//Audio Pars
ALCcontext *context;
ALCdevice *device;
char*     alBuffer;             //data for the buffer
ALenum alFormatBuffer;    //buffer format
ALsizei   alFreqBuffer;       //frequency
long       alBufferLen;        //bit depth
ALboolean    alLoop;         //loop
unsigned int alSource;      //source
unsigned int alSampleSet;


// Global Variables
bool bFullScreen = false;
Display *gpDisplay = NULL;
XVisualInfo *gpXViual = NULL;
Colormap gColormap;
Window gWindow;
int gWinWidth = 800;
int gWinHeight = 600;

//Animation Parameter
bool bI1Reached = false, bNReached = false, bDReached = false,bI2Reached = false, bAReached = false, bPlaneReached = false;
static float fTransSpeed = 0.004f;


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
    void createAudioContext(void);
    void PlaySound();
    void deleteAudioContext();


	// Vars
	static int iWinWidth = gWinWidth;
	static int iWinHeight = gWinHeight;
	bool bDone = false;

	// Create Window
	CreateWindow();

	// Now Initialize Window
	initialize();
    ToggleFullScreen();
    createAudioContext();

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

	XStoreName(gpDisplay, gWindow, "!...  INDIA  ...!");

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

    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
	void drawRefLines(void);
	void drawPlane1(void);
	void drawPlane2(void);
	void drawPlane3(void);

    static bool bPlaySong = false;
    void PlaySound(void);

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0,0,3,0,0,0,0,1,0);

	if(!bPlaySong) {
        PlaySound();
    }

	glLineWidth(9.0f);
	drawI1();
	
	if(bAReached == true) {
		drawN();
	}

	if(bI2Reached == true) {
		drawD();
	}

	if(bNReached ==true) {
		drawI2();
	}

	if(bI1Reached == true) {
		drawA();
	}

	if(bDReached == true) {

		drawPlane1();
		drawPlane2();
		drawPlane3();
	}
	bPlaySong = true;
    glXSwapBuffers(gpDisplay, gWindow);
}

void drawI1() {

	static GLfloat bXTrans = -0.8f;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glTranslatef(bXTrans,0.0f,-3.0f);
    glBegin(GL_LINES);
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(-1.5f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(-1.5f,-0.5f,0.0f);
    glEnd();

	bXTrans += fTransSpeed;
	//fprintf(fptr,"Val = %f\n",bXTrans);
	if(bXTrans >= 0.0f){
		bXTrans = 0.00f;
		bI1Reached = true;
	}

}

void drawN(){

	static GLfloat bYTrans = 2.0f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f,bYTrans,-3.0f);

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

	bYTrans -= fTransSpeed;
	if(bYTrans <= 0.0f) {
		bYTrans = 0.0f;
		bNReached = true;
	}
	
}
	
void drawD(){
    //D
    glPointSize(6.4f);
 
	
	GLfloat G = GREEN_G, R = GREEN_R, B = GREEN_B;
	static GLfloat A = 0.0f;
	//static int itr =0;
    glBegin(GL_POINTS);
    for(GLfloat angle = 3*M_PI/2; angle <= 5*M_PI/2 ; angle += 0.01f) {

		glColor4f(R,G,B,A);
        glVertex3f(-0.1+cosf(angle)*0.5f,sinf(angle)*0.5f,0.0f);

		//calcus for increment G = SAFFRON_G - GREEN_G /itr tya alelya avlue na increase kel only!!
		G += 0.00021f;
		R += 0.0029f;
		B += 0.00054f;

    }

    glEnd();
	

	static GLfloat Sr=0.0f, Sg = 0.0f, Sb = 0.0f;
	static GLfloat Gr=0.0f, Gg = 0.0f, Gb = 0.0f;

	glBegin(GL_LINES);

 	glColor3f(Sr,Sg,Sb);
	glVertex3f(-0.21f,0.5f,0.0f);
	glVertex3f(-0.1f,0.5f,0.0f);

    glVertex3f(-0.2f,0.5f,0.0f);

    glColor3f(Gr,Gg,Gb);
    glVertex3f(-0.2f,-0.5f,0.0f);

	glVertex3f(-0.21f,-0.5f,0.0f);
	glVertex3f(-0.1f,-0.5f,0.0f);

    glEnd();

	//fprintf(fptr,"Here Val SR =%d\n",iCntStat);

	Sr += 0.001f;
	Sg += 0.0006f;
	Sb += 0.0002f; 
	if(Sr >= SAFFRON_R) {
		//fprintf(fptr,"Here Val itr =%d\n",itr);
		bDReached = true;
		Sr = SAFFRON_R;
	}
	if(Sg >= SAFFRON_G) {
		Sg = SAFFRON_G;
	}
	if(Sb >= SAFFRON_B) {
		Sb = SAFFRON_B;
	}

	Gr += 0.00007f;
	Gg += 0.0006f;
	Gb += 0.000027f;

	if(Gr >= GREEN_R){
		Gr = GREEN_R;
	}
	if(Gg >= GREEN_G){
		Gg = GREEN_G;
	}
	if(Gb >= GREEN_B){
		Gb = GREEN_B;
	}
		
	A += 0.00057f;
	if(A >= 1.0f){
		A = 1.0f;
	}

}

void drawI2() {

	static GLfloat bYTrans = -2.0f;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f,bYTrans,-3.0f);

	//I
    glBegin(GL_LINES);
    glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
    glVertex3f(0.7f,0.5f,0.0f);

    glColor3f(GREEN_R,GREEN_G,GREEN_B);
    glVertex3f(0.7f,-0.5f,0.0f);
    glEnd();

	bYTrans += fTransSpeed;
	if(bYTrans >= 0.0f) {
		bYTrans = 0.0f;
		bI2Reached = true;
	}

}

void drawA(){

	static GLfloat bXTrans = 0.8f, Alpaha = 0.0f;
	static int inCnt = 0;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glTranslatef(bXTrans,0.0f,-3.0f);	

	if(bPlaneReached == true) {
		glBegin(GL_LINES);
		glColor4f(SAFFRON_R,SAFFRON_G,SAFFRON_B,Alpaha);
		glVertex3f(1.16f,0.020f,0.0f);
		glVertex3f(1.44f,0.020f,0.0f);

		glColor4f(1.0f,1.0f,1.0f,Alpaha);
		glVertex3f(1.15f,0.0f,0.0f);
		glVertex3f(1.45f,0.0f,0.0f);

		glColor4f(GREEN_R,GREEN_G,GREEN_B,Alpaha);
		glVertex3f(1.15f,-0.020f,0.0f);	
		glVertex3f(1.45f,-0.020f,0.0f);

		glEnd();
	}	
	//A
	glBegin(GL_LINES);

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


	bXTrans -= fTransSpeed;
	if(bXTrans<= 0.0f){
		bXTrans = 0.0f;
		bAReached = true;
	}
	if(bPlaneReached == true) {
		if(inCnt == 5) {
			Alpaha += 0.04f;
			if(Alpaha >= 1.0f) {
				Alpaha = 1.0f;
			}
			inCnt =0;
		}
		inCnt++;
	}
	
}


void drawPlane1(void) {

	void drawPlane2(void);

	static GLfloat fxTrans=-4.0f;

	//drawPlane2();
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(fxTrans,0.0f,-5.0f);

	glBegin(GL_TRIANGLES);
	//Powder Blue 186 226  238
	glColor3f(0.729f,0.886f,0.933f);
	glVertex3f(-0.02f,0.0f,0.0f);
	glVertex3f(-0.3f,0.07f,0.0f);
	glVertex3f(-0.3f,-0.07f,0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-0.3f,0.07f,0.0f);
	glVertex3f(-0.6f,0.09f,0.0f);
	glVertex3f(-0.9f,0.06f,0.0f);
	glVertex3f(-0.9f,-0.06f,0.0f);
	glVertex3f(-0.6f,-0.09f,0.0f);
	glVertex3f(-0.3f,-0.07f,0.0f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(-0.50,0.08,0.0f);
	glVertex3f(-0.7f,0.3f,0.0f);
	glVertex3f(-0.7f,0.075f,0.0f);

	glVertex3f(-0.50,-0.08,0.0f);
	glVertex3f(-0.7f,-0.3f,0.0f);
	glVertex3f(-0.7f,-0.075f,0.0f);

	glVertex3f(-0.8f,0.068f,0.0f);
	glVertex3f(-0.9f,0.2f,0.0f);
	glVertex3f(-0.9f,0.06f,0.0f);

	glVertex3f(-0.8f,-0.068f,0.0f);
	glVertex3f(-0.9f,-0.2f,0.0f);
	glVertex3f(-0.9f,-0.06f,0.0f);
	glEnd();

	glLineWidth(1.5f);
	glBegin(GL_LINES);
	glColor3f(0.0f,0.0f,0.0f);
	//I
	glVertex3f(-0.64f,0.05f,0.0f);
	glVertex3f(-0.64f,-0.05f,0.0f);

	//A
	glVertex3f(-0.61f,0.05f,0.0f);		
	glVertex3f(-0.63f,-0.05f,0.0f);

	glVertex3f(-0.62f,0.0f,0.0f);
	glVertex3f(-0.60,0.0f,0.0f);

	glVertex3f(-0.61f,0.05f,0.0f);	
	glVertex3f(-0.59f,-0.05f,0.0f);

	//F	
	glVertex3f(-0.58f,0.05f,0.0f);
	glVertex3f(-0.58f,-0.05f,0.0f);
	
	glVertex3f(-0.58f,0.05f,0.0f);
	glVertex3f(-0.55f,0.05f,0.0f);

	glVertex3f(-0.58f,0.01f,0.0f);
	glVertex3f(-0.56f,0.01f,0.0f);

	glEnd();

	glLineWidth(9.0f);
	glBegin(GL_LINES);
	glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
	glVertex3f(-0.9f,0.040f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3f(-1.3f,0.040f,0.0f);

	glColor3f(1.0f,1.0f,1.0f);
	glVertex3f(-0.9f,0.0f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3f(-1.3f,0.0f,0.0f);

	glColor3f(GREEN_R,GREEN_G,GREEN_B);
	glVertex3f(-0.9f,-0.040f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);	
	glVertex3f(-1.3f,-0.040f,0.0f);

	glEnd();


	fxTrans += 0.01f;


}

void drawPlane2(void) {

	static GLfloat fxTrans=-4.0f,fyTrans = 2.3f, angle = -90.0f, fIxTrans = 3.8f, fIyTrans =0.0f, Iangle = 0.0f ;
	static bool bInvTrans = false;
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if(!bInvTrans) {
		glTranslatef(fxTrans, fyTrans,-5.0f);
		glRotatef(angle,0.0f,0.0f,1.0f);
	}
	else {
		glTranslatef(fIxTrans, fIyTrans,-5.0f);
		glRotatef(Iangle,0.0f,0.0f,1.0f);
	}

	glBegin(GL_TRIANGLES);
	//Powder Blue 186 226  238
	glColor3f(0.729f,0.886f,0.933f);
	glVertex3f(-0.02f,0.0f,0.0f);
	glVertex3f(-0.3f,0.07f,0.0f);
	glVertex3f(-0.3f,-0.07f,0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-0.3f,0.07f,0.0f);
	glVertex3f(-0.6f,0.09f,0.0f);
	glVertex3f(-0.9f,0.06f,0.0f);
	glVertex3f(-0.9f,-0.06f,0.0f);
	glVertex3f(-0.6f,-0.09f,0.0f);
	glVertex3f(-0.3f,-0.07f,0.0f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(-0.50,0.08,0.0f);
	glVertex3f(-0.7f,0.3f,0.0f);
	glVertex3f(-0.7f,0.075f,0.0f);

	glVertex3f(-0.50,-0.08,0.0f);
	glVertex3f(-0.7f,-0.3f,0.0f);
	glVertex3f(-0.7f,-0.075f,0.0f);

	glVertex3f(-0.8f,0.068f,0.0f);
	glVertex3f(-0.9f,0.2f,0.0f);
	glVertex3f(-0.9f,0.06f,0.0f);

	glVertex3f(-0.8f,-0.068f,0.0f);
	glVertex3f(-0.9f,-0.2f,0.0f);
	glVertex3f(-0.9f,-0.06f,0.0f);
	glEnd();

	glLineWidth(1.5f);
	glBegin(GL_LINES);
	glColor3f(0.0f,0.0f,0.0f);
	//I
	glVertex3f(-0.64f,0.05f,0.0f);
	glVertex3f(-0.64f,-0.05f,0.0f);

	//A
	glVertex3f(-0.61f,0.05f,0.0f);		
	glVertex3f(-0.63f,-0.05f,0.0f);

	glVertex3f(-0.62f,0.0f,0.0f);
	glVertex3f(-0.60,0.0f,0.0f);

	glVertex3f(-0.61f,0.05f,0.0f);	
	glVertex3f(-0.59f,-0.05f,0.0f);

	//F	
	glVertex3f(-0.58f,0.05f,0.0f);
	glVertex3f(-0.58f,-0.05f,0.0f);
	
	glVertex3f(-0.58f,0.05f,0.0f);
	glVertex3f(-0.55f,0.05f,0.0f);

	glVertex3f(-0.58f,0.01f,0.0f);
	glVertex3f(-0.56f,0.01f,0.0f);

	glEnd();

	glLineWidth(9.0f);
	glBegin(GL_LINES);
	glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
	glVertex3f(-0.9f,0.040f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3f(-1.3f,0.040f,0.0f);

	glColor3f(1.0f,1.0f,1.0f);
	glVertex3f(-0.9f,0.0f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3f(-1.3f,0.0f,0.0f);

	glColor3f(GREEN_R,GREEN_G,GREEN_B);
	glVertex3f(-0.9f,-0.040f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);	
	glVertex3f(-1.3f,-0.040f,0.0f);

	glEnd();

	if(!bInvTrans) {
		fxTrans += 0.01f;
		fyTrans -= 0.01f;
		if(fxTrans >= 3.0f){
			bPlaneReached = true;
		}
		if(fxTrans >= 3.8f){
			fxTrans = 3.8f;
			bInvTrans = true;
		}
		if(fyTrans <= 0.0f){
			fyTrans = 0.0f;
		}
		angle += 0.4f;
		if(angle >= 0.0f){
			angle = 0.0f;
		}
	}
	else {
		fIxTrans += 0.01f;
		fIyTrans -= 0.01f;
		
		Iangle -= 0.4f;

	}


}

void drawPlane3(void) {

	static GLfloat fxTrans=-4.0f,fyTrans = -2.3f, angle = 90.0f, fIxTrans = 3.8f, fIyTrans =0.0f, Iangle = 0.0f ;
	static bool bInvTrans = false;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glTranslatef(-3.5f,1.8f,-5.0f);
	//glTranslatef(0.0f,0.0f,-5.0f);
	if(!bInvTrans) {
		glTranslatef(fxTrans, fyTrans,-5.0f);
		glRotatef(angle,0.0f,0.0f,1.0f);
	}
	else {
		glTranslatef(fIxTrans, fIyTrans,-5.0f);
		glRotatef(Iangle,0.0f,0.0f,1.0f);
	}	
	//glTranslatef(-1.5f+cosf(angle)*1.25f,1.50f+sinf(angle)*1.25f,-5.0f);
	// P-Y,p-X
	//glTranslatef(-1.8f, -2.64f,-5.0f);

	glBegin(GL_TRIANGLES);
	//Powder Blue 186 226  238
	glColor3f(0.729f,0.886f,0.933f);
	glVertex3f(-0.02f,0.0f,0.0f);
	glVertex3f(-0.3f,0.07f,0.0f);
	glVertex3f(-0.3f,-0.07f,0.0f);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex3f(-0.3f,0.07f,0.0f);
	glVertex3f(-0.6f,0.09f,0.0f);
	glVertex3f(-0.9f,0.06f,0.0f);
	glVertex3f(-0.9f,-0.06f,0.0f);
	glVertex3f(-0.6f,-0.09f,0.0f);
	glVertex3f(-0.3f,-0.07f,0.0f);
	glEnd();

	glBegin(GL_TRIANGLES);
	glVertex3f(-0.50,0.08,0.0f);
	glVertex3f(-0.7f,0.3f,0.0f);
	glVertex3f(-0.7f,0.075f,0.0f);

	glVertex3f(-0.50,-0.08,0.0f);
	glVertex3f(-0.7f,-0.3f,0.0f);
	glVertex3f(-0.7f,-0.075f,0.0f);

	glVertex3f(-0.8f,0.068f,0.0f);
	glVertex3f(-0.9f,0.2f,0.0f);
	glVertex3f(-0.9f,0.06f,0.0f);

	glVertex3f(-0.8f,-0.068f,0.0f);
	glVertex3f(-0.9f,-0.2f,0.0f);
	glVertex3f(-0.9f,-0.06f,0.0f);
	glEnd();

	glLineWidth(1.5f);
	glBegin(GL_LINES);
	glColor3f(0.0f,0.0f,0.0f);
	//I
	glVertex3f(-0.64f,0.05f,0.0f);
	glVertex3f(-0.64f,-0.05f,0.0f);

	//A
	glVertex3f(-0.61f,0.05f,0.0f);		
	glVertex3f(-0.63f,-0.05f,0.0f);

	glVertex3f(-0.62f,0.0f,0.0f);
	glVertex3f(-0.60,0.0f,0.0f);

	glVertex3f(-0.61f,0.05f,0.0f);	
	glVertex3f(-0.59f,-0.05f,0.0f);

	//F	
	glVertex3f(-0.58f,0.05f,0.0f);
	glVertex3f(-0.58f,-0.05f,0.0f);
	
	glVertex3f(-0.58f,0.05f,0.0f);
	glVertex3f(-0.55f,0.05f,0.0f);

	glVertex3f(-0.58f,0.01f,0.0f);
	glVertex3f(-0.56f,0.01f,0.0f);

	glEnd();

	glLineWidth(9.0f);
	glBegin(GL_LINES);
	glColor3f(SAFFRON_R,SAFFRON_G,SAFFRON_B);
	glVertex3f(-0.9f,0.040f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3f(-1.3f,0.040f,0.0f);

	glColor3f(1.0f,1.0f,1.0f);
	glVertex3f(-0.9f,0.0f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);
	glVertex3f(-1.3f,0.0f,0.0f);

	glColor3f(GREEN_R,GREEN_G,GREEN_B);
	glVertex3f(-0.9f,-0.040f,0.0f);
	glColor3f(0.0f,0.0f,0.0f);	
	glVertex3f(-1.3f,-0.040f,0.0f);

	glEnd();

	if(!bInvTrans) {
		fxTrans += 0.01f;
		fyTrans += 0.01f;
		if(fxTrans >= 3.8f){
			fxTrans = 3.8f;
			bInvTrans = true;
		}
		if(fyTrans >= 0.0f){
			fyTrans = 0.0f;
		}
		angle -= 0.4f;
		if(angle <= 0.0f){
			angle = 0.0f;
		}
	}
	else {
		fIxTrans += 0.01f;
		fIyTrans += 0.01f;

		Iangle += 0.4f;
	}
}

void createAudioContext(void) {
    void uninitialize(void);

    device = alcOpenDevice(NULL);
    if (device == NULL)
    {
        printf(" ERROR : Cannot Open Audio Context../n Exiting NOw!\n");
        uninitialize();
        exit(0);
    }
 
    //Create a context
    context=alcCreateContext(device,NULL);
 
    //Set active context
    alcMakeContextCurrent(context);
    
        //load the wave file
    alutLoadWAVFile((ALbyte*)"VandeMataram.wav",&alFormatBuffer, (void **) &alBuffer,(ALsizei*)&alBufferLen, &alFreqBuffer, &alLoop);
    
    //create a source
    alGenSources(1, &alSource);
    
    //create  buffer
    alGenBuffers(1, &alSampleSet);
    
    //put the data into our sampleset buffer
    alBufferData(alSampleSet, alFormatBuffer, alBuffer, alBufferLen, alFreqBuffer);
    
    //assign the buffer to this source
    alSourcei(alSource, AL_BUFFER, alSampleSet);
    
    //release the data
    alutUnloadWAV(alFormatBuffer, alBuffer, alBufferLen, alFreqBuffer);
}

void PlaySound() {

    alSourcei(alSource,AL_LOOPING,AL_FALSE);    
    //play
    alSourcePlay(alSource);
}

void StopSound() {
    alSourceStop(alSource);
}

void deleteAudioContext() {

    alDeleteSources(1,&alSource);
    
    //delete our buffer
    alDeleteBuffers(1,&alSampleSet);
    
    context=alcGetCurrentContext();
    
    //Get device for active context
    device=alcGetContextsDevice(context);
    
    //Disable context
    alcMakeContextCurrent(NULL);
    
    //Release context(s)
    alcDestroyContext(context);
    
    //Close device
    alcCloseDevice(device);
}


void uninitialize(void)
{

    deleteAudioContext();

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
