
#include<GL\freeglut.h>

//variables declarations
bool bIsFullScreen = false;
int iYear = 0;
int iDay = 0;



//function declarations
void Initialize(void);
void Uninitialize(void);
void Reshape(int, int);
void Display(void);
void Keyboard(unsigned char, int, int);
void Mouse(int, int, int, int);


int main(int argc, char* argv[]) {

	//code start 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Solar System");

	Initialize();

	//callback registrations
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(Mouse);
	glutCloseFunc(Uninitialize);

	glutMainLoop();

	return(0);

}

void Initialize(void) {

	//code First OpenGL Function
	glShadeModel(GL_FLAT);

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
	if (Height <= 0) {
		Height = 1;
	}

	glViewport(0, 0, (GLsizei)width, (GLsizei)Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(60.0f,
		(GLfloat)width / (GLfloat)Height,
		0.1f,
		20.0f);
}

void Display(void) {


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	glPushMatrix();

	glutWireSphere(1.0f, 20, 16);

	glRotatef((GLfloat)iYear, 0.0f, 1.0f, 0.0f);

	glTranslatef(2.0f, 0.0f, 0.0f);

	glRotatef((GLfloat)iDay, 0.0f, 1.0f, 0.0f);

	glutWireSphere(0.2f, 10, 8);

	glPopMatrix();

	glutSwapBuffers();

}

void Keyboard(unsigned char key, int x, int y) {

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

	case 'Y':
		iYear = (iYear + 5) % 360;
		glutPostRedisplay();
		break;

	case 'y':
		iYear = (iYear - 5) % 360;
		glutPostRedisplay();
		break;

	case 'D':
		iDay = (iDay + 10) % 360;
		glutPostRedisplay();
		break;

	case 'd':
		iDay = (iDay - 10) % 360;
		glutPostRedisplay(); 
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

