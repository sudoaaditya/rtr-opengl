#include <Windows.h>
#include <gl\glew.h>
#include <gl/GL.h>
#include <stdio.h> 
#include "vmath.h"
#include "RCheader.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")

#define BUFFER_SIZE 256

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define BUFFER_SIZE 256	

enum
{
	AC_ATTRIBUTE_VERTEX = 0,
	AC_ATTRIBUTE_COLOR,
	AC_ATTRIBUTE_NORMAL,
	AC_ATTRIBUTE_TEXTURE0,
};

using namespace vmath;

HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbActiveWindow = false;
bool gbEscapeKeyIsPressed = false;
bool gbFullScreen = false;

GLuint ac_gVertexShaderObject;
GLuint ac_gFragmentShaderObject;
GLuint ac_gShaderProgramObject;

FILE *gpFile = NULL;

FILE *gpExtensionData = NULL;

#define SPHERE_VDG_PI  3.14159265358979323846f

//Cube Variables
GLuint gVao_cube;
GLuint gVbo_cube;
GLuint gMVPUniform;
mat4 gPerspectiveProjectionMatrix;
GLuint gModelMatrixUniform, gViewMatrixUniform, gProjectionMatrixUniform;
static float angle_pyramid = 0.0f;
static float angle_cube = 0.0f;

//Texture
GLuint gTexture_smapler_uniform;
GLuint gTexture_Marble;

//Light Variables
GLuint La_uniform;
GLuint Ld_Uniform;
GLuint Ls_Uniform;
GLuint Ka_Uniform;
GLuint Kd_Uniform;
GLuint Ks_Uniform;
GLuint Material_shininess_uniform;
GLuint gLightPositionUniform;
GLuint gLKeyPressedUniform;
bool bIsLkeyPressed;
bool gbLight;
GLfloat lightAmbient[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat lightDiffuse[] = { 1.0f,1.0f,0.8784f,1.0f };
GLfloat lightSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
//GLfloat lightPosition[] = { 2.0f,1.0f,1.0f,1.0f };
GLfloat lightPosition[] = { 0.0f,0.0f,0.0f,0.0f };
GLfloat material_Ambient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat material_Diffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat material_Specular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat material_shininess = 128.0f;

LRESULT CALLBACK AcCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

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
	glVertexAttribPointer(AC_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AC_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// vbo for normals
	glGenBuffers(1, &sphere_vbo_normal);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo_normal);
	glBufferData(GL_ARRAY_BUFFER, (sphere_maxElements * 3 * sizeof(GLfloat) / 3), sphere_norms, GL_STATIC_DRAW);
	glVertexAttribPointer(AC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AC_ATTRIBUTE_NORMAL);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	// vbo for texture
	glGenBuffers(1, &sphere_vbo_texture);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_vbo_texture);
	glBufferData(GL_ARRAY_BUFFER, (sphere_maxElements * 2 * sizeof(GLfloat) / 3), sphere_texCoords, GL_STATIC_DRAW);
	glVertexAttribPointer(AC_ATTRIBUTE_TEXTURE0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AC_ATTRIBUTE_TEXTURE0);
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


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	void initialize(void);
	void uninitialize(void);
	void update(void);
	void display(void);
	void resize(int, int);

	WNDCLASSEX ac;
	HWND hwnd;
	MSG msg;
	TCHAR szClassName[] = TEXT("Shree Ganesha : Interleaved using  1D Array");
	bool bDone = false;

	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Log File Can Not Be Created"), TEXT("Error"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log File Is Successfully Opened\n");
	}

	ac.cbSize = sizeof(WNDCLASSEX);
	ac.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	ac.cbClsExtra = 0;
	ac.cbWndExtra = 0;
	ac.hInstance = hInstance;
	ac.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	ac.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	ac.hCursor = LoadCursor(NULL, IDC_ARROW);
	ac.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	ac.lpfnWndProc = AcCallBack;
	ac.lpszClassName = szClassName;
	ac.lpszMenuName = NULL;

	RegisterClassEx(&ac);

	hwnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		szClassName,
		TEXT("Shree Ganesha"),
		WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	ghwnd = hwnd;

	initialize();

	ShowWindow(hwnd, SW_SHOWNORMAL);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == true)
			{
				if (gbEscapeKeyIsPressed == true)
				{
					bDone = true;
				}
				update();
				display();
			}
		}
	}
	uninitialize();
	return ((int)msg.wParam);
}

void initialize(void)
{
	void resize(GLint width, GLint height);
	int LoadGLTextures(GLuint *texture, TCHAR imageResourceId[]);

	//OpenGL calls
	PIXELFORMATDESCRIPTOR pfd;
	GLint iPixelFormatIndex;

	//For getting extensions
	GLint num;

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cBlueBits = 8;
	pfd.cGreenBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	ghdc = GetDC(ghwnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == false)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc == NULL)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	GLenum glew_error = glewInit();
	if (glew_error != GLEW_OK)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	//Extensions supported by OpenGL
	if (fopen_s(&gpExtensionData, "Extensions.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Extensions.txt File Can Not Be Created"), TEXT("Error"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
		exit(0);
	}
	else
	{
		fprintf_s(gpExtensionData, "Extension.txt File Is Successfully Opened\n");
	}

	fprintf_s(gpExtensionData, "This is Assignment to find extension supported by my graphic card\n");

	glGetIntegerv(GL_NUM_EXTENSIONS, &num);

	for (int i = 0; i<num; i++)
	{
		fprintf_s(gpExtensionData, "%d : %s\n", i + 1, (char*)glGetStringi(GL_EXTENSIONS, i));
	}

	fprintf_s(gpExtensionData, "Extension.txt File Is Successfully Closed\n");
	fclose(gpExtensionData);
	gpExtensionData = NULL;


	//------------------------------------------------------------------------------

	//Addding code for vertex and fragment shader here
	ac_gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	const GLchar *vertexShaderSourceCode =
		"#version 440 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec2 vTexture0_Coord;" \
		"in vec4 vColor;" \
		"in vec3 vNormal;" \
		"out vec4 out_color;"\
		"out vec2 out_texture0_coord;" \
		"uniform mat4 u_model_matrix;" \
		"uniform mat4 u_view_matrix;" \
		"uniform mat4 u_projection_matrix;" \
		"uniform int u_LKeyPressed;" \
		"out vec3 transformed_normals;" \
		"out vec3 light_direction;"\
		"out vec3 viewer_vector;"\
		"uniform vec4 u_light_position;" \
		"void main (void)" \
		"{" \
		"if(u_LKeyPressed == 1)" \
		"{" \
		"vec4 eyeCoordinates =u_view_matrix * u_model_matrix * vPosition;" \
		"transformed_normals = mat3(u_view_matrix * u_model_matrix)*vNormal;" \
		"light_direction = vec3(u_light_position) - eyeCoordinates.xyz;" \
		"viewer_vector = -eyeCoordinates.xyz;" \
		"}" \
		"out_texture0_coord = vTexture0_Coord;" \
		"out_color = vColor;" \
		"gl_Position = u_projection_matrix * u_view_matrix * u_model_matrix *vPosition;" \
		"}";


	glShaderSource(ac_gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

	glCompileShader(ac_gVertexShaderObject);
	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	char * szInfoLog = NULL;
	glGetShaderiv(ac_gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(ac_gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(ac_gVertexShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf_s(gpFile, "Vertex Shader Info Log: %s\n", szInfoLog);
				free(szInfoLog);
				exit(0);
			}
		}
	}

	ac_gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	/*
	const GLchar * fragmentShaderSourceCode =
		"#version 440 core" \
		"\n" \
		"in vec2 out_texture0_coord;"
		"in vec3 transformed_normals;"\
		"in vec3 light_direction;"\
		"in vec3 viewer_vector;"\
		"uniform vec3 u_La;" \
		"uniform vec3 u_Ld;" \
		"uniform vec3 u_Ls;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_material_shininess;" \
		"in vec4 out_color;"
		"uniform int u_LKeyPressed;"\
		"uniform sampler2D u_texture0_sampler;" \
		"out vec4 FragColor;" \
		"void main (void)" \
		"{" \
		"vec4 color;" \
		"vec4 phong_ads_color;" \
		"if(u_LKeyPressed == 1)" \
		"{" \
		"vec3 normalized_transformed_normals = normalize(transformed_normals);"\
		"vec3 normalized_light_direction = normalize(light_direction);"\
		"vec3 normalized_viewer_vector = normalize(viewer_vector);"\
		"vec3 ambient = u_La * u_Ka;"\
		"float tn_dot_ld = max(dot(normalized_transformed_normals , normalized_light_direction),0.0);"\
		"vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;"\
		"vec3 reflection_vector = reflect(-normalized_light_direction,normalized_transformed_normals);"\
		"vec3 specular = u_Ls * u_Ks *pow(max(dot(reflection_vector,normalized_viewer_vector),0.0),u_material_shininess);"\
		"phong_ads_color = vec4(ambient + diffuse + specular,1.0);"\
		"color = phong_ads_color * texture(u_texture0_sampler,out_texture0_coord) * out_color;" \
		"}" \
		"else" \
		"{" \
		"color = texture(u_texture0_sampler,out_texture0_coord) + out_color;" \
		"}" \
		"FragColor = color;" \
		"}";
	*/

	const GLchar * fragmentShaderSourceCode =
		"#version 440 core" \
		"\n" \
		"in vec2 out_texture0_coord;"
		"in vec3 transformed_normals;"\
		"in vec3 light_direction;"\
		"in vec3 viewer_vector;"\
		"uniform vec3 u_La;" \
		"uniform vec3 u_Ld;" \
		"uniform vec3 u_Ls;" \
		"uniform vec3 u_Ka;" \
		"uniform vec3 u_Kd;" \
		"uniform vec3 u_Ks;" \
		"uniform float u_material_shininess;" \
		"in vec4 out_color;"
		"uniform int u_LKeyPressed;"\
		"uniform sampler2D u_texture0_sampler;" \
		"out vec4 FragColor;" \
		"void main (void)" \
		"{" \
		"vec4 color;" \
		"vec4 phong_ads_color;" \
		"if(u_LKeyPressed == 1)" \
		"{" \
		"vec3 normalized_transformed_normals = normalize(transformed_normals);"\
		"vec3 normalized_light_direction = normalize(light_direction);"\
		"vec3 normalized_viewer_vector = normalize(viewer_vector);"\
		"vec3 ambient = u_La * u_Ka;"\
		"float tn_dot_ld = max(dot(normalized_transformed_normals , normalized_light_direction),0.0);"\
		"vec3 diffuse = u_Ld * u_Kd * tn_dot_ld;"\
		"vec3 reflection_vector = reflect(-normalized_light_direction,normalized_transformed_normals);"\
		"vec3 specular = u_Ls * u_Ks *pow(max(dot(reflection_vector,normalized_viewer_vector),0.0),u_material_shininess);"\
		"phong_ads_color = vec4(ambient + diffuse,1.0);"\
		"color = phong_ads_color * texture(u_texture0_sampler,out_texture0_coord) * out_color;" \
		"}" \
		"else" \
		"{" \
		"color = texture(u_texture0_sampler,out_texture0_coord) + out_color;" \
		"}" \
		"FragColor = color;" \
		"}";

	glShaderSource(ac_gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

	glCompileShader(ac_gFragmentShaderObject);

	glShaderSource(ac_gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);
	glCompileShader(ac_gFragmentShaderObject);
	glGetShaderiv(ac_gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(ac_gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(ac_gFragmentShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf_s(gpFile, "Fragment Shader Info Log: %s\n", szInfoLog);
				free(szInfoLog);
				exit(0);
			}
		}
	}

	//Main Shader Program starts here
	ac_gShaderProgramObject = glCreateProgram();
	glAttachShader(ac_gShaderProgramObject, ac_gVertexShaderObject);
	glAttachShader(ac_gShaderProgramObject, ac_gFragmentShaderObject);

	//bind in variables before linking
	glBindAttribLocation(ac_gShaderProgramObject, AC_ATTRIBUTE_VERTEX, "vPosition");
	glBindAttribLocation(ac_gShaderProgramObject, AC_ATTRIBUTE_COLOR, "vColor");
	glBindAttribLocation(ac_gShaderProgramObject, AC_ATTRIBUTE_NORMAL, "vNormal");
	glBindAttribLocation(ac_gShaderProgramObject, AC_ATTRIBUTE_TEXTURE0, "vTexture0_Coord");

	glLinkProgram(ac_gShaderProgramObject);
	GLint iShaderProgramLinkStatus = 0;
	glGetProgramiv(ac_gShaderProgramObject, GL_LINK_STATUS, &iShaderProgramLinkStatus);
	if (iShaderProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(ac_gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char *)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(ac_gShaderProgramObject, iInfoLogLength, &written, szInfoLog);
				fprintf_s(gpFile, "Program Link Log: %s \n", szInfoLog);
				free(szInfoLog);
				exit(0);
			}
		}
	}

	//get uniform's address
	gModelMatrixUniform = glGetUniformLocation(ac_gShaderProgramObject, "u_model_matrix");
	gViewMatrixUniform = glGetUniformLocation(ac_gShaderProgramObject, "u_view_matrix");
	gProjectionMatrixUniform = glGetUniformLocation(ac_gShaderProgramObject, "u_projection_matrix");

	//Light Uniform
	Ka_Uniform = glGetUniformLocation(ac_gShaderProgramObject, "u_Ka");
	Kd_Uniform = glGetUniformLocation(ac_gShaderProgramObject, "u_Kd");
	Ks_Uniform = glGetUniformLocation(ac_gShaderProgramObject, "u_Ks");
	Material_shininess_uniform = glGetUniformLocation(ac_gShaderProgramObject, "u_material_shininess");
	gLightPositionUniform = glGetUniformLocation(ac_gShaderProgramObject, "u_light_position");
	Ld_Uniform = glGetUniformLocation(ac_gShaderProgramObject, "u_Ld");
	La_uniform = glGetUniformLocation(ac_gShaderProgramObject, "u_La");
	La_uniform = glGetUniformLocation(ac_gShaderProgramObject, "u_Ls");
	gLKeyPressedUniform = glGetUniformLocation(ac_gShaderProgramObject, "u_LKeyPressed");

	//Texture Uniform
	gTexture_smapler_uniform = glGetUniformLocation(ac_gShaderProgramObject, "u_texture0_sampler");

	//------------------------------------------------------------------------------

	makeSphere(2.0f, 30, 30);
	

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	//glEnable(GL_CULL_FACE);

	LoadGLTextures(&gTexture_Marble, MAKEINTRESOURCE(IDB_MARBLE_BMP));

	glEnable(GL_TEXTURE_2D);

	//Defined Background color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	gPerspectiveProjectionMatrix = mat4::identity();
	resize(WIN_WIDTH, WIN_HEIGHT);
}

int LoadGLTextures(GLuint *texture, TCHAR imageResourceId[])
{
	HBITMAP hBitmap;
	BITMAP bmp;
	int iStatus = FALSE;
	HRESULT hr;
	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL),
		imageResourceId,
		IMAGE_BITMAP,
		0,
		0,
		LR_CREATEDIBSECTION);

	hr = HRESULT_FROM_WIN32(GetLastError());


	if (hBitmap)
	{

		iStatus = TRUE;
		GetObject(hBitmap, sizeof(bmp), &bmp);

		glGenTextures(1, texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

		glBindTexture(GL_TEXTURE_2D, *texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp.bmWidth, bmp.bmHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, bmp.bmBits);
		glGenerateMipmap(GL_TEXTURE_2D);
		DeleteObject(hBitmap);
	}

	return iStatus;
}


void uninitialize(void)
{
	if (gbFullScreen == true)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);

	}

	if (gVao_cube)
	{
		glDeleteVertexArrays(1, &gVao_cube);
		gVao_cube = 0;
	}

	if (gVbo_cube)
	{
		glDeleteBuffers(1, &gVbo_cube);
		gVbo_cube = 0;
	}

	if (gTexture_Marble)
	{
		glDeleteTextures(1, &gTexture_Marble);
		gTexture_Marble = 0;
	}

	glDetachShader(ac_gShaderProgramObject, ac_gVertexShaderObject);

	glDetachShader(ac_gShaderProgramObject, ac_gFragmentShaderObject);

	glDeleteShader(ac_gVertexShaderObject);
	ac_gVertexShaderObject = 0;

	glDeleteShader(ac_gFragmentShaderObject);
	ac_gFragmentShaderObject = 0;

	glDeleteProgram(ac_gShaderProgramObject);
	ac_gShaderProgramObject = 0;

	glUseProgram(0);

	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(ghrc);
	ghrc = NULL;

	ReleaseDC(ghwnd, ghdc);
	ghdc = NULL;

	if (gpFile)
	{
		fprintf(gpFile, "\nLog file is Successfully closed");
		fclose(gpFile);
		gpFile = NULL;
	}

	DestroyWindow(ghwnd);
	ghwnd = NULL;
}

void resize(GLint width, GLint height)
{
	if (height == 0)
	{
		height = 1;
	}
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	
	gPerspectiveProjectionMatrix = perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void ToggleFullScreen(void)
{
	MONITORINFO mi;

	if (gbFullScreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle&WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle&~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}
}

void update(void)
{
	angle_cube = angle_cube + 0.5f;
	if (angle_cube >= 360.0f)
	{
		angle_cube -= 360.0f;
	}
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(ac_gShaderProgramObject);

	//Check for light switch
	if (gbLight == true)
	{
		glUniform1i(gLKeyPressedUniform, 1);

		//light properties
		glUniform3fv(La_uniform, 1, lightAmbient);
		glUniform3fv(Ld_Uniform, 1, lightDiffuse);
		glUniform3fv(Ls_Uniform, 1, lightSpecular);
		glUniform4fv(gLightPositionUniform, 1, lightPosition);

		glUniform3fv(Ka_Uniform, 1, material_Ambient);
		glUniform3fv(Kd_Uniform, 1, material_Diffuse);
		glUniform3fv(Ks_Uniform, 1, material_Specular);
		glUniform1f(Material_shininess_uniform, material_shininess);
	}
	else
	{
		glUniform1i(gLKeyPressedUniform, 0);
	}

	mat4 modelMatrix = mat4::identity();
	mat4 viewMatrix = mat4::identity();
	mat4 savedMatrix = mat4::identity();
	mat4 scaleMatrix = mat4::identity();
	mat4 translationMatrix = mat4::identity();
	mat4 rotationMatrix = mat4::identity();
	translationMatrix = translate(0.0f, 0.0f, -10.0f);
	modelMatrix = modelMatrix*translationMatrix;
	rotationMatrix = rotate(284.0f, 1.0f, 0.0f, 0.0f);
	modelMatrix = modelMatrix * rotationMatrix;
	savedMatrix = modelMatrix;
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, modelMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);

	modelMatrix = savedMatrix;
	scaleMatrix = scale(0.5f, 0.5f, 0.5f);
	modelMatrix = modelMatrix*scaleMatrix;
	rotationMatrix = rotate(angle_cube, 0.0f, 0.0f, 1.0f);
	modelMatrix = modelMatrix * rotationMatrix;
	savedMatrix = modelMatrix;
	glUniformMatrix4fv(gModelMatrixUniform, 1, GL_FALSE, savedMatrix);
	glUniformMatrix4fv(gViewMatrixUniform, 1, GL_FALSE, viewMatrix);
	glUniformMatrix4fv(gProjectionMatrixUniform, 1, GL_FALSE, gPerspectiveProjectionMatrix);
	drawSphere();

	glVertexAttrib3f(AC_ATTRIBUTE_COLOR, 1.0f, 1.0f, 1.0f);
	
	glUseProgram(0);

	SwapBuffers(ghdc);
}

LRESULT CALLBACK AcCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void uninitialize(void);
	void resize(GLint width, GLint height);
	void ToggleFullScreen(void);

	switch (iMsg)
	{
	case WM_CREATE:
		MessageBox(NULL,TEXT("Game Window created"),TEXT("Message"),MB_OK);
		break;

	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
		{
			gbActiveWindow = true;
		}
		else
		{
			gbActiveWindow = false;
		}
		break;

	case WM_ERASEBKGND:
		return  0;

	case WM_LBUTTONDOWN:
		break;

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CLOSE:
		uninitialize();
		break;

	case WM_KEYDOWN:
		switch (LOWORD(wParam))
		{
		case VK_ESCAPE:
			if (gbEscapeKeyIsPressed == false)
			{
				gbEscapeKeyIsPressed = true;
			}
			break;

		case 0x46:
			if (gbFullScreen == false)
			{
				ToggleFullScreen();
				gbFullScreen = true;
			}
			else
			{
				ToggleFullScreen();
				gbFullScreen = false;
			}
			break;

		case 0x4C:  //L or l
			if (bIsLkeyPressed == false)
			{
				gbLight = true;
				bIsLkeyPressed = true;
			}
			else
			{
				gbLight = false;
				bIsLkeyPressed = false;
			}
			break;

		default:
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}