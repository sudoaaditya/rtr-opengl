#include<Windows.h>
#include<stdio.h>

#include<gl\glew.h>

#include<gl/GL.h>

#include "vmath.h"

#include "Variables.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Gdi32.lib")
#pragma comment(lib,"Kernel32.lib")

#define WIN_WIDTH 800
#define WIN_HEIGHT 600
using namespace vmath;

enum
{
	AC_ATTRIBUTE_VERTEX = 0,
	AC_ATTRIBUTE_COLOR,
	AC_ATTRIBUTE_NORMAL,
	AC_ATTRIBUTE_TEXTURE0
};

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
FILE *gpFile = NULL;

HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};

bool gbActiveWindow = false;
bool gbEscapeKeyIsPressed = false;
bool gbFullScreen = false;
bool bDone = false;

GLuint gVertexShaderObject = 0;
GLuint gFragmentShaderObject = 0;
GLuint gShaderProgramObject = 0;
GLuint gComputeShaderObject = 0;
GLuint gComputeProgramShaderObject = 0;

GLuint render_vao;
GLuint gVbo_Pyramid_Position;
GLfloat gAnglePyramid = 0.0f;

GLuint gMVPUniform;

mat4 gPerspectiveProjectionMatrix; //Change 1

static inline float random_float()
{
	float res;
	unsigned int tmp;
	static unsigned int seed = 0xFFFF0C59;

	seed *= 16807;

	tmp = seed ^ (seed >> 4) ^ (seed << 15);

	*((unsigned int*)& res) = (tmp >> 9) | 0x3F800000;

	return (res - 1.0f);
}

static vmath::vec3 random_vector(float minmag = 0.0f, float maxmag = 1.0f)
{
	vmath::vec3 randomvec(random_float() * 2.0f - 1.0f, random_float() * 2.0f - 1.0f, random_float() * 2.0f - 1.0f);
	randomvec = normalize(randomvec);
	randomvec *= (random_float() * (maxmag - minmag) + minmag);

	return randomvec;
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int iCmdShow)
{
	void initialize(void);
	void update(void);
	void display(void);
	void uninitialize(void);

	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;

	if (fopen_s(&gpFile,"Log.txt","w")!=0)
	{
		MessageBox(NULL,TEXT("Log.txt cannot be created"),TEXT("Error"),MB_OK|MB_TOPMOST|MB_ICONSTOP);
		exit(0);
	}
	else
	{
		fprintf(gpFile,"Log.txt is sucessfully opened\n");
	}
	
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
	wndclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = TEXT("3D-Rotation");
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

	RegisterClassEx(&wndclass);

	hwnd=CreateWindowEx(WS_EX_APPWINDOW, TEXT("3D-Rotation") , TEXT("Shree Ganesha : 3D - Rotation"),WS_OVERLAPPEDWINDOW,100,100,WIN_WIDTH,WIN_HEIGHT,NULL,NULL,hInstance,NULL);

	ghwnd = hwnd;

	ShowWindow(hwnd,iCmdShow);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	initialize();

	while (bDone==false)
	{
		if (PeekMessage(&msg,hwnd,0,0,PM_REMOVE))
		{
			if (msg.message==WM_QUIT)
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
			update();
			display();

			if (gbActiveWindow==true)
			{
				if (gbEscapeKeyIsPressed==true)
				{
					bDone = true;
				}
			}
		}
	}

	uninitialize();
	return ((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void resize(int width, int height);
	void ToggleFullScreen(void);
	void uninitialize(void);

	static WORD xMouse = NULL;
	static WORD yMouse = NULL;

	switch (iMsg)
	{
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
		return 0;
		break;

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

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
void initialize(void)
{
	void uninitialize(void);
	void resize(int width, int height);
	const char* PassShaderinString(const char* fileName);

	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	memset(&pfd,0,sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	pfd.cDepthBits = 32;

	ghdc = GetDC(ghwnd);

	iPixelFormatIndex = ChoosePixelFormat(ghdc,&pfd);
	if (iPixelFormatIndex==0)
	{
		ReleaseDC(ghwnd,ghdc);
		ghdc = NULL;
	}

	if (SetPixelFormat(ghdc,iPixelFormatIndex,&pfd)==false)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	ghrc = wglCreateContext(ghdc);
	if (ghrc==NULL)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (wglMakeCurrent(ghdc,ghrc)==false)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd,ghdc);
		ghdc = NULL;
	}

	GLenum glew_error = glewInit();
	if (glew_error!=GLEW_OK)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd,ghdc);
		ghdc = NULL;
	}

	gComputeShaderObject = glCreateShader(GL_COMPUTE_SHADER);
	const GLchar* computeShaderSourceCode = PassShaderinString("ComputeShader.txt");
	glShaderSource(gComputeShaderObject, 1, (const GLchar * *)& computeShaderSourceCode, NULL);
	glCompileShader(gComputeShaderObject);

	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	char* szInfoLog = NULL;

	glGetShaderiv(gComputeShaderObject, GL_COMPILE_STATUS, &iShaderCompiledStatus);
	if (iShaderCompiledStatus == GL_FALSE)
	{
		glGetShaderiv(gComputeShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gComputeShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Compute Shader Compilation Log : %s\n", szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	else
	{
		fprintf(gpFile, "Compute Shader Compilation Log : No Errors\n");
	}

	gComputeProgramShaderObject = glCreateProgram();
	glAttachShader(gComputeProgramShaderObject, gComputeShaderObject);

	glLinkProgram(gComputeProgramShaderObject);

	GLint iShaderLinkProgramLinkStatus = 0;
	glGetProgramiv(gComputeProgramShaderObject, GL_LINK_STATUS, &iShaderLinkProgramLinkStatus);
	if (iShaderLinkProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gComputeProgramShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLength);
		if (iInfoLogLength > 0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gComputeProgramShaderObject, iInfoLogLength, &written, szInfoLog);
				fprintf(gpFile, "Compute Shader program Link Log : %s\n", szInfoLog);
				free(szInfoLog);
				szInfoLog = NULL;
				uninitialize();
				exit(0);
			}
		}
	}
	else
	{
		fprintf(gpFile, "Compute Shader program Link Log : No Errors\n");
	}

	dt_location = glGetUniformLocation(gComputeProgramShaderObject, "dt");

	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);
	/*
	const GLchar *vertexShaderSourceCode =
		"#version 440 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec4 vColor;" \
		"out vec4 out_color;" \
		"uniform mat4 u_mvp_matrix;" \
		"void main(void)" \
		"{" \
		"gl_Position = u_mvp_matrix * vPosition;" \
		"out_color=vColor;"
		"}";
	*/
	const GLchar* vertexShaderSourceCode = PassShaderinString("VertexShader.txt");

	glShaderSource(gVertexShaderObject,1, (const GLchar **)&vertexShaderSourceCode,NULL);
	
	glCompileShader(gVertexShaderObject);

	glGetShaderiv(gVertexShaderObject,GL_COMPILE_STATUS,&iShaderCompiledStatus);
	if (iShaderCompiledStatus==GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog!=NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject,iInfoLogLength,&written,szInfoLog);
				fprintf(gpFile,"Vertex Shader Compilation Log : %s\n",szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	else
	{
		fprintf(gpFile, "Vertex Shader Compilation Log : No Errors\n");
	}

	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);
	/*
	const GLchar *fragmentShaderSourceCode = 
		"#version 440 core" \
		"\n" \
		"in vec4 out_color;" \
		"out vec4 FragColor;" \
		"void main(void)" \
		"{" \
		"FragColor = out_color;" \
		"}";
	*/
	const GLchar* fragmentShaderSourceCode = PassShaderinString("FragmentShader.txt");
	glShaderSource(gFragmentShaderObject,1,(const GLchar**)&fragmentShaderSourceCode,NULL);

	glCompileShader(gFragmentShaderObject);

	glGetShaderiv(gFragmentShaderObject,GL_COMPILE_STATUS,&iShaderCompiledStatus);
	if (iShaderCompiledStatus==GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog!=NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject,iInfoLogLength,&written,szInfoLog);
				fprintf(gpFile,"Fragment Shader Compilation Log : %s\n",szInfoLog);
				free(szInfoLog);
				szInfoLog = NULL;
				uninitialize();
				exit(0);
			}
		}
	}
	else
	{
		fprintf(gpFile, "Fragment Shader Compilation Log : No Errors\n");
	}

	gShaderProgramObject = glCreateProgram();
	glAttachShader(gShaderProgramObject,gVertexShaderObject);
	glAttachShader(gShaderProgramObject,gFragmentShaderObject);

	glBindAttribLocation(gShaderProgramObject,AC_ATTRIBUTE_VERTEX,"vPosition");

	glLinkProgram(gShaderProgramObject);

	iShaderLinkProgramLinkStatus = 0;
	glGetProgramiv(gShaderProgramObject,GL_LINK_STATUS,&iShaderLinkProgramLinkStatus);
	if (iShaderLinkProgramLinkStatus==GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog!=NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject,iInfoLogLength,&written,szInfoLog);
				fprintf(gpFile,"Shader program Link Log : %s\n",szInfoLog);
				free(szInfoLog);
				szInfoLog = NULL;
				uninitialize();
				exit(0);
			}
		}
	}
	else
	{
		fprintf(gpFile, "Shader program Link Log : No Errors\n");
	}

	gMVPUniform = glGetUniformLocation(gShaderProgramObject,"mvp");

	const GLfloat PyramidVertices[] =
	{
		0, 1, 0,
		-1, -1, 1,
		1, -1, 1,

		0, 1, 0,
		1, -1, 1,
		1, -1, -1,

		0, 1, 0,
		1, -1, -1,
		-1, -1, -1,

		0, 1, 0,
		-1, -1, -1,
		-1, -1, 1
	};

	const GLfloat PyramidColors[] =
	{
		1, 0, 0,
		0, 1, 0,
		0, 0, 1,

		1, 0, 0,
		0, 0, 1,
		0, 1, 0,

		1, 0, 0,
		0, 1, 0,
		0, 0, 1,

		1, 0, 0,
		0, 0, 1,
		0, 1, 0,
	};

	//Code for Pyramid
	//-------------
	glGenVertexArrays(1, &render_vao);
	glBindVertexArray(render_vao);

	glGenBuffers(2, buffers);

	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof(vmath::vec4), NULL, GL_DYNAMIC_COPY);
	vmath::vec4* positions = (vmath::vec4*)glMapBufferRange(GL_ARRAY_BUFFER,0,PARTICLE_COUNT * sizeof(vmath::vec4),GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (unsigned long long i = 0; i < PARTICLE_COUNT; i++)
	{
		positions[i] = vmath::vec4(random_vector(-10.0f, 10.0f), random_float());
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);
	glVertexAttribPointer(AC_ATTRIBUTE_VERTEX, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AC_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, velocity_buffer);
	glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof(vmath::vec4), NULL, GL_DYNAMIC_COPY);
	vmath::vec4* velocities = (vmath::vec4*)glMapBufferRange(GL_ARRAY_BUFFER,0,PARTICLE_COUNT * sizeof(vmath::vec4),GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (unsigned long long i = 0; i < PARTICLE_COUNT; i++)
	{
		velocities[i] = vmath::vec4(random_vector(-0.1f, 0.1f), 0.0f);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glGenTextures(2, tbos);
	for (unsigned long long i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_BUFFER, tbos[i]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, buffers[i]);
	}

	glGenBuffers(1, &attractor_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, attractor_buffer);
	glBufferData(GL_UNIFORM_BUFFER, 32 * sizeof(vmath::vec4), NULL, GL_STATIC_DRAW);
	for (unsigned long long i = 0; i < MAX_ATTRACTORS; i++)
	{
		attractor_masses[i] = 0.5f + random_float() * 0.5f;
	}
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, attractor_buffer);


	/*
	//Vertices for Pyramid
	glGenBuffers(1, &gVbo_Pyramid_Position);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_Pyramid_Position);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PyramidVertices), PyramidVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(AC_ATTRIBUTE_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AC_ATTRIBUTE_VERTEX);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//Color for Pyramid
	glGenBuffers(1, &gVbo_Pyramid_Color);
	glBindBuffer(GL_ARRAY_BUFFER, gVbo_Pyramid_Color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PyramidColors), PyramidColors, GL_STATIC_DRAW);
	glVertexAttribPointer(AC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(AC_ATTRIBUTE_COLOR);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	*/

	glBindVertexArray(0);
	//-------------

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	//glEnable(GL_DEPTH_TEST);
	glDisable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LEQUAL);
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glEnable(GL_CULL_FACE);

	glClearColor(0.0f,0.0f,0.0f,0.0f);

	glPointSize(2.0f);

	gPerspectiveProjectionMatrix = mat4::identity(); //Change 2

	resize(WIN_WIDTH,WIN_HEIGHT);
}

void update(void)
{
	gAnglePyramid = gAnglePyramid + 0.2f;
	if (gAnglePyramid >= 360.0f)
		gAnglePyramid = gAnglePyramid - 360.0f;
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	/*
	static const GLuint start_ticks = app_time() - 100000;
	GLuint current_ticks = app_time();
	static GLuint last_ticks = current_ticks;
	float time = ((start_ticks - current_ticks) & 0xFFFFF) / float(0xFFFFF);
	float delta_time = (float)(current_ticks - last_ticks) * 0.075f;

	if (delta_time < 0.01f)
	{
		return;
	}
	*/

	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&StartingTime);

	vmath::vec4* attractors = (vmath::vec4*)glMapBufferRange(GL_UNIFORM_BUFFER,
		0,
		32 * sizeof(vmath::vec4),
		GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	int i;

	for (i = 0; i < 32; i++)
	{
		attractors[i] = vmath::vec4(sinf(ElapsedMicroseconds.QuadPart * (float)(i + 4) * 7.5f * 20.0f) * 50.0f,cosf(ElapsedMicroseconds.QuadPart * (float)(i + 7) * 3.9f * 20.0f) * 50.0f,sinf(ElapsedMicroseconds.QuadPart * (float)(i + 3) * 5.3f * 20.0f) * cosf(ElapsedMicroseconds.QuadPart * (float)(i + 5) * 9.1f) * 100.0f,attractor_masses[i]);
	}

	glUnmapBuffer(GL_UNIFORM_BUFFER);

	/*
	// If dt is too large, the system could explode, so cap it to
	// some maximum allowed value
	if (delta_time >= 2.0f)
	{
		delta_time = 2.0f;
	}
	*/

	glUseProgram(gComputeProgramShaderObject);
	glBindImageTexture(0, velocity_tbo, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	glBindImageTexture(1, position_tbo, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
	// Set delta time

	//glUniform1f(dt_location, delta_time);
	glUniform1f(dt_location, ElapsedMicroseconds.QuadPart);
	
	// Dispatch
	glDispatchCompute(PARTICLE_GROUP_COUNT, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glUseProgram(0);

	//vmath::mat4 mvp = gPerspectiveProjectionMatrix * vmath::translate(0.0f, 0.0f, -160.0f) * vmath::rotate(time * 1000.0f, vmath::vec3(0.0f, 1.0f, 0.0f));
	vmath::mat4 mvp = gPerspectiveProjectionMatrix * vmath::translate(0.0f, 0.0f, -160.0f) * vmath::rotate(ElapsedMicroseconds.QuadPart * 1000.0f, vmath::vec3(0.0f, 1.0f, 0.0f));

	glUseProgram(gShaderProgramObject);
	glUniformMatrix4fv(0, 1, GL_FALSE, mvp);
	glBindVertexArray(render_vao);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	// glPointSize(2.0f);
	glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);
	glUseProgram(0);

	/*
	glUseProgram(gShaderProgramObject);

	//---------------- // For Pyramid
	mat4 modelViewMatrix = mat4::identity();
	mat4 modelViewProjectionMatrix = mat4::identity();

	modelViewMatrix = vmath::translate(0.0f, 0.0f, -6.0f);
	modelViewMatrix = modelViewMatrix*vmath::rotate(gAnglePyramid, 0.0f, 1.0f, 0.0f);
	modelViewProjectionMatrix = gPerspectiveProjectionMatrix*modelViewMatrix;
	glUniformMatrix4fv(gMVPUniform,1,GL_FALSE,modelViewProjectionMatrix);

	glBindVertexArray(gVao_Pyramid);
	glDrawArrays(GL_TRIANGLES, 0, 12);
	glBindVertexArray(0);
	//----------------

	glUseProgram(0);
	*/

	SwapBuffers(ghdc);

	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;


	//
	// We now have the elapsed number of ticks, along with the
	// number of ticks-per-second. We use these values
	// to convert to the number of elapsed microseconds.
	// To guard against loss-of-precision, we convert
	// to microseconds *before* dividing by ticks-per-second.
	//

	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
}

void uninitialize(void)
{
	if (gbFullScreen==true)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}

	if (render_vao)
	{
		glDeleteVertexArrays(1,&render_vao);
		render_vao = 0;
	}

	/*
	if (gVbo_Pyramid_Position)
	{
		glDeleteBuffers(1,&gVbo_Pyramid_Position);
		gVbo_Pyramid_Position = 0;
	}

	if (gVbo_Pyramid_Color)
	{
		glDeleteBuffers(1, &gVbo_Pyramid_Color);
		gVbo_Pyramid_Color = 0;
	}
	*/

	glDetachShader(gShaderProgramObject,gVertexShaderObject);
	glDetachShader(gShaderProgramObject,gFragmentShaderObject);

	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject = 0;

	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject = 0;

	glDeleteProgram(gShaderProgramObject);
	gShaderProgramObject = 0;

	glUseProgram(0);

	wglMakeCurrent(NULL,NULL);

	wglDeleteContext(ghrc);
	ghrc = NULL;

	ReleaseDC(ghwnd,ghdc);
	ghdc = NULL;

	if (gpFile)
	{
		fprintf(gpFile,"Log.txt is sucessfully closed\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}

void resize(int width, int height)
{
	if (height == 0)
		height = 1;
	
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	gPerspectiveProjectionMatrix=perspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,1000.0f);
}

void ToggleFullScreen(void)
{
	MONITORINFO mi;

	if (gbFullScreen == false)
	{
		dwStyle = GetWindowLong(ghwnd,GWL_STYLE);
		if (dwStyle && WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };

			if (GetWindowPlacement(ghwnd,&wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd,MONITORINFOF_PRIMARY),&mi))
			{
				SetWindowLong(ghwnd,GWL_STYLE,dwStyle&~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd,HWND_TOP,mi.rcMonitor.left,mi.rcMonitor.top,mi.rcMonitor.right-mi.rcMonitor.left,mi.rcMonitor.bottom-mi.rcMonitor.top,SWP_NOZORDER|SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle|WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd,&wpPrev);
		SetWindowPos(ghwnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}
}

const char* PassShaderinString(const char* fileName)
{
	FILE* f = NULL;
	if (fopen_s(&f, fileName, "rb") == 0)
	{
		fseek(f, 0, SEEK_END);
		long fsize = ftell(f);
		fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

		char* string = (char*)malloc(fsize + 1);
		fread(string, 1, fsize, f);
		fclose(f);

		string[fsize] = 0;
		return string;
	}
	return NULL;
}


