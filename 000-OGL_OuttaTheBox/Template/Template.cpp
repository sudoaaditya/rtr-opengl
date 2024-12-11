#include<Windows.h>
#include "gl\glew.h"
#include "gl/GL.h"
#include <stdio.h> 

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"User32.lib")
#pragma comment(lib,"GdI32.lib")
#pragma comment(lib,"kernel32.lib")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

HDC ghdc = NULL;
HWND ghwnd = NULL;
HGLRC ghrc = NULL;
DWORD dwStyle;

WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

bool gbFullscreen = false;
bool gbEscapeKeyIsPressed = false;
bool gbActiveWindow = false;


GLuint gShaderProgramObject;
GLuint gComputeShaderObject;

FILE *gpFile;

const GLint arraySize = 200;
float A[arraySize]; // Input array A
float B[arraySize]; // Input array B
float O[arraySize]; // Output array

// Create buffers
GLuint gVao_Compute;
GLuint inA, inB, out; //These are vbo's
GLuint gBufferSizeUniform;

bool bComputeDone = false;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
	void initialize(void);
	void uninitialize(void);
	void display(void);

	HWND hwnd;
	WNDCLASSEX wndclass;
	MSG msg;
	TCHAR szAppName[] = TEXT("GLEW Window");
	bool bDone = false;


	if(fopen_s(&gpFile,"Log.txt","w")!= 0)
	{
		MessageBox(NULL,TEXT("Log File Cannot be created"),TEXT("Error"),MB_OK);
		
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log File Successfully Opened\n");
	}	

	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hbrBackground = CreateSolidBrush(RGB(0, 255, 0));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wndclass);
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("GLEW Window"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);
	ghwnd = hwnd;

	initialize();
	ShowWindow(hwnd, nCmdShow);
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
				display();
			}
		}
	}

	uninitialize();
	return((int)msg.wParam);
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	void ToggleFullscreen(void);
	void resize(int, int);
	void uninitialize(void);

	switch (iMsg)
	{
	case WM_CREATE:
		break;
	case WM_ACTIVATE:
		if (HIWORD(wParam) == 0)
			gbActiveWindow = true;
		else
			gbActiveWindow = false;
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;
		case 0x46:
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
		default:
			break;
		}
		break;
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
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

void initialize(void)
{
	void resize(int, int);
	void uninitialize(void);
	const char* PassShaderinString(const char *fileName);

	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

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

	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
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
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	GLenum glew_error = glewInit();
	if(glew_error != GLEW_OK)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
		ReleaseDC(ghwnd,ghdc);
		ghdc = NULL;
	}
	
	gComputeShaderObject = glCreateShader(GL_COMPUTE_SHADER);
	const GLchar *computeShaderSourceCode = PassShaderinString("ComputeShader.txt");
	if (computeShaderSourceCode==NULL)
	{
		fprintf(gpFile, "Not able to read compute shader\n");
		DestroyWindow(ghwnd);
	}
	glShaderSource(gComputeShaderObject,1, (const GLchar **)&computeShaderSourceCode,NULL);
	glCompileShader(gComputeShaderObject);
	GLint iInfoLogLength = 0;
	GLint iShaderCompiledStatus = 0;
	char *szInfoLog = NULL;
	glGetShaderiv(gComputeShaderObject,GL_COMPILE_STATUS,&iShaderCompiledStatus);
	if (iShaderCompiledStatus==GL_FALSE)
	{
		glGetShaderiv(gComputeShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
		if (iInfoLogLength>0)
		{
			szInfoLog = (char*)malloc(iInfoLogLength);
			if (szInfoLog!=NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gComputeShaderObject,iInfoLogLength,&written,szInfoLog);
				fprintf(gpFile,"Compute Shader Compilation Log : %s\n",szInfoLog);
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
	
	gShaderProgramObject = glCreateProgram();
	glAttachShader(gShaderProgramObject,gComputeShaderObject);
	glLinkProgram(gShaderProgramObject);
	GLint iShaderLinkProgramLinkStatus = 0;
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

	gBufferSizeUniform = glGetUniformLocation(gShaderProgramObject, "BufferSize");
	
	// fill with some sample values
	for(size_t i = 0; i < arraySize; ++i)
	{
		A[i] = (float)i;
		B[i] = arraySize - i - 1.0f;
		O[i] = (float)1000.0f;
	}


	glGenVertexArrays(1, &gVao_Compute);
	glBindVertexArray(gVao_Compute);

		glGenBuffers(1, &inA);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, inA); //Bind buffer A
		glBufferData(GL_SHADER_STORAGE_BUFFER, arraySize * sizeof(float), A,GL_STATIC_DRAW); //Fill Buffer data
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
	
		glGenBuffers(1, &inB);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, inB); //Bind buffer B
		glBufferData(GL_SHADER_STORAGE_BUFFER, arraySize * sizeof(float), B,GL_STATIC_DRAW); //Fill Buffer data
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	
		glGenBuffers(1, &out);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, out); //Bind buffer O
		glBufferData(GL_SHADER_STORAGE_BUFFER, arraySize * sizeof(float), O,GL_STATIC_DRAW); //Fill Buffer data
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		//Bind buffers to fixed binding points (later will be used in the shader)
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, inA);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, inB);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, out);

	glBindVertexArray(0);

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
	glEnable(GL_CULL_FACE);


	glClearColor(0.0f, 0.0f, 1.0f, 0.0f); //blue

	resize(WIN_WIDTH, WIN_HEIGHT);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glUseProgram(gShaderProgramObject);

	glUniform1i(gBufferSizeUniform, arraySize);

	glBindVertexArray(gVao_Compute);

	if (bComputeDone==false)
	{

		glDispatchCompute(13, 1, 1); //Execute the compute shader with 13 workgroups

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT | GL_BUFFER_UPDATE_BARRIER_BIT); //force completeness before read back data

		bComputeDone = true;

		//Read back the output buffer to check the results
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, out); // Bind output buffer

		//Obtain a pointer to the output buffer data
		float* data = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		//Copy the data to our CPU located memory buffer
		memcpy(&O[0], data, sizeof(float) * arraySize);

		//Release the GPU pointer
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

		//From here, write results to a file, screen or send them back to memory for further process
		for (size_t i = 0; i < arraySize; ++i)
		{
			fprintf(gpFile, "Input1 (Index %zd : Value %f)\n", i, *(A + i));
			fprintf(gpFile, "Input2 (Index %zd : Value %f)\n", i, *(B + i));
			fprintf(gpFile, "Compute Output (Index %zd : Value %f)\n", i, *(data + i));
			fprintf(gpFile, "-----------------------------------------------------------------\n");
		}

		fprintf(gpFile, "Processing done\n");
	}
	else
	{
		fprintf(gpFile, "Skipping display() code\n");
	}


	glBindVertexArray(0);

	glUseProgram(0);

	SwapBuffers(ghdc);
}

void resize(int width, int height)
{
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	
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

	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(ghrc);
	ghrc = NULL;

	ReleaseDC(ghwnd, ghdc);
	ghdc = NULL;

	if(gpFile)
	{
		fprintf(gpFile, "\nLog file is Successfully closed");
		fclose(gpFile);
		gpFile = NULL;
	}

	DestroyWindow(ghwnd);
	ghwnd = NULL;
}

const char* PassShaderinString(const char *fileName)
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
