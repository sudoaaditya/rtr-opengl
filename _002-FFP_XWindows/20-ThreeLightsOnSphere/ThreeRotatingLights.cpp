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

static GLXContext gGlxContext;

//Lights Vars
bool bLights = false;
GLfloat flightAngleZero = 0.0f;
GLfloat flightAngleOne = 0.0f;
GLfloat flightAngleTwo = 0.0f;

GLfloat lightAmbientZero[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuseZero[] = { 1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightSpecularZero[] = { 1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightPositionZero[] = { 0.0f, 0.0f, 0.0f, 1.0f};

GLfloat lightAmbientOne[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuseOne[] = { 0.0f, 1.0f, 0.0f, 1.0f};
GLfloat lightSpecularOne[] = { 0.0f, 1.0f, 0.0f, 1.0f};
GLfloat lightPositionOne[] = { 0.0f, 0.0f, 0.0f, 1.0f};


GLfloat lightAmbientTwo[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuseTwo[] = { 0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightSpecularTwo[] = { 0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightPositionTwo[] = { 0.0f, 0.0f, 0.0f, 1.0f};


GLfloat MaterialAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat MaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat MaterialSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat MaterialShininess[] = { 50.0f }; // { 128.0f } //juni value

GLUquadric *Sphere = NULL;

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
    if (initialize() == 0)
    {
        printf(" SUCCESS : Initialization Successful!.\n");
    }
    else
    {
        printf(" ERROR : Initialization Failed!!..\n");
        uninitialize();
        exit(0);
    }

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
                printf(" MessageLoop: Map Notify called!.\n");
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

                default:
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
        update();
        display();
    }
    uninitialize();
    return (0);
}

void CreateWindow(void) {

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
                                          GLX_DEPTH_SIZE, 24,
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

    XStoreName(gpDisplay, gWindow, "Single Light On Sphere");

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

int initialize(void)
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

    if (glXMakeCurrent(gpDisplay, gWindow, gGlxContext) != True)
    {
        return (-1);
    }
    else
    {
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
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbientOne);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuseOne);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecularOne);
    glEnable(GL_LIGHT1);
 
    glLightfv(GL_LIGHT2, GL_AMBIENT, lightAmbientTwo);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, lightDiffuseTwo);
    glLightfv(GL_LIGHT2, GL_SPECULAR, lightSpecularTwo);
    glEnable(GL_LIGHT2);


    glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    resize(gWinWidth, gWinHeight);

    return (0);
}

void resize(int width, int height) {

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

void display(void) {

    void drawSphere(void);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();

    gluLookAt(0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    //For Light Zero along X
    glPushMatrix();
    glRotatef(flightAngleZero, 1.0f, 0.0f, 0.0f);
    //chng Y Par
    lightPositionZero[1] = flightAngleZero;
    glLightfv(GL_LIGHT0, GL_POSITION, lightPositionZero);
    glPopMatrix();

    //For Light One along Y
    glPushMatrix();
    glRotatef(flightAngleOne, 0.0f, 1.0f, 0.0f);
    //chng z Par
    lightPositionOne[2] = flightAngleOne;
    glLightfv(GL_LIGHT1, GL_POSITION, lightPositionOne);
    glPopMatrix();
    
    //For Light Zero along Z
    glPushMatrix();
    glRotatef(flightAngleTwo, 0.0f, 0.0f, 1.0f);
    //chng X Par
    lightPositionTwo[0] = flightAngleTwo;
    glLightfv(GL_LIGHT2, GL_POSITION, lightPositionTwo);
    glPopMatrix();
    
    drawSphere();

    glPopMatrix();

    glXSwapBuffers(gpDisplay, gWindow);
}

void drawSphere(void) {

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    Sphere = gluNewQuadric();

    gluSphere(Sphere, 1.0f, 100, 100);
    
}

void update(void) {

    flightAngleZero += 0.9f;
    if(flightAngleZero >= 360.0f) {
        flightAngleZero = 0.0f;
    }

    flightAngleOne += 0.9f;
    if(flightAngleOne >= 360.0f) {
        flightAngleOne = 0.0f;
    }

    flightAngleTwo += 0.9f;
    if(flightAngleTwo >= 360.0f) {
        flightAngleTwo = 0.0f;
    }

}
void uninitialize(void) {

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