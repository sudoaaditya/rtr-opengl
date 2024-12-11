#include<GL\freeglut.h>

//variables declarations
bool bIsFullScreen = false;
GLfloat fRPyAngle = 0.0f;
GLfloat fRCuAngle = 0.0f;

//function declarations
void Initialize(void);
void Uninitialize(void);
void Reshape(int, int);
void Display(void);
void Keyboard(unsigned char, int, int);
void Mouse(int, int, int, int);
void update(void);

int main(int argc, char* argv[]) {
	
	//code start 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("MyFirstOpenGLProgram-Aaditya");

	Initialize();

	//callback registrations
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutIdleFunc(update);
	glutCloseFunc(Uninitialize);

	glutMainLoop();

	return(0);
	   
}

void Initialize(void) {

	//code First OpenGL Function
	glShadeModel(GL_SMOOTH);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

}

void Uninitialize() {
	//code  ToDO
}

void Reshape(int width, int Height) {

	//code
    if(Height <= 0) {
		Height = 1;
    }

    glViewport(0, 0, (GLsizei)width, (GLsizei)Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, 
        (GLfloat)width / (GLfloat)Height,
        0.1f,
        100.0f);
}

void Display(void) {
	//code
    void drawCube(void);
    void drawPyramid(void);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(-1.5f, 0.0f, -5.0f);
    glRotatef(fRPyAngle, 0.0f, 1.0f, 0.0f);

    drawPyramid();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(1.5f, 0.0f, -5.0f);
    glScalef(0.75f, 0.75f, 0.75f);
    glRotatef(fRCuAngle, 1.0f, 1.0f, 1.0f);

    drawCube();

	glutSwapBuffers();

}

void Keyboard(unsigned char key,int x,int y){

	//code
	switch (key) {

	case 27:
		glutLeaveMainLoop();
		break;

	case 'F':
	case 'f':

		if (bIsFullScreen == false) {
			glutFullScreen();
			bIsFullScreen = true;
		}
		else {
			glutLeaveFullScreen();
			bIsFullScreen = false;
		}
		break;
	}
}

void Mouse(int button, int state, int x, int y) {
	//code

	switch (button) {

	case GLUT_LEFT_BUTTON:
		break;

	case GLUT_RIGHT_BUTTON:
		glutLeaveMainLoop();
		break;
	}
}

void drawPyramid(void) {
        
    glBegin(GL_TRIANGLES);

    //FRontal Face { R G B}
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f); //Appex
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f); //left
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f); //right

    //Right face { R B G }
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f); //Appex
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);  //left
    glColor3f(0.0f, 1.0f, 0.0f);    
    glVertex3f(1.0f, -1.0f, -1.0f);     //right

    //back face { R G B}
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);   //Appex
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f); //left
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);    //right

    //Left Face
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 1.0f, 0.0f);   //Appex
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);    //left
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f); //right

    glEnd();
}

void drawCube(void) {

    glBegin(GL_QUADS);

    // Order of vertices  Right Top >> Left Top >> Left Bottom >> Right Bottom

    //Top Face with RED && Y pos
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);

    //Bottom Face with Green all Y -ve
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    //Front face with Blue and all Z +ve
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    //Back face with Cyan and all Z -ve
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);

    //Right face with Magenta with all X pos
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);

    //Right face with Magenta with all X pos
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);

    glEnd();
}


void update(void) {

    fRPyAngle += 0.009f;
    if(fRPyAngle >= 360.0f) {
        fRPyAngle = 0.0f;
    }

    fRCuAngle += 0.009f;
    if(fRCuAngle >= 360.0f) {
        fRCuAngle = 0.0f;
    }

	glutPostRedisplay();
}