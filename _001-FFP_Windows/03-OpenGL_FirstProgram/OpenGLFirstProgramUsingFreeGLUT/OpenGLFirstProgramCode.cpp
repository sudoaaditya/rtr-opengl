#include<GL\freeglut.h>

//variables declarations
bool bIsFullScreen = false;

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
	glutCloseFunc(Uninitialize);

	glutMainLoop();

	return(0);
	   
}

void Initialize(void) {

	//code First OpenGL Function
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void Uninitialize() {
	//code  ToDO
}

void Reshape(int width, int Height) {

	//code
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
}

void Display(void) {
	//code
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_TRIANGLES);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(-1.0f, -1.0f);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex2f(1.0f, -1.0f);

	glEnd();
	glFlush();

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

