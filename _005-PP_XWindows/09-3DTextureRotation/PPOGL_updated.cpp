#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>

#include<X11/Xlib.h>
#include<X11/XKBlib.h>
#include<X11/keysym.h>
#include<X11/Xutil.h>


#include"./../vmath/vmath.h"

#include<GL/glew.h>
#include<GL/gl.h>
#include<GL/glx.h>

#include<SOIL/SOIL.h>

using namespace std;
using namespace vmath;


bool gbIsFullscreen=false;

int giWindowWidth=800;
int giWindowHeight=600;


Window gWindow;
Display *gpDisplay=NULL;
Colormap gColormap;

// attributes enum 
enum
{
	AMC_ATTRIBUTE_POSITION=0,
	AMC_ATTRIBUTE_COLOR,
	AMC_ATTRIBUTE_NORMAL,
	AMC_ATTRIBUTE_TEXCOORD0
};


GLuint gShaderProgramObject=0;
GLfloat angleTriangle=0.0f;
GLfloat angleSquare=0.0f;


GLXContext gGLXContext;
XVisualInfo *gpXVisualInfo=NULL;
GLXFBConfig gGLXFBConfig;

//fbConfig stroing function pointer variables
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*,GLXFBConfig,GLXContext,Bool,const int*);
glXCreateContextAttribsARBProc glXCreateContextAttribsARB=NULL;

mat4 perspectiveProjectionMatrix;

GLuint vao_pyramid,vao_cube;

GLuint vbo_position_pyramid,vbo_position_cube;
GLuint vbo_texture_pyramid;
GLuint vbo_texture_cube;

GLuint mvpUniform=0;
GLuint samplerUniform=0;

GLuint texture_stone=0;
GLuint texture_kundali=0;

int main(void)
{
	void initialize(void);
	void uninitialize(void);
	void display(void);
	void update(void);
	void ToggleFullscreen(void);
	void resize(int,int);
	void createWindow(void);

	XEvent event;
	KeySym keysym;

	static int windowWidth=giWindowWidth;
	static int windowHeight=giWindowHeight;

	bool bDone=false;
	char keys[26];

	createWindow();
	initialize();

	while(bDone==false)
	{
		while(XPending(gpDisplay))
		{
			XNextEvent(gpDisplay,&event);
			switch(event.type)
			{
				case MapNotify:
					break;
				case KeyPress:
					keysym=XkbKeycodeToKeysym(gpDisplay,event.xkey.keycode,0,0);
					switch(keysym)
					{
						case XK_Escape:
							bDone=true;
							break;


					}
					XLookupString(&event.xkey,keys,sizeof(keys),NULL,NULL);
					switch(keys[0])
					{
						case 'f':
						case 'F':
							if(gbIsFullscreen==false)
							{
								gbIsFullscreen=true;
								ToggleFullscreen();

							}
							else
							{
								gbIsFullscreen=false;
								ToggleFullscreen();
							}
							break;
					}

					break;

					case ConfigureNotify:
						windowWidth=event.xconfigure.width;
						windowHeight=event.xconfigure.height;
						resize(windowWidth,windowHeight);
						break;
					case ButtonPress:
						switch(event.xbutton.button)
						{
							case 1:
								break;
							case 2:
								break;
							case 3:
								break;
							case 4:
								break;
						}
						break;
					case Expose:
						break;
					case MotionNotify:
						break;
					case 33:
						bDone=true;
						break;
			}
		}
		update();
		display();
	}

	uninitialize();
	return 0;
}


void createWindow(void)
{
	void uninitialize();

	XSetWindowAttributes winAttribs;

	int defaultScreen;
	int styleMask;

	GLXFBConfig *pGLXFBConfig=NULL;
	GLXFBConfig bestGLXFBConfig;
	XVisualInfo *pTempXVisualInfo=NULL;
	int iNumberOfFBConfig=0;

	static int frameBufferAttributes[]={
		GLX_X_RENDERABLE,True,
		GLX_DRAWABLE_TYPE,GLX_WINDOW_BIT,
		GLX_RENDER_TYPE,GLX_RGBA_BIT,
		GLX_X_VISUAL_TYPE,GLX_TRUE_COLOR,
		GL_DOUBLEBUFFER,True,
		GLX_RED_SIZE,8,
		GLX_BLUE_SIZE,8,
		GLX_GREEN_SIZE,8,
		GLX_ALPHA_SIZE,8,
		GLX_DEPTH_SIZE,24,
		GLX_STENCIL_SIZE,8,
		None
	};

	gpDisplay=XOpenDisplay(NULL);
	if(gpDisplay==NULL)
	{
		printf("\nError in XOpenDisplay()");
		uninitialize();
		exit(0);
	}
	defaultScreen=XDefaultScreen(gpDisplay);

	//code of GLXFBConfig
	pGLXFBConfig=glXGetFBConfigs(gpDisplay,defaultScreen,&iNumberOfFBConfig);

	int bestFrameBufferConfig=-1;
	int bestNumberOfSamples=-1;
	int worstFrameBufferConfig=-1;
	int worstNumberOfSamples=999;

	for (int i=0;i<iNumberOfFBConfig;i++)
	{
		pTempXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,pGLXFBConfig[i]);
		if(pTempXVisualInfo)
		{
			int iSampleBuffer;
			int iSamples;

			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfig[i],GLX_SAMPLE_BUFFERS,&iSampleBuffer);
			glXGetFBConfigAttrib(gpDisplay,pGLXFBConfig[i],GLX_SAMPLES,&iSamples);

			if(bestNumberOfSamples <0 ||iSampleBuffer && iSamples > bestNumberOfSamples)
			{
				bestFrameBufferConfig=i;
				bestNumberOfSamples=iSamples;
			}
			if(worstNumberOfSamples<0 || !iSampleBuffer || iSamples<worstNumberOfSamples)
			{
				worstNumberOfSamples=iSamples;
				worstFrameBufferConfig=i;
			}
		}

		XFree(pTempXVisualInfo);
		pTempXVisualInfo=NULL;
	}

	bestGLXFBConfig=pGLXFBConfig[bestFrameBufferConfig];
	gGLXFBConfig=bestGLXFBConfig;

	XFree(pGLXFBConfig);

	gpXVisualInfo=glXGetVisualFromFBConfig(gpDisplay,bestGLXFBConfig);

	winAttribs.border_pixel=0;
	winAttribs.border_pixmap=0;
	winAttribs.background_pixel=BlackPixel(gpDisplay,defaultScreen);
	winAttribs.background_pixmap=0;
	winAttribs.colormap=XCreateColormap(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo->screen),gpXVisualInfo->visual,AllocNone);
	gColormap=winAttribs.colormap;
	winAttribs.event_mask=ExposureMask|StructureNotifyMask|KeyPressMask|ButtonPressMask|VisibilityChangeMask|PointerMotionMask;

	styleMask=CWBackPixel|CWBorderPixel|CWColormap|CWEventMask;


	gWindow=XCreateWindow(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo->screen),0,0,giWindowWidth,giWindowHeight,0,gpXVisualInfo->depth,InputOutput,gpXVisualInfo->visual,styleMask,&winAttribs);
	if(!gWindow)
	{
		printf("\n Error in XCreateWindow()");
		uninitialize();
		exit(0);
	}

	XStoreName(gpDisplay,gWindow,"MY PPOGL APPLICATION");

	Atom windowManagerDelete=XInternAtom(gpDisplay,"WM_DELETE_WINDOW",True);

	XSetWMProtocols(gpDisplay,gWindow,&windowManagerDelete,1);

	XMapWindow(gpDisplay,gWindow);	

}

void ToggleFullscreen()
{
	Atom wm_state;
	Atom fullscreen;

	XEvent xev={0};

	wm_state=XInternAtom(gpDisplay,"_NET_WM_STATE",False);

	xev.type=ClientMessage;
	xev.xclient.window=gWindow;
	xev.xclient.format=32;
	xev.xclient.message_type=wm_state;
	xev.xclient.data.l[0]=gbIsFullscreen ? 0 : 1;

	fullscreen=XInternAtom(gpDisplay,"_NET_WM_STATE_FULLSCREEN",False);

	xev.xclient.data.l[1]=fullscreen;

	XSendEvent(gpDisplay,RootWindow(gpDisplay,gpXVisualInfo->screen),False,StructureNotifyMask,&xev);

}

void initialize()
{
	void resize(int,int);
	void uninitialize();
	bool LoadTextures(GLuint*,const char*);
	GLuint vertexShaderObject=0;
	GLuint fragmentShaderObject=0;


	//gGLXContext=glXCreateContext(gpDisplay,gpXVisualInfo,NULL,GL_TRUE);
	//getting required hardware renderable context for drawing on the screen which is best
	//this function is implementataion dependant for that we define funcrion poiter and the get the address for the same
	glXCreateContextAttribsARB=(glXCreateContextAttribsARBProc)glXGetProcAddressARB((GLubyte*)"glXCreateContextAttribsARB");
	if(glXCreateContextAttribsARB==NULL)
	{
		printf("\nError while getting the function pointer");
		uninitialize();
		exit(0);
	}


	const int attrib[]=
	{
		GLX_CONTEXT_MAJOR_VERSION_ARB,4,
		GLX_CONTEXT_MINOR_VERSION_ARB,5,
		GLX_CONTEXT_PROFILE_MASK_ARB,GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

	gGLXContext=glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attrib);
	if(!gGLXContext)
	{
		printf("\n Error in glXCreateContextAttribsARB the required fbConfigs hardware context is not availble");

		const int attrib[]={
			GLX_CONTEXT_MINOR_VERSION_ARB,1,
			GLX_CONTEXT_MINOR_VERSION_ARB,0,
			None
		};

		gGLXContext=glXCreateContextAttribsARB(gpDisplay,gGLXFBConfig,0,True,attrib);

	}

	if(!glXIsDirect(gpDisplay,gGLXContext))
	{
		printf("WARNING: Context is not hardware rendering context!..\n\n");
	}
	else
	{
		printf("\n\nSUCCESS: Obtained context is hardware rendeing context..\n\n");

	}

	glXMakeCurrent(gpDisplay,gWindow,gGLXContext);

	GLenum result ;
	result=glewInit();
	if(result!=GLEW_OK)
	{
		printf("\n Error while initializing glew\n\n exiting");
		uninitialize();
		exit(0);
	}

	//shaders

	//vertex shader object
	vertexShaderObject=glCreateShader(GL_VERTEX_SHADER);

	const GLchar *vertexShaderSourceCode=
		"#version 450 core" \
		"\n" \
		"in vec4 vPosition;" \
		"in vec2 vTexCoord;" \
		"uniform mat4 u_mvp_uniform;" \
		"out vec2 out_texcoord;" \
		"void main(void)" \
		"{" \
			"gl_Position=u_mvp_uniform * vPosition;" \
			"out_texcoord=vTexCoord;" \
		"}";

	//shader source to shaderobject
	glShaderSource(vertexShaderObject,1,(GLchar**)&vertexShaderSourceCode,NULL);

	//glCompileShader
	glCompileShader(vertexShaderObject);
	
	//error checing for shader compilation
	GLint iShadercompileStatus;
	int iInfoLogLength;
	char* szInfoLog=NULL;

	glGetShaderiv(vertexShaderObject,GL_COMPILE_STATUS,&iShadercompileStatus);
	if(iShadercompileStatus==GL_FALSE)
	{
		glGetShaderiv(vertexShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char*)malloc(sizeof(char)*iInfoLogLength);
			if(szInfoLog)
			{
				GLsizei written;
				glGetShaderInfoLog(vertexShaderObject,iInfoLogLength,&written,szInfoLog);
				printf("\n Error in vertex shader: %s",szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	else
	{
		printf("\n vertex shader compiled Successfully");
	}
	
	//fragment shader

	//create shader object
	fragmentShaderObject=glCreateShader(GL_FRAGMENT_SHADER);

	//shader source code
	const GLchar* fragmentShaderSourceCode=
		"#version 450 core" \
		"\n" \
		"in vec2 out_texcoord;" \
		"out vec4 fragColor;" \
		"uniform sampler2D u_sampler;" \
		"void main(void)" \
		"{" \
			"fragColor=texture(u_sampler,out_texcoord);" \
		"}";

	//specify shader source to shader object
	glShaderSource(fragmentShaderObject,1,(const GLchar**)&fragmentShaderSourceCode,NULL);

	//compile shader source
	glCompileShader(fragmentShaderObject);

	//error checking for compilation


	iShadercompileStatus=0;
	szInfoLog=NULL;
	iInfoLogLength=0;

	glGetShaderiv(fragmentShaderObject,GL_COMPILE_STATUS,&iShadercompileStatus);
	if(iShadercompileStatus==GL_FALSE)
	{
		glGetShaderiv(fragmentShaderObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char*)malloc(sizeof(char)*iInfoLogLength);
			if(szInfoLog)
			{
				GLsizei written;
				glGetShaderInfoLog(fragmentShaderObject,iInfoLogLength,&written,szInfoLog);
				printf("\nError in fragment shader source code compilation= %s",szInfoLog);
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	else
	{
		printf("\n fragment shader compiled Successfully");
	}
	


	//creating program for shaders for gpu
	gShaderProgramObject=glCreateProgram();

	//attach shader to program
	glAttachShader(gShaderProgramObject, vertexShaderObject);
	glAttachShader(gShaderProgramObject, fragmentShaderObject);


	//prelinking the atteibutes
	glBindAttribLocation(gShaderProgramObject,AMC_ATTRIBUTE_POSITION,"vPosition");
	glBindAttribLocation(gShaderProgramObject,AMC_ATTRIBUTE_TEXCOORD0,"vTexCoord");
	printf("\n AMC_ATTRIBUTE_TEXCCORD0:=%u",AMC_ATTRIBUTE_TEXCOORD0);

	//linking program
	glLinkProgram(gShaderProgramObject);

	//linking error 
	GLint iProgramLinkStatus=0;
	iInfoLogLength=0;
	szInfoLog=NULL;
	glGetProgramiv(gShaderProgramObject,GL_LINK_STATUS,&iProgramLinkStatus);

	if(iProgramLinkStatus==GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject,GL_INFO_LOG_LENGTH,&iInfoLogLength);
		if(iInfoLogLength>0)
		{
			szInfoLog=(char*)malloc(sizeof(char)*iInfoLogLength);
			if(szInfoLog)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject,iInfoLogLength,&written,szInfoLog);
				printf("\nError in linking the program:=%s ",szInfoLog );
				free(szInfoLog);
				uninitialize();
				exit(0);
			}
		}
	}
	else
	{
		printf("\n Linking Successfull");
	}

	//post linking for uniform binding
	mvpUniform=glGetUniformLocation(gShaderProgramObject,"u_mvp_uniform");
	printf("\n samplerUniform=%u",mvpUniform);
	samplerUniform=glGetUniformLocation(gShaderProgramObject,"u_sampler");
	printf("\n samplerUniform=%u",samplerUniform);

	const GLfloat pyramidVertices[]={
		0.0f,1.0f,0.0f,
		-1.0f,-1.0f,1.0f,
		1.0f,-1.0f,1.0f,

		 0.0f,1.0f,0.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,1.0f,
		
		0.0f,1.0f,0.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,

		0.0f,1.0f,0.0f,
		1.0f,-1.0f,1.0f,
		1.0f,-1.0f,-1.0f,


		
	};

	const GLfloat pyramidTexCoord[]={
		0.5f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,
	
		0.5f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,

		0.5f,1.0f,		
		0.0f,0.0f,
		1.0f,0.0f,

		0.5f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f
	};
	const GLfloat cubeVertices[]={
		-1.0f,1.0f,1.0f,
		-1.0f,-1.0f,1.0f,
		1.0f,-1.0f,1.0f,
		1.0f,1.0f,1.0f,//Front

		1.0f,1.0f,1.0f,
		1.0f,-1.0f,1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f,1.0f,-1.0f,//Right

		1.0f,1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,1.0f,-1.0f,//Back

		-1.0f,1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,1.0f,
		-1.0f,1.0f,1.0f,//Left

		-1.0f,1.0f,-1.0f,
		-1.0f,1.0f,1.0f,
		1.0f,1.0f,1.0f,
		1.0f,1.0f,-1.0f,//Top

		-1.0f,-1.0f,1.0f,
		-1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,-1.0f,
		1.0f,-1.0f,1.0f//Bottom
	};

	const GLfloat cubeTexCoord[]={
		1.0f,1.0f,
		0.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,
		
		1.0f,1.0f,
		0.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,

		1.0f,1.0f,
		0.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,

		1.0f,1.0f,
		0.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,
		
		1.0f,1.0f,
		0.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,

		1.0f,1.0f,
		0.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f
	};


	//start recording creation and starting the buffers to access for triangle of gpu
	glGenVertexArrays(1,&vao_pyramid);
	glBindVertexArray(vao_pyramid);
		glGenBuffers(1,&vbo_position_pyramid);
		glBindBuffer(GL_ARRAY_BUFFER,vbo_position_pyramid);
			glBufferData(GL_ARRAY_BUFFER,sizeof(pyramidVertices),pyramidVertices,GL_STATIC_DRAW);
			glVertexAttribPointer(AMC_ATTRIBUTE_POSITION,3,GL_FLOAT,GL_FALSE,0,NULL);
			glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
		glBindBuffer(GL_ARRAY_BUFFER,0);
		//color vbo
		glGenBuffers(1,&vbo_texture_pyramid);	
		glBindBuffer(GL_ARRAY_BUFFER,vbo_texture_pyramid);

			glBufferData(GL_ARRAY_BUFFER,sizeof(pyramidTexCoord),pyramidTexCoord,GL_STATIC_DRAW);
			glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD0,2,GL_FLOAT,GL_FALSE,0,NULL);
			glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD0);

		glBindBuffer(GL_ARRAY_BUFFER,0);

	glBindVertexArray(0);


	//for square vertices
	printf("cube vertices");
	glGenVertexArrays(1,&vao_cube);
	glBindVertexArray(vao_cube);
		glGenBuffers(1,&vbo_position_cube);
		glBindBuffer(GL_ARRAY_BUFFER,vbo_position_cube);
			glBufferData(GL_ARRAY_BUFFER,sizeof(cubeVertices),cubeVertices,GL_STATIC_DRAW);
			glVertexAttribPointer(AMC_ATTRIBUTE_POSITION,3,GL_FLOAT,GL_FALSE,0,NULL);
			glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
		glBindBuffer(GL_ARRAY_BUFFER,0);
	
		//glVertexAttrib3f(AMC_ATTRIBUTE_COLOR,0.0f,0.0f,1.0f);
		glGenBuffers(1,&vbo_texture_cube);
		glBindBuffer(GL_ARRAY_BUFFER,vbo_texture_cube);
			glBufferData(GL_ARRAY_BUFFER,sizeof(cubeTexCoord),cubeTexCoord,GL_STATIC_DRAW);
			glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD0,2,GL_FLOAT,GL_FALSE,0,NULL);
			glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD0);
		glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);



	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClearDepth(1.0f);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//percpetive projection matrix load identity
	perspectiveProjectionMatrix=mat4::identity();
	glEnable(GL_TEXTURE_2D);

	LoadTextures(&texture_stone,"Stone.bmp");
	LoadTextures(&texture_kundali,"Vijay_Kundali.bmp");

	//glEnable(GL_CULL_FACE);
	//glDisable(GL_FRONT_AND_BACK);

	resize(giWindowWidth,giWindowHeight);

}

bool LoadTextures(GLuint* texture,const char* imagePath)
{
	printf("In LoadTextures");
	int imageWidth;
	int imageHeight;

	bool bResult=false;

	unsigned char *imageData=NULL;
	imageData=SOIL_load_image(imagePath,&imageWidth,&imageHeight,0,SOIL_LOAD_RGB);

	if(imageData==NULL)
	{
		printf("\n Error in SOIL_load_image()");
		bResult=false;
		return bResult;
	}
	else
	{
		bResult=true;

		glPixelStorei(GL_UNPACK_ALIGNMENT,4);
		glGenTextures(1,texture);
		glBindTexture(GL_TEXTURE_2D,*texture);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,imageWidth,imageHeight,0,GL_RGB,GL_UNSIGNED_BYTE,imageData);

		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,0);
		SOIL_free_image_data(imageData);
		return bResult;
	}
	
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glUseProgram(gShaderProgramObject);
		
	//declaration for matrices
	mat4 modelViewMatrix;
	mat4 modelViewProjectionMatrix;
	mat4 translationMatrix;
	mat4 rotationMatrix;
	mat4 scaleMatrix;

	//load identity for matrix
	modelViewMatrix=mat4::identity();
	modelViewProjectionMatrix=mat4::identity();
	translationMatrix=mat4::identity();
	rotationMatrix=mat4::identity();
	scaleMatrix=mat4::identity();

	//translation for the same
	translationMatrix=translate(-1.50f,0.0f,-6.0f);

	//rotation for matrix
	rotationMatrix=rotate(angleTriangle,0.0f,1.0f,0.0f);

	
	modelViewMatrix=modelViewMatrix*translationMatrix;
	modelViewMatrix=modelViewMatrix*rotationMatrix;
	

	//multiplication of matrices
	modelViewProjectionMatrix=perspectiveProjectionMatrix*modelViewMatrix;


	glUniformMatrix4fv(mvpUniform,1,GL_FALSE,modelViewProjectionMatrix);

	//for textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,texture_stone);
	glUniform1i(samplerUniform,0);

	glBindVertexArray(vao_pyramid);
		glDrawArrays(GL_TRIANGLES,0,12);
		glBindTexture(GL_TEXTURE_2D,0);
	glBindVertexArray(0);


	//for square

	modelViewMatrix=mat4::identity();
	modelViewProjectionMatrix=mat4::identity();
	translationMatrix=mat4::identity();
	rotationMatrix=mat4::identity();

	//do necessary multiplication
	translationMatrix=translate(1.5f,0.0f,-6.0f);

	scaleMatrix=scale(0.75f,0.75f,0.75f);

	rotationMatrix=rotate(angleSquare,angleSquare,angleSquare);
	
	modelViewMatrix=modelViewMatrix*translationMatrix;
	modelViewMatrix=modelViewMatrix*scaleMatrix;
	modelViewMatrix=modelViewMatrix*rotationMatrix;

	modelViewProjectionMatrix=perspectiveProjectionMatrix*modelViewMatrix;

	glUniformMatrix4fv(mvpUniform,1,GL_FALSE,modelViewProjectionMatrix);

	//for texture of kundali
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,texture_kundali);
	glUniform1i(samplerUniform,0);

	glBindVertexArray(vao_cube);
		glDrawArrays(GL_TRIANGLE_FAN,0,4);
		glDrawArrays(GL_TRIANGLE_FAN,4,4);
		glDrawArrays(GL_TRIANGLE_FAN,8,4);
		glDrawArrays(GL_TRIANGLE_FAN,12,4);
		glDrawArrays(GL_TRIANGLE_FAN,16,4);
		glDrawArrays(GL_TRIANGLE_FAN,20,4);
		//glDrawArrays(GL_TRIANGLE_FAN,24,4);
	glBindVertexArray(0);

	glUseProgram(0);

	glXSwapBuffers(gpDisplay,gWindow);
}
void resize(int width,int height)
{
	if(height==0)
	{
		height=1;
	}
	glViewport(0,0,GLsizei(width),GLsizei(height));

	perspectiveProjectionMatrix=perspective(45.0f,GLfloat(width)/GLfloat(height),0.1f,100.0f);

}

void uninitialize()
{

	if(vao_pyramid)
	{
		glDeleteVertexArrays(1,&vao_pyramid);
		vao_pyramid=0;
	}

	if(vao_cube)
	{
		glDeleteVertexArrays(1,&vao_cube);
		vao_cube=0;
	}

	if(vbo_position_cube)
	{
		glDeleteBuffers(1,&vbo_position_cube);
		vbo_position_cube=0;
	}

	if(vbo_position_pyramid)
	{
		glDeleteBuffers(1,&vbo_position_pyramid);
		vbo_position_pyramid=0;
	}

	if(vbo_texture_pyramid)
	{
		glDeleteBuffers(1,&vbo_texture_pyramid);
		vbo_texture_pyramid=0;
	}

	if(gShaderProgramObject)
	{
		int iShaderCount=0;
		int iShaderNumber=0;
		glUseProgram(gShaderProgramObject);
			glGetProgramiv(gShaderProgramObject,GL_ATTACHED_SHADERS,&iShaderCount);

			GLuint  *pShaders=(GLuint*)malloc(sizeof(GLuint)*iShaderCount);

			for(iShaderNumber=0;iShaderNumber<iShaderCount;iShaderNumber++)
			{
				glDetachShader(gShaderProgramObject,pShaders[iShaderNumber]);
				glDeleteShader( pShaders[iShaderNumber]);
				pShaders[iShaderNumber]=0;
			}
			free(pShaders);

		glUseProgram(0);
		glDeleteProgram(gShaderProgramObject);
		gShaderProgramObject=0;

	}

	GLXContext currentContext;
	currentContext=glXGetCurrentContext();
	if(currentContext && gGLXContext==currentContext)
	{
		glXMakeCurrent(gpDisplay,0,0);
	}
	if(gGLXContext)
	{
		glXDestroyContext(gpDisplay,gGLXContext);
	
	}
	if(gWindow)
	{
		XDestroyWindow(gpDisplay,gWindow);
	}
	if(gColormap)
	{
		XFreeColormap(gpDisplay,gColormap);
	}
	if(gpXVisualInfo)
	{
		free(gpXVisualInfo);
		gpXVisualInfo=NULL;
	}
	if(gpDisplay)
	{
		XCloseDisplay(gpDisplay);
		gpDisplay=NULL;
	}
}

void update()
{
	angleTriangle=angleTriangle+0.5f;
	if(angleTriangle>360.0f)
	{
		angleTriangle=0.0f;
	}

	angleSquare=angleSquare+0.5f;
	if(angleSquare>360.0f)
	{
		angleSquare=0.0f;
	}
}
