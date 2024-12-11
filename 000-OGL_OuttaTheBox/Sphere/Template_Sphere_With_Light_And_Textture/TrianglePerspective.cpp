#include <windows.h>
#include <stdio.h> // for FILE I/O
#include <gl\glew.h> // for GLSL extensions IMPORTANT : This Line Should Be Before #include<gl\gl.h> And #include<gl\glu.h>
#include <gl/GL.h>
#include "vmath.h"
#include "LightHeader.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

using namespace vmath;

enum
{
	VDG_ATTRIBUTE_VERTEX = 0,
	VDG_ATTRIBUTE_COLOR,
	VDG_ATTRIBUTE_NORMAL,
	VDG_ATTRIBUTE_TEXTURE0,
};

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
FILE *gpFile = NULL;

HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbActiveWindow = false;
bool gbEscapeKeyIsPressed = false;
bool gbFullscreen = false;

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLuint gVao_Triangle;
GLuint gVbo_Position;
GLuint gVbo_Color;
GLuint gMVPUniform;

GLuint model_matrix_uniform, view_matrix_uniform, projection_matrix_uniform;

mat4 gPerspectiveProjectionMatrix;

#define SPHERE_VDG_PI  3.14159265358979323846f

unsigned short* sphere_elements;
GLfloat* sphere_verts;
GLfloat* sphere_norms;
GLfloat* sphere_texCoords;

GLint sphere_numElements;
GLint sphere_maxElements;
GLint sphere_numVertices;

GLuint sphere_vbo_position;
GLuint sphere_vbo_normal;
GLuint sphere_vbo_texture;
GLuint sphere_vbo_index;
GLuint sphere_vao;

void spehereMesh_allocate(int numIndices);
void sphereMesh_AddTriangle(GLfloat **single_vertex, GLfloat **single_normal, GLfloat **single_texture);
void sphereMesh_prepareToDraw();

int getSphereIndexCount();
int getSphereVertexCount();
void SpherenormalizeVector(GLfloat *v);
bool isFoundIdentical(GLfloat val1, GLfloat val2, GLfloat diff);
void cleanupSphereMeshData();
void cleanupSphereBuffers();

void makeSphere(int fRadius, int iSlices, int iStacks);
void drawSphere();


void spehereMesh_allocate(int numIndices)
{

	cleanupSphereMeshData();

	sphere_maxElements = numIndices;
	sphere_numElements = 0;
	sphere_numVertices = 0;

	int iNumIndices = numIndices / 3;

	sphere_elements = (unsigned short*)malloc(iNumIndices * 3 * sizeof(unsigned short));// 3 is x,y,z and 2 is sizeof short
	sphere_verts = (GLfloat*)malloc(iNumIndices * 3 * sizeof(GLfloat)); // 3 is x,y,z and 4 is sizeof float
	sphere_norms = (GLfloat*)malloc(iNumIndices * 3 * sizeof(GLfloat)); // 3 is x,y,z and 4 is sizeof float
	sphere_texCoords = (GLfloat*)malloc(iNumIndices * 2 * sizeof(GLfloat));// 2 is s,t and 4 is sizeof float

}


// Add 3 vertices, 3 normal and 2 texcoords i.e. one triangle to the geometry.
// This searches the current list for identical vertices (exactly or nearly) and
// if one is found, it is added to the index array.
// if not, it is added to both the index array and the vertex array.


int getSphereIndexCount()
{
	return sphere_numElements;
}

int getSphereVertexCount()
{
	return sphere_numVertices;
}

void SpherenormalizeVector(GLfloat *v)
{
	// square the vector length
	GLfloat squaredVectorLength = ((v[0] * v[0]) + (v[1] * v[1]) + (v[2] * v[2]));

	// get square root of above 'squared vector length'
	GLfloat squareRootOfSquaredVectorLength = (GLfloat)sqrt(squaredVectorLength);

	// scale the vector with 1/squareRootOfSquaredVectorLength
	v[0] = v[0] * 1.0f / squareRootOfSquaredVectorLength;
	v[1] = v[1] * 1.0f / squareRootOfSquaredVectorLength;
	v[2] = v[2] * 1.0f / squareRootOfSquaredVectorLength;
}

bool isFoundIdentical(GLfloat val1, GLfloat val2, GLfloat diff)
{
	if (fabs(val1 - val2) < diff)
		return true;
	else
		return false;
}

void cleanupSphereMeshData()
{

	//dispose
	// code
	if (sphere_elements != NULL)
	{
		free(sphere_elements);
		sphere_elements = NULL;
	}

	if (sphere_verts != NULL)
	{
		free(sphere_verts);
		sphere_verts = NULL;
	}

	if (sphere_norms != NULL)
	{
		free(sphere_norms);
		sphere_norms = NULL;
	}

	if (sphere_texCoords != NULL)
	{
		free(sphere_texCoords);
		sphere_texCoords = NULL;
	}
	//

}

inline void cleanupSphereBuffers()
{
	if (sphere_vao)
	{
		glDeleteBuffers(1, &sphere_vao);
		sphere_vao = (GLint)NULL;
	}

	if (sphere_vbo_index)
	{
		glDeleteBuffers(1, &sphere_vbo_index);
		sphere_vbo_index = (GLint)NULL;
	}

	if (sphere_vbo_texture)
	{
		glDeleteBuffers(1, &sphere_vbo_texture);
		sphere_vbo_texture = (GLint)NULL;
	}

	if (sphere_vbo_normal)
	{
		glDeleteBuffers(1, &sphere_vbo_normal);
		sphere_vbo_normal = (GLint)NULL;
	}

	if (sphere_vbo_position)
	{
		glDeleteBuffers(1, &sphere_vbo_position);
		sphere_vbo_position = (GLint)NULL;
	}
}

void sphereMesh_AddTriangle(GLfloat **single_vertex, GLfloat **single_normal, GLfloat **single_texture)
{
	const GLfloat diff = 0.00001f;
	int i, j;


	SpherenormalizeVector(single_normal[0]);
	SpherenormalizeVector(single_normal[1]);
	SpherenormalizeVector(single_normal[2]);


	for (i = 0; i < 3; i++)
	{
		/* code */

		for (j = 0; j < sphere_numVertices; j++)
		{
			/* code */

			if (isFoundIdentical(sphere_verts[j * 3], single_vertex[i][0], diff) &&
				isFoundIdentical(sphere_verts[(j * 3) + 1], single_vertex[i][1], diff) &&
				isFoundIdentical(sphere_verts[(j * 3) + 2], single_vertex[i][2], diff) &&

				isFoundIdentical(sphere_norms[j * 3], single_normal[i][0], diff) &&
				isFoundIdentical(sphere_norms[(j * 3) + 1], single_normal[i][1], diff) &&
				isFoundIdentical(sphere_norms[(j * 3) + 2], single_normal[i][2], diff) &&

				isFoundIdentical(sphere_texCoords[j * 2], single_texture[i][0], diff) &&
				isFoundIdentical(sphere_texCoords[(j * 2) + 1], single_texture[i][1], diff)
				) {
				sphere_elements[sphere_numElements] = (short)j;
				sphere_numElements++;
				break;
			}
		}


		if ((j == sphere_numVertices) && (sphere_numVertices < sphere_maxElements) && (sphere_numElements < sphere_maxElements))
		{

			sphere_verts[sphere_numVertices * 3] = single_vertex[i][0];
			sphere_verts[(sphere_numVertices * 3) + 1] = single_vertex[i][1];
			sphere_verts[(sphere_numVertices * 3) + 2] = single_vertex[i][2];

			sphere_norms[sphere_numVertices * 3] = single_normal[i][0];
			sphere_norms[(sphere_numVertices * 3) + 1] = single_normal[i][1];
			sphere_norms[(sphere_numVertices * 3) + 2] = single_normal[i][2];

			sphere_texCoords[sphere_numVertices * 2] = single_texture[i][0];
			sphere_texCoords[(sphere_numVertices * 2) + 1] = single_texture[i][1];

			sphere_elements[sphere_numElements] = (short)sphere_numVertices;
			sphere_numElements++;
			sphere_numVertices++;

		}

	}
	//
}

void sphereMesh_prepareToDraw()
{
	// vao
	glGenVertexArrays(1, &sphere_vao);
	glBindVertexArray(sphere_vao);

	// vbo for position
	glGenBuffers(1, &sphere_vbo_position);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo_position);
	glBufferData(GL_ARRAY_BUFFER, (sphere_maxElements * 3 * sizeof(GLfloat) / 3), sphere_verts, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// vbo for normals
	glGenBuffers(1, &sphere_vbo_normal);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo_normal);
	glBufferData(GL_ARRAY_BUFFER, (sphere_maxElements * 3 * sizeof(GLfloat) / 3), sphere_norms, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// vbo for texture
	glGenBuffers(1, &sphere_vbo_texture);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo_texture);
	glBufferData(GL_ARRAY_BUFFER, (sphere_maxElements * 2 * sizeof(GLfloat) / 3), sphere_texCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(VDG_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(VDG_ATTRIBUTE_TEXTURE0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// vbo for index
	glGenBuffers(1, &sphere_vbo_index);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_vbo_index);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (sphere_maxElements * 3 * sizeof(unsigned short) / 3), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	cleanupSphereMeshData();

}

void drawSphere()
{
	glBindVertexArray(sphere_vao);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_vbo_index);
	glDrawElements(GL_TRIANGLES, sphere_numElements, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);

}

void makeSphere(int fRadius, int iSlices, int iStacks)
{
	GLfloat drho = (GLfloat)SPHERE_VDG_PI / (GLfloat)iStacks;
	GLfloat dtheta = 2.0f * (GLfloat)SPHERE_VDG_PI / (GLfloat)iSlices;
	GLfloat ds = 1.0f / (GLfloat)iSlices;
	GLfloat dt = 1.0f / (GLfloat)iStacks;
	GLfloat t = 1.0f;
	GLfloat s = 0.0f;
	int i = 0;
	int j = 0;

	spehereMesh_allocate(iSlices * iStacks * 6);

	for (i = 0; i < iStacks; i++)
	{

		GLfloat rho = (GLfloat)(i * drho);
		GLfloat srho = (GLfloat)(sin(rho));
		GLfloat crho = (GLfloat)(cos(rho));
		GLfloat srhodrho = (GLfloat)(sin(rho + drho));
		GLfloat crhodrho = (GLfloat)(cos(rho + drho));

		s = 0.0f;

		// initialization of three 2-D arrays, two are 4 x 3 and one is 4 x 2
		GLfloat **vertex = (GLfloat **)malloc(sizeof(GLfloat *) * 4); // 4 rows
		for (int a = 0; a < 4; a++)
			vertex[a] = (GLfloat *)malloc(sizeof(GLfloat) * 3); // 3 columns

		GLfloat **normal = (GLfloat **)malloc(sizeof(GLfloat *) * 4); // 4 rows
		for (int a = 0; a < 4; a++)
			normal[a] = (GLfloat *)malloc(sizeof(GLfloat) * 3); // 3 columns

		GLfloat **texture = (GLfloat **)malloc(sizeof(GLfloat *) * 4); // 4 rows
		for (int a = 0; a < 4; a++)
			texture[a] = (GLfloat *)malloc(sizeof(GLfloat) * 2); // 2 columns

		for (j = 0; j < iSlices; j++)
		{
			GLfloat thetha = (j == iSlices) ? 0.0f : j * dtheta;
			GLfloat sthetha = (GLfloat)(-sin(thetha));
			GLfloat cthetha = (GLfloat)(cos(thetha));

			GLfloat x = sthetha * srho;
			GLfloat y = cthetha * srho;
			GLfloat z = crho;


			texture[0][0] = s;
			texture[0][1] = t;

			normal[0][0] = x;
			normal[0][1] = y;
			normal[0][2] = z;

			vertex[0][0] = x * fRadius;
			vertex[0][1] = y * fRadius;
			vertex[0][2] = z * fRadius;


			x = sthetha * srhodrho;
			y = cthetha * srhodrho;
			z = crhodrho;

			texture[1][0] = s;
			texture[1][1] = t - dt;

			normal[1][0] = x;
			normal[1][1] = y;
			normal[1][2] = z;

			vertex[1][0] = x * fRadius;
			vertex[1][1] = y * fRadius;
			vertex[1][2] = z * fRadius;


			thetha = ((j + 1) == iSlices) ? 0.0f : (j + 1) * dtheta;
			sthetha = (GLfloat)(-sin(thetha));
			cthetha = (GLfloat)(cos(thetha));


			x = sthetha * srho;
			y = cthetha * srho;
			z = crho;

			s += ds;

			texture[2][0] = s;
			texture[2][1] = t;

			normal[2][0] = x;
			normal[2][1] = y;
			normal[2][2] = z;

			vertex[2][0] = x * fRadius;
			vertex[2][1] = y * fRadius;
			vertex[2][2] = z * fRadius;


			x = sthetha * srhodrho;
			y = cthetha * srhodrho;
			z = crhodrho;

			texture[3][0] = s;
			texture[3][1] = t - dt;

			normal[3][0] = x;
			normal[3][1] = y;
			normal[3][2] = z;

			vertex[3][0] = x * fRadius;
			vertex[3][1] = y * fRadius;
			vertex[3][2] = z * fRadius;

			sphereMesh_AddTriangle(vertex, normal, texture);


			// Rearrange for next triangle
			vertex[0][0] = vertex[1][0];
			vertex[0][1] = vertex[1][1];
			vertex[0][2] = vertex[1][2];

			normal[0][0] = normal[1][0];
			normal[0][1] = normal[1][1];
			normal[0][2] = normal[1][2];

			texture[0][0] = texture[1][0];
			texture[0][1] = texture[1][1];


			vertex[1][0] = vertex[3][0];
			vertex[1][1] = vertex[3][1];
			vertex[1][2] = vertex[3][2];

			normal[1][0] = normal[3][0];
			normal[1][1] = normal[3][1];
			normal[1][2] = normal[3][2];

			texture[1][0] = texture[3][0];
			texture[1][1] = texture[3][1];

			sphereMesh_AddTriangle(vertex, normal, texture);
		}
		t -= dt;
	}
	sphereMesh_prepareToDraw();
}

// WinMain()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// function declarations
	void initialize(void);
	void uninitialize(void);
	void display(void);

	// variable declaration
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szClassName[] = TEXT("OpenGLPP");
	bool bDone = false;

	// code
	// create log file
	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Log File Can Not Be Created\nExitting ..."), TEXT("Error"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log File Is Successfully Opened.\n");
	}

	// initializing members of struct WNDCLASSEX
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szClassName;
	wndclass.lpszMenuName = NULL;

	// registering class
	RegisterClassEx(&wndclass);

	// create window
	hwnd = CreateWindow(szClassName,
		TEXT("OpenGL Programmable Pipeline Window"),
		WS_OVERLAPPEDWINDOW,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);

	ghwnd = hwnd;

	ShowWindow(hwnd, iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// initialize
	initialize();

	// message loop
	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = true;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			// rendring function
			display();

			if (gbActiveWindow == true)
			{
				if (gbEscapeKeyIsPressed == true) //Continuation to glutLeaveMainLoop();
					bDone = true;
			}
		}
	}

	uninitialize();

	return((int)msg.wParam);
}

// WndProc()
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function declarations
	void resize(int, int);
	void ToggleFullscreen(void);
	void uninitialize(void);

	// variable declarations
	static bool bIsAKeyPressed = false;
	static bool bIsLKeyPressed = false;

	// code
	switch (iMsg)
	{
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
			gbActiveWindow = true;
		else
			gbActiveWindow = false;
		break;
	case WM_ERASEBKGND:
		return(0);
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			if (gbEscapeKeyIsPressed == false)
			{
				gbEscapeKeyIsPressed = true;
			}
			break;
		case 0x46: // for 'f' or 'F'
			if (gbFullscreen == false)
			{
				ToggleFullscreen();
				gbFullscreen = true;
			}
			else
			{
				ToggleFullscreen();
				gbFullscreen = false;
			}
			break;
		case 0x4C: // for 'L' or 'l'
			if (bIsLKeyPressed == false)
			{
				gbLight = true;
				bIsLKeyPressed = true;
			}
			else
			{
				gbLight = false;
				bIsLKeyPressed = false;
			}
			break;
		default:
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_CLOSE:
		uninitialize();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void initialize(void)
{
	// function declarations
	void uninitialize(void);
	void resize(int, int);

	// variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	// code
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	// initialization of structure 'PIXELFORMATDESCRIPTOR'
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	ghdc = GetDC(ghwnd);

	// choose a pixel format which best matches with that of 'pfd'
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	// set the pixel format chosen above
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == false)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	// create OpenGL rendering context
	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	// make the rendering context created above as current n the current hdc
	if (wglMakeCurrent(ghdc, ghrc) == false)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	// GLEW Initialization Code For GLSL ( IMPORTANT : It Must Be Here. Means After Creating OpenGL Context But Before Using Any OpenGL Function )
	GLenum glew_error = glewInit();
	if (glew_error != GLEW_OK)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	// *** VERTEX SHADER ***
	// create shader
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	// provide source code to shader
	const GLchar *vertexShaderSourceCode =
		"#version 440 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec3 vNormal;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform vec4 u_light_position;" \
		"uniform int u_lighting_enabled;" \
		"out vec3 transformed_normals;" \
		"out vec3 light_direction;" \
		"out vec3 viewer_vector;" \
		"void main(void)" \
		"{" \
		"if(u_lighting_enabled==1)" \
		"{" \
		"vec4 eye_coordinates=u_view_matrix * u_model_matrix * vPosition;" \
		"transformed_normals=mat3(u_view_matrix * u_model_matrix) * vNormal;" \
		"light_direction = vec3(u_light_position) - eye_coordinates.xyz;" \
		"viewer_vector = -eye_coordinates.xyz;" \
		"}" \
		"gl_Position=u_projection_matrix * u_view_matrix * u_model_matrix * vPosition;" \
		"}";
	glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

	// compile shader
	glCompileShader(gVertexShaderObject);
	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	char *szInfoLog = NULL;
	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Vertex Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	// *** FRAGMENT SHADER ***
	// create shader
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	// provide source code to shader
	const GLchar *fragmentShaderSourceCode =
		"#version 440 core" \
		"\n" \
		"in vec3 transformed_normals;" \
		"in vec3 light_direction;" \
		"in vec3 viewer_vector;" \
		"out vec4 FragColor;" \
		"uniform vec3 u_La;" \
		"uniform vec3 u_Ld;" \
		"uniform vec3 u_Ls;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_material_shininess;" \
		"uniform int u_lighting_enabled;" \
		"void main(void)" \
		"{" \
		"vec3 phong_ads_color;" \
		"if(u_lighting_enabled==1)" \
		"{" \
		"vec3 normalized_transformed_normals=normalize(transformed_normals);" \
		"vec3 normalized_light_direction=normalize(light_direction);" \
		"vec3 normalized_viewer_vector=normalize(viewer_vector);" \
		"vec3 ambient = u_La * u_Ka;" \
		"float tn_dot_ld = max(dot(normalized_transformed_normals, normalized_light_direction),0.0);" \
		"vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;" \
		"vec3 reflection_vector = reflect(-normalized_light_direction, normalized_transformed_normals);" \
		"vec3 specular = u_Ls * u_Ks * pow(max(dot(reflection_vector, normalized_viewer_vector), 0.0), u_material_shininess);" \
		"phong_ads_color=ambient + diffuse + specular;" \
		"}" \
		"else" \
		"{" \
		"phong_ads_color = vec3(1.0, 1.0, 1.0);" \
		"}" \
		"FragColor = vec4(phong_ads_color, 1.0);" \
		"}";

	glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

	// compile shader
	glCompileShader(gFragmentShaderObject);
	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Fragment Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	// *** SHADER PROGRAM ***
	// create
	gShaderProgramObject = glCreateProgram();

	// attach vertex shader to shader program
	glAttachShader(gShaderProgramObject, gVertexShaderObject);

	// attach fragment shader to shader program
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	// pre-link binding of shader program object with vertex shader position attribute
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_VERTEX, "vPosition");
	glBindAttribLocation(gShaderProgramObject, VDG_ATTRIBUTE_NORMAL, "vNormal");

	// link shader
	glLinkProgram(gShaderProgramObject);
	GLint iShaderProgramLinkStatus = 0;
	glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if (iShaderProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Shader Program Link Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}

	// get uniform locations
	model_matrix_uniform = glGetUniformLocation(gShaderProgramObject, "u_model_matrix");
	view_matrix_uniform = glGetUniformLocation(gShaderProgramObject, "u_view_matrix");
	projection_matrix_uniform = glGetUniformLocation(gShaderProgramObject, "u_projection_matrix");

	makeSphere(2.0f, 30, 30);

	// L/l key is pressed or not
	L_KeyPressed_uniform = glGetUniformLocation(gShaderProgramObject, "u_lighting_enabled");

	// ambient color intensity of light
	La_uniform = glGetUniformLocation(gShaderProgramObject, "u_La");
	// diffuse color intensity of light
	Ld_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ld");
	// specular color intensity of light
	Ls_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ls");
	// position of light
	light_position_uniform = glGetUniformLocation(gShaderProgramObject, "u_light_position");;

	// ambient reflective color intensity of material
	Ka_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ka");
	// diffuse reflective color intensity of material
	Kd_uniform = glGetUniformLocation(gShaderProgramObject, "u_Kd");
	// specular reflective color intensity of material
	Ks_uniform = glGetUniformLocation(gShaderProgramObject, "u_Ks");
	// shininess of material ( value is conventionally between 1 to 200 )
	material_shininess_uniform = glGetUniformLocation(gShaderProgramObject, "u_material_shininess");;

	/*
	
	getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);

	gNumVertices = getNumberOfSphereVertices();
	gNumElements = getNumberOfSphereElements();

	// vao
	glGenVertexArrays(1, &gVao_sphere);
	glBindVertexArray(gVao_sphere);

	// position vbo
	glGenBuffers(1, &gVbo_sphere_position);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_vertices), sphere_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(VDG_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(VDG_ATTRIBUTE_VERTEX);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// normal vbo
	glGenBuffers(1, &gVbo_sphere_normal);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_sphere_normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_normals), sphere_normals, GL_STATIC_DRAW);

	glVertexAttribPointer(VDG_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(VDG_ATTRIBUTE_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// element vbo
	glGenBuffers(1, &gVbo_sphere_element);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sphere_elements), sphere_elements, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	*/

	glShadeModel(GL_SMOOTH);
	// set-up depth buffer
	glClearDepth(1.0f);
	// enable depth testing
	glEnable(GL_DEPTH_TEST);
	// depth test to do
	glDepthFunc(GL_LEQUAL);
	// set really nice percpective calculations ?
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	// We will always cull back faces for better performance
	glEnable(GL_CULL_FACE);

	// set background color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // black

										  // set perspective matrix to identitu matrix
	gPerspectiveProjectionMatrix = mat4::identity();

	gbLight = false;

	// resize
	resize(WIN_WIDTH, WIN_HEIGHT);
}

void display(void)
{
	//code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// start using OpenGL program object
	glUseProgram(gShaderProgramObject);

	if (gbLight == true)
	{
		// set 'u_lighting_enabled' uniform
		glUniform1i(L_KeyPressed_uniform, 1);

		// setting light's properties
		glUniform3fv(La_uniform, 1, lightAmbient);
		glUniform3fv(Ld_uniform, 1, lightDiffuse);
		glUniform3fv(Ls_uniform, 1, lightSpecular);
		glUniform4fv(light_position_uniform, 1, lightPosition);

		// setting material's properties
		glUniform3fv(Ka_uniform, 1, material_ambient);
		glUniform3fv(Kd_uniform, 1, material_diffuse);
		glUniform3fv(Ks_uniform, 1, material_specular);
		glUniform1f(material_shininess_uniform, material_shininess);
	}
	else
	{
		// set 'u_lighting_enabled' uniform
		glUniform1i(L_KeyPressed_uniform, 0);
	}

	// OpenGL Drawing
	
	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();
	modelMatrix = translate(-2.5f, 0.0f, -10.0f);
	modelMatrix = modelMatrix*scale(0.3f,0.3f,0.3f);
	glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	drawSphere();

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	modelMatrix = translate(0.0f, 0.0f, -10.0f);
	modelMatrix = modelMatrix*scale(0.3f, 0.3f, 0.3f);
	glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	drawSphere();

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	modelMatrix = translate(2.5f, 0.0f, -10.0f);
	modelMatrix = modelMatrix*scale(0.3f, 0.3f, 0.3f);
	glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	drawSphere();

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	modelMatrix = translate(0.0f, 2.0f, -10.0f);
	modelMatrix = modelMatrix*scale(0.3f, 0.3f, 0.3f);
	glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	drawSphere();

	modelMatrix = mat4::identity();
	viewMatrix = mat4::identity();
	modelMatrix = translate(0.0f, -2.0f, -10.0f);
	modelMatrix = modelMatrix*scale(0.3f, 0.3f, 0.3f);
	glUniformMatrix4fv(model_matrix_uniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(view_matrix_uniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(projection_matrix_uniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	drawSphere();


	/*
	// *** bind vao ***
	glBindVertexArray(gVao_sphere);

	// *** draw, either by glDrawTriangles() or glDrawArrays() or glDrawElements()
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gVbo_sphere_element);
	glDrawElements(GL_TRIANGLES, gNumElements, GL_UNSIGNED_SHORT, 0);

	// *** unbind vao ***
	glBindVertexArray(0);
	*/

	/*
	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();

	modelViewMatrix = vmath::translate(-2.5f, 0.0f, -6.0f);
	modelViewMatrix = modelViewMatrix *scale(0.2f, 0.2f, 0.2f);
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix*modelViewMatrix;
	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	drawSphere();

	modelViewMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
	modelViewMatrix = modelViewMatrix *scale(0.2f, 0.2f, 0.2f);
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix*modelViewMatrix;
	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	drawSphere();

	modelViewMatrix = vmath::translate(2.5f, 0.0f, -6.0f);
	modelViewMatrix = modelViewMatrix *scale(0.2f, 0.2f, 0.2f);
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix*modelViewMatrix;
	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	drawSphere();

	modelViewMatrix = vmath::translate(0.0f, 2.0f, -6.0f);
	modelViewMatrix = modelViewMatrix *scale(0.2f, 0.2f, 0.2f);
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix*modelViewMatrix;
	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	drawSphere();

	modelViewMatrix = vmath::translate(0.0f, -2.0f, -6.0f);
	modelViewMatrix = modelViewMatrix *scale(0.2f, 0.2f, 0.2f);
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix*modelViewMatrix;
	glUniformMatrix4fv(gMVPUniform, 1, GL_FALSE, modelViewProjectionMatrix);
	drawSphere();
	*/

	// stop using OpenGL program object
	glUseProgram(0);

	SwapBuffers(ghdc);
}

void ToggleFullscreen(void)
{
	MONITORINFO mi;

	if (gbFullscreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
	}

	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}
}

void resize(int width, int height)
{
	if (height == 0)
	{
		height = 1;
	}
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	gPerspectiveProjectionMatrix = perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void uninitialize(void)
{
	if (gbFullscreen == true)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
	}

	/*
	if (gVao_sphere)
	{
		glDeleteVertexArrays(1, &gVao_sphere);
		gVao_sphere = 0;
	}

	if (gVbo_sphere_position)
	{
		glDeleteBuffers(1, &gVbo_sphere_position);
		gVbo_sphere_position = 0;
	}

	if (gVbo_sphere_normal)
	{
		glDeleteBuffers(1, &gVbo_sphere_normal);
		gVbo_sphere_normal = 0;
	}

	if (gVbo_sphere_element)
	{
		glDeleteBuffers(1, &gVbo_sphere_element);
		gVbo_sphere_element = 0;
	}
	*/

	cleanupSphereBuffers();
	cleanupSphereMeshData();

	glDetachShader(gShaderProgramObject, gVertexShaderObject);
	
	glDetachShader(gShaderProgramObject, gFragmentShaderObject);

	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject = 0;
	
	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject = 0;

	glDeleteProgram(gShaderProgramObject);
	gShaderProgramObject = 0;

	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(ghrc);
	ghrc = NULL;

	ReleaseDC(ghwnd, ghdc);
	ghdc = NULL;

	if (gpFile)
	{
		fprintf(gpFile, "Log File Is Successfully Closed.\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
