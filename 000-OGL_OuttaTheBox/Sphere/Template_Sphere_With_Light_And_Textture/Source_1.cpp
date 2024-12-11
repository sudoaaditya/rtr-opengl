/*
Took BluScreen Code as Base
*/

#include <Windows.h>
#include <gl/glew.h>
#include <gl/GL.h>
#include <stdio.h> 
#include "vmath.h"
#include "OGLheader.h"

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

LRESULT CALLBACK AcCallBack(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	void initialize(void);
	void uninitialize(void);
	void display(void);
	void resize(int, int);

	WNDCLASSEX ac;
	HWND hwnd;
	MSG msg;
	TCHAR szClassName[] = TEXT("Window Native Code");
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
				display();

				if (gbEscapeKeyIsPressed == true)
				{
					bDone = true;
				}
			}
		}
	}
	uninitialize();
	return ((int)msg.wParam);
}

void initialize(void)
{
	void resize(GLint width, GLint height);
	DWORD WINAPI GetVertexShader(LPVOID *param);

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

	//------------------------------------------------------------------------------

	//Addding code for vertex and fragment shader here
	ac_gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	const GLchar *vertexShaderSourceCode =
		"void main(void)" \
		"{" \
		"}";

	glShaderSource(ac_gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

	glCompileShader(ac_gVertexShaderObject);

	ac_gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	const GLchar *fragmentShaderSourceCode =
		"void main(void)" \
		"{" \
		"}";

	glShaderSource(ac_gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

	glCompileShader(ac_gFragmentShaderObject);

	//Main Shader Program starts here
	ac_gShaderProgramObject = glCreateProgram();

	glAttachShader(ac_gShaderProgramObject, ac_gVertexShaderObject);

	glAttachShader(ac_gShaderProgramObject, ac_gFragmentShaderObject);

	glLinkProgram(ac_gShaderProgramObject);

	//------------------------------------------------------------------------------

	glShadeModel(GL_SMOOTH);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_CULL_FACE);

	//Defined Background color
	glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

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

	resize(WIN_WIDTH, WIN_HEIGHT);
}

void uninitialize(void)
{
	if (gbFullScreen == true)
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
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
	DestroyWindow(ghwnd);
	ghwnd = NULL;

	if (gpFile)
	{
		fprintf(gpFile, "Log File Is Successfully Closed.\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}

void resize(GLint width, GLint height)
{
	if (height == 0)
	{
		height = 1;
	}
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
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

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(ac_gShaderProgramObject);

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

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}