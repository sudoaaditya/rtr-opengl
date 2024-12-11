
//
//  Created by Aaditya Kashid on 06/01/20.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "../vmath/vmath.h"
using namespace vmath;

//Color MAcros
//Saffron 255 153 51
#define SAFFRON_R 1.0f
#define SAFFRON_G 0.600f
#define SAFFRON_B 0.20f
//White 255 255 255
#define WHITE_R 1.0f
#define WHITE_G 1.0f
#define WHITE_B 1.0f
//Green 18 136 7
#define GREEN_R 0.070f
#define GREEN_G 0.533f
#define GREEN_B 0.027f
//Blue 0 0 136
#define BLUE_R 0.0f
#define BLUE_G 0.0f
#define BLUE_B 0.533f


enum {
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD0
};


//C Style Function For DisplayLink!.
CVReturn MyDisplayLinkCallBack (CVDisplayLinkRef, const CVTimeStamp *, const CVTimeStamp *, CVOptionFlags, CVOptionFlags *, void *);

//Global Vars!.
FILE *fptr = NULL;

//Interface Declaration!.
@interface AppDelegate : NSObject <NSApplicationDelegate, NSWindowDelegate>
@end

@interface GLView: NSOpenGLView

@end

//Main
int main(int argc, const char* argv[]){
    
    NSAutoreleasePool *pNsPool = [[NSAutoreleasePool alloc] init];
    
    NSApp = [NSApplication sharedApplication];
    
    [NSApp setDelegate: [[AppDelegate alloc] init]];
    
    [NSApp run];
    
    [pNsPool release];
    
    return(0);
}

//Imterface Implementation.
@implementation AppDelegate {
@private
    NSWindow *window;
    GLView *glView;
}

-(void) applicationDidFinishLaunching:(NSNotification *)aNotify {
    
    //Log File!.
    NSBundle *mainBundle = [NSBundle mainBundle];
    NSString *appDirName = [mainBundle bundlePath];
    NSString *parentDirPath = [appDirName stringByDeletingLastPathComponent];
    NSString *logFileNameWithPath = [NSString stringWithFormat:@"%@/_StaticInidaLog.txt", parentDirPath];
    const char* szLogFileName = [logFileNameWithPath cStringUsingEncoding:NSASCIIStringEncoding];
    
    fptr = fopen(szLogFileName, "w");
    if(fptr == NULL){
        printf("Cannot Create Log File!...");
        [self release];
        [NSApp terminate:self];
    }
    else {
        fprintf(fptr, "Log File Created Successfully!!..\n\n");
    }
    
    //Create Window!.
    NSRect winRect = NSMakeRect(0.0, 0.0, 800.0, 600.0);
    
    window = [[NSWindow alloc]initWithContentRect:winRect styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable |NSWindowStyleMaskResizable backing: NSBackingStoreBuffered defer:NO];
    
    [window setTitle:@"macOS:: Static India!."];
    [window center];
    
    //Create View Object & Set it as view of out Window
    glView = [[GLView alloc]initWithFrame:winRect];
    [window setContentView:glView];
    //Tell WIndow that AppDelegate Will Take Charge Onwards && Set Winodw on Focus & Front in ZOrder
    [window setDelegate:self];
    [window makeKeyAndOrderFront: self];
    
}

-(void) applicationWillTerminate:(NSNotification *)aNotify{
    //Code.
    //Here we will take care of LogFile Closing
    fprintf(fptr, "Log File Closed Successfully!!..\n");
    
    if(fptr) {
        fclose(fptr);
        fptr = NULL;
    }
}

-(void) windowWillClose:(NSNotification *)aNotify {
    //Tell NSApp to close the APP. by closing AppDelegate
    [NSApp terminate:self];
}

-(void) dealloc{
    [glView release];
    
    [window release];
    
    [super dealloc];
    
}
@end

@implementation GLView {
    //Not Specifying AccessSpecifer is what declaring Private is. CPP!! MacOS doesn't allow inline initialization.
    CVDisplayLinkRef displayLink;

    // Shader Variables
    GLuint gVertexShaderObj;
    GLuint gFragmentShaderObj;
    GLuint gProgramShaderObj;
    GLuint vaoI1, vaoN, vaoD, vaoI2, vaoA, vaoDArc;
    GLuint vboI1Pos, vboI1Col, vboNPos, vboNCol, vboDPos, vboDCol, vboI2Pos, vboI2Col, vboAPos, vboACol, vboDArcPos, vboDArcCol;
    GLuint mvpUniform;
    mat4 perspectiveProjMat;

    //Animation Parameter
    bool bI1Reached, bNReached, bDReached, bI2Reached, bAReached;
    float fTransSpeed;
    GLfloat dArcCol[1260];

    //Declare MAtrices
    mat4 modelViewMat;    
    mat4 modelviewProjMat;
    mat4 translateMat;

}

-(id) initWithFrame:(NSRect) rect {
    
    self =[super initWithFrame:rect];
    
    if(self){
        
        [[self window] setContentView:self];
        
        NSOpenGLPixelFormatAttribute attribs[] = {
            NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion4_1Core,
            NSOpenGLPFAScreenMask, CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
            NSOpenGLPFANoRecovery, NSOpenGLPFAAccelerated,
            NSOpenGLPFAColorSize, 24,
            NSOpenGLPFADepthSize, 24,
            NSOpenGLPFAAlphaSize, 8,
            NSOpenGLPFADoubleBuffer, 0
        };
        
        NSOpenGLPixelFormat *pixelsFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attribs]autorelease];
        
        if(pixelsFormat == nil) {
            fprintf(fptr, "No Valid Pixel Format Available!.\n");
            [self release];
            [NSApp terminate:self];
        }else {
            fprintf(fptr, "Pixel Format Acquired!!..\n");
        }
        
        NSOpenGLContext *glContext = [[[NSOpenGLContext alloc]initWithFormat:pixelsFormat shareContext:nil]autorelease];
        
        if(glContext == nil) {
            fprintf(fptr, "No Valid Context Available!.\n");
            [self release];
            [NSApp terminate:self];
        }else {
            fprintf(fptr, "OpenGL Context Acquired!!..\n");
        }
        
        bI1Reached = false;
        bI2Reached = false;
        bAReached = false;
        bDReached = false;
        bNReached = false;
        fTransSpeed = 0.003f;
        
        [self setPixelFormat: pixelsFormat];
        [self setOpenGLContext: glContext];
    }
    return(self);
    
}

-(CVReturn) getFrameForTime:(const CVTimeStamp *) pOutTime {
    
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc]init];
    
    [self drawView];
    
    [pool release];
    
    return(kCVReturnSuccess);
}

-(void) prepareOpenGL {
    
    //For Shader Comiple
    GLint iShaderCompileStatus = 0;
    GLint iShaderInfoLogLen = 0;
    GLchar* szInfoLog = NULL;
    
    //print OGL Info!.
    fprintf(fptr, "\nOpenGLVersion:: %s\n ShadingLanguageVersion:: %s\n\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));
    
    [[self openGLContext] makeCurrentContext];
    
    GLint swapInterval = 1;
    [[self openGLContext] setValues:&swapInterval forParameter: NSOpenGLCPSwapInterval];
    
    //OPENGL!
    //Shader Code : Define Vertex Shader Object
    gVertexShaderObj = glCreateShader(GL_VERTEX_SHADER);
    
    // Write Vertex Shader Code!.
    const GLchar *vertexShaderSourceCode =
                    "#version 410 core" \
                    "\n" \
                    "in vec4 vPosition;" \
                    "in vec4 vColor;" \
                    "out vec4 out_Color;"   \
                    "uniform mat4 u_mvp_matrix;" \
                    "void main(void)" \
                    "{" \
                    "gl_Position = u_mvp_matrix * vPosition;" \
                    "out_Color = vColor;" \
                    "}";
    
    // Specify above code to VertexShader Object.
    glShaderSource(gVertexShaderObj, 1, (const GLchar**)&vertexShaderSourceCode, NULL);
    
    //Compile The Vertex Shader.
    glCompileShader(gVertexShaderObj);
    
    //Error Checking.
    glGetShaderiv(gVertexShaderObj, GL_COMPILE_STATUS, &iShaderCompileStatus);
    
    if(iShaderCompileStatus == GL_FALSE) {
        glGetShaderiv(gVertexShaderObj, GL_INFO_LOG_LENGTH, &iShaderInfoLogLen);
        
        if(iShaderInfoLogLen > 0) {
            szInfoLog = (GLchar*)malloc(iShaderInfoLogLen);
            
            if(szInfoLog != NULL) {
                GLsizei written;
                
                glGetShaderInfoLog(gVertexShaderObj,
                                   iShaderInfoLogLen,
                                   &written,
                                   szInfoLog);
                
                fprintf(fptr, "Vertex Shader Log::\n %s\n", szInfoLog);
                
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    else {
        fprintf(fptr, "Vertex Shader Compiled Successfully!!..\n");
    }
    
    //Fragment Shader : create Shader Objet!
    iShaderCompileStatus = 0;
    iShaderInfoLogLen = 0;
    szInfoLog = NULL;
    gFragmentShaderObj = glCreateShader(GL_FRAGMENT_SHADER);
    
    const GLchar *fragmentShaderSourceCode =
                    "#version 410 core" \
                    "\n" \
                    "in vec4 out_Color;" \
                    "out vec4 FragColor;" \
                    "void main(void)" \
                    "{" \
                    "FragColor = out_Color;" \
                    "}";
    
    glShaderSource(gFragmentShaderObj, 1, (const GLchar**)&fragmentShaderSourceCode, NULL);
    
    glCompileShader(gFragmentShaderObj);
    
    glGetShaderiv(gFragmentShaderObj, GL_COMPILE_STATUS, &iShaderCompileStatus);
    
    if(iShaderCompileStatus == GL_FALSE) {
        glGetShaderiv(gFragmentShaderObj, GL_INFO_LOG_LENGTH, &iShaderInfoLogLen);
        
        if(iShaderInfoLogLen > 0) {
            szInfoLog = (GLchar*)malloc(iShaderInfoLogLen);
            if(szInfoLog != NULL) {
                GLint written;
                
                glGetShaderInfoLog(gFragmentShaderObj,
                                   iShaderInfoLogLen,
                                   &written,
                                   szInfoLog);
                
                fprintf(fptr,"Fragment Shader Log :\n %s\n",szInfoLog);
                free(szInfoLog);
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    else {
        fprintf(fptr, "Fragement ShaderCompiled Successfully!!..\n");
    }
    
    //Noe Create Program
    GLint iProgLinkStatus = 0;
    GLint iProgLogLen = 0;
    GLchar* szProgLog = NULL;

    gProgramShaderObj = glCreateProgram();
    
    glAttachShader(gProgramShaderObj, gVertexShaderObj);
    glAttachShader(gProgramShaderObj, gFragmentShaderObj);
    
    glBindAttribLocation(gProgramShaderObj, AMC_ATTRIBUTE_POSITION, "vPosition");
    glBindAttribLocation(gProgramShaderObj, AMC_ATTRIBUTE_COLOR, "vColor");
    
    glLinkProgram(gProgramShaderObj);
    
    glGetProgramiv(gProgramShaderObj, GL_LINK_STATUS, &iProgLinkStatus);
    
    if(iProgLinkStatus == GL_FALSE) {
        glGetProgramiv(gProgramShaderObj, GL_INFO_LOG_LENGTH, &iProgLogLen);
        
        if(iProgLogLen > 0) {
            szProgLog = (GLchar*)malloc(iProgLogLen);
            
            if(szProgLog != NULL) {
                GLint written;
                
                glGetProgramInfoLog(gProgramShaderObj, iProgLogLen, &written, szProgLog);
                
                fprintf(fptr,"Program Link Log :\n %s\n",szProgLog);
                
                [self release];
                [NSApp terminate:self];
            }
        }
    }
    else {
        fprintf(fptr, "Program Linkage Successful!!\n");
    }
    
    mvpUniform = glGetUniformLocation(gProgramShaderObj, "u_mvp_matrix");
    
    
    //I
    GLfloat I1Vert[] = {-1.5f, 0.5f, 0.0f, -1.5f, -0.5f, 0.0f};
    GLfloat I1Col[] = {SAFFRON_R, SAFFRON_G, SAFFRON_B, GREEN_R, GREEN_G, GREEN_B};
    glGenVertexArrays(1, &vaoI1);
    glBindVertexArray(vaoI1);
    glGenBuffers(1, &vboI1Pos);
    glBindBuffer(GL_ARRAY_BUFFER, vboI1Pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(I1Vert), I1Vert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &vboI1Col);
    glBindBuffer(GL_ARRAY_BUFFER, vboI1Col);
    glBufferData(GL_ARRAY_BUFFER, sizeof(I1Col), I1Col, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //N
    GLfloat NVert[] = {-1.1f, -0.5f, 0.0f, -1.1f, 0.5f, 0.0f, -0.6f, -0.5f, 0.0f, -0.6f, 0.5f, 0.0f};
    GLfloat NCol [] = {GREEN_R,GREEN_G,GREEN_B,SAFFRON_R,SAFFRON_G,SAFFRON_B,GREEN_R,GREEN_G,GREEN_B,SAFFRON_R,SAFFRON_G,SAFFRON_B};
    glGenVertexArrays(1, &vaoN);
    glBindVertexArray(vaoN);
    glGenBuffers(1, &vboNPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboNPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(NVert), NVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &vboNCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboNCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(NCol), NCol, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //D
    //D ARC
    GLfloat dArcVert[945];
    GLfloat R = GREEN_R, G = GREEN_G, B = GREEN_B; 
    int i , j;
    GLfloat angle = 3 * M_PI/2;
    for(i=0; i < 315; i++) {
        for(j = 0; j < 3; j++) {
            if(j == 0)
                dArcVert[(i*3)+j] = -0.1+cosf(angle)*0.5f;
            else if(j == 1) 
                dArcVert[(i*3)+j] = sinf(angle)*0.5f;
            else 
                dArcVert[(i*3) +j] = 0.0f;
            
            dArcCol[(i*4)+0] = R;
            dArcCol[(i*4)+1] = G;
            dArcCol[(i*4)+2] = B;
            dArcCol[(i*4)+3] = 0.0f;
        }
        G += 0.00021f;
		if(G == SAFFRON_G){
			G = SAFFRON_G;
		}
		R += 0.0029f;
		if( R == SAFFRON_R){
			R = SAFFRON_R;
		}
		B += 0.00054f;
		if(B == SAFFRON_B) {
			B = SAFFRON_B;
		}
        angle += 0.01f;
    }
    glGenVertexArrays(1, &vaoDArc);
    glBindVertexArray(vaoDArc);
    glGenBuffers(1, &vboDArcPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboDArcPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dArcVert), dArcVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &vboDArcCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboDArcCol);
    glBufferData(GL_ARRAY_BUFFER, 1260 * 4, NULL, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 4, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //D Line
    GLfloat dLineVert[] = {-0.1f, 0.5f, 0.0f, -0.21f, 0.5f, 0.0f, -0.2f, 0.5f, 0.0f, -0.2f, -0.5f, 0.0f, -0.21f, -0.5f, 0.0f, -0.1f, -0.5f, 0.0f};
    //GLfloat dLineCol[] = {SAFFRON_R, SAFFRON_G, SAFFRON_B, SAFFRON_R, SAFFRON_G, SAFFRON_B, SAFFRON_R, SAFFRON_G, SAFFRON_B, GREEN_R, GREEN_G, GREEN_B, GREEN_R, GREEN_G, GREEN_B, GREEN_R, GREEN_G, GREEN_B};
    glGenVertexArrays(1, &vaoD);
    glBindVertexArray(vaoD);
    glGenBuffers(1, &vboDPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboDPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dLineVert), dLineVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &vboDCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboDCol);
    glBufferData(GL_ARRAY_BUFFER, 3*6*4, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //I2
    GLfloat I2Vert[] = {0.7f,0.5f,0.0f,0.7f,-0.5f,0.0f};
    glGenVertexArrays(1, &vaoI2);
    glBindVertexArray(vaoI2);
    glGenBuffers(1, &vboI2Pos);
    glBindBuffer(GL_ARRAY_BUFFER, vboI2Pos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(I2Vert), I2Vert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &vboI2Col);
    glBindBuffer(GL_ARRAY_BUFFER, vboI2Col);
    glBufferData(GL_ARRAY_BUFFER, sizeof(I1Col), I1Col, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //A
    GLfloat aVert[] = {1.0f, -0.5f, 0.0f, 1.3f, 0.5f, 0.0f, 1.6f, -0.5f, 0.0f, 1.15f,0.025f,0.0f, 1.45f,0.025f,0.0f, 1.15f,0.0f,0.0f,1.45f,0.0f,0.0f, 1.15f,-0.02f,0.0f,1.45f,-0.02f,0.0f};
    GLfloat aCol[] = {GREEN_R,GREEN_G,GREEN_B, SAFFRON_R,SAFFRON_G,SAFFRON_B, GREEN_R,GREEN_G,GREEN_B, SAFFRON_R,SAFFRON_G,SAFFRON_B, SAFFRON_R,SAFFRON_G,SAFFRON_B, 1.0f,1.0f,1.0f, 1.0f,1.0f,1.0f, GREEN_R,GREEN_G,GREEN_B,GREEN_R,GREEN_G,GREEN_B};
    glGenVertexArrays(1, &vaoA);
    glBindVertexArray(vaoA);
    glGenBuffers(1, &vboAPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboAPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(aVert), aVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenBuffers(1, &vboACol);
    glBindBuffer(GL_ARRAY_BUFFER, vboACol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(aCol), aCol, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    perspectiveProjMat = mat4::identity();
    
    //Display Link COde!.
    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &MyDisplayLinkCallBack, self);
    CGLContextObj cglContext = (CGLContextObj)[[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
    CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, cglContext, cglPixelFormat);
    CVDisplayLinkStart(displayLink);
}

-(void) reshape {
    
    //Lock the context
    CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    
    NSRect rect = [self bounds];
    
    GLfloat width = rect.size.width;
    GLfloat height = rect.size.height;
    
    if(height == 0){
        height = 1;
    }
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    
    perspectiveProjMat = perspective(45.0f, width/height, 0.1f, 100.0f);
    
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}

-(void) drawRect:(NSRect) dirtyRect {
    //Code!.
    [self drawView];
}

-(void) drawView {
    [[self openGLContext] makeCurrentContext];
    
    CGLLockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(gProgramShaderObj);

    [self drawI1];
	
	if(bAReached == true) {
		[self drawN];
	}
	if(bI2Reached == true) {
		[self drawD];
	}
	if(bNReached == true) {
		[self drawI2];
	}
    if(bI1Reached == true) {
		[self drawA];
	}

    glUseProgram(0);

    CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
}


-(void) drawI1 {
    static float bXTrans = -0.8f;

    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();

    modelViewMat  = translate(bXTrans, 0.0f, -3.0f);
    modelviewProjMat =  perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);
    
    //Draw I
    glBindVertexArray(vaoI1);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);

    bXTrans += fTransSpeed;
	if(bXTrans >= 0.0f){
		bXTrans = 0.0f;
		bI1Reached = true;
	}


}

-(void) drawN {

    static GLfloat bYTrans = 2.0f;

    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();

    modelViewMat  = translate(0.0f, bYTrans, -3.0f);
    modelviewProjMat =  perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);
    
    //Draw N
    glBindVertexArray(vaoN);
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 1, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glBindVertexArray(0);

    bYTrans -= fTransSpeed;
	if(bYTrans <= 0.0f) {
		bYTrans = 0.0f;
		bNReached = true;
	}
	
}

-(void) drawD{
    //D
	static GLfloat A = 0.0f;
	static GLfloat Sr=0.0f, Sg = 0.0f, Sb = 0.0f;
	static GLfloat Gr=0.0f, Gg = 0.0f, Gb = 0.0f;
	static int iCntStat = 0;
    GLfloat colorArray[] = {Sr, Sg, Sb, Sr, Sg, Sb, Sr, Sg, Sb, Gr, Gg, Gb, Gr, Gg, Gb, Gr, Gg, Gb};

    glBindVertexArray(vaoD);
    glBindBuffer(GL_ARRAY_BUFFER, vboDCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colorArray), colorArray, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glDrawArrays(GL_LINES, 4, 2);
    glBindVertexArray(0);
    for(int i = 0; i<315; i++) {
        dArcCol[((i*4)+3)] = A;
        
    }
    glBindVertexArray(vaoDArc);
    glBindBuffer(GL_ARRAY_BUFFER, vboDArcCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(dArcCol), dArcCol, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_POINTS, 0, sizeof(dArcCol)/sizeof(GLfloat));
    glBindVertexArray(0);

    if(iCntStat >= 5) {
	//fprintf(fptr,"Here Val SR =%d\n",iCntStat);

		Sr += 0.001f;
		Sg += 0.0006f;
		Sb += 0.0002f; 
		if(Sr >= SAFFRON_R) {
			//fprintf(fptr,"Here Val itr =%d\n",itr);
			bDReached = true;
			Sr = SAFFRON_R;
		}
		if(Sg >= SAFFRON_G) {
			Sg = SAFFRON_G;
		}
		if(Sb >= SAFFRON_B) {
			Sb = SAFFRON_B;
		}

		Gr += 0.00007f;
		Gg += 0.0006f;
		Gb += 0.000027f;

		if(Gr >= GREEN_R){
			Gr = GREEN_R;
		}
		if(Gg >= GREEN_G){
			Gg = GREEN_G;
		}
		if(Gb >= GREEN_B){
			Gb = GREEN_B;
		}
		
		A += 0.00057f;
		if(A >= 1.0f){
			A = 1.0f;
		}
		iCntStat =0;
	}
	else {
		iCntStat++;
	}
}

-(void)  drawI2{
    static GLfloat bYTrans = -2.0f;
        //Draw I
    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();

    modelViewMat  = translate(0.0f, bYTrans, -3.0f);
    modelviewProjMat =  perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);
    
    glBindVertexArray(vaoI2);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);

    bYTrans += fTransSpeed;
	if(bYTrans >= 0.0f) {
		bYTrans = 0.0f;
		bI2Reached = true;
	}
}

-(void) drawA{
    static GLfloat bXTrans = 0.8f;

    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();

    modelViewMat  = translate(bXTrans, 0.0f, -3.0f);
    modelviewProjMat =  perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);

    //Draw  A
    glBindVertexArray(vaoA);
    //Middle Strip
    glDrawArrays(GL_LINES, 3, 2);
    glDrawArrays(GL_LINES, 5, 2);
    glDrawArrays(GL_LINES, 7, 2);
    //Lines of A
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 1, 2);
    glBindVertexArray(0);

    bXTrans -= fTransSpeed;
	if(bXTrans <= 0.0f){
	    bXTrans = 0.0f;
		bAReached = true;
	}
}

//This Step to add our app at the recent most level in Responser chain!.
-(BOOL) acceptsFirstResponder {
    //Code!.
    [[self window] makeFirstResponder: self];
    return(YES);
}

-(void) keyDown:(NSEvent *)event {
    
    //This to Take first key pressed in the multiple keyDowns
    int keyCode = (int)[[event characters] characterAtIndex:0];
    
    switch (keyCode) {
        case 27: //Esc
            [self release];
            [NSApp terminate:self];
            break;
            
        case 'F':
        case 'f':
            [[self window] toggleFullScreen:self]; //Repainting is took care of!
            break;
            
        default:
            break;
    }
    
}


-(void) dealloc {
    
    CVDisplayLinkStop(displayLink);
    CVDisplayLinkRelease(displayLink);
    
    //Uninitialize
    glUseProgram(gProgramShaderObj);
    //Delete VBO
    if(vboAPos) {
        glDeleteBuffers(1, &vboAPos);
        vboAPos = 0;
    }
    if(vboACol) {
        glDeleteBuffers(1, &vboACol);
        vboACol = 0;
    }
    if(vaoA) {
        glDeleteVertexArrays(1, &vaoA);
        vaoA = 0;
    }

    if(vboNCol) {
        glDeleteBuffers(1, &vboNCol);
        vboNCol = 0;
    }
    if(vboNCol) {
        glDeleteBuffers(1, &vboNCol);
        vboNCol = 0;
    }
    if(vaoN) {
        glDeleteVertexArrays(1, &vaoN);
        vaoN = 0;
    }

    if(vboDCol) {
        glDeleteBuffers(1, &vboDCol);
        vboDCol = 0;
    }
    if(vboDCol) {
        glDeleteBuffers(1, &vboDCol);
        vboDCol = 0;
    }
    if(vaoD) {
        glDeleteVertexArrays(1, &vaoD);
        vaoD = 0;
    }

    if(vboDArcCol) {
        glDeleteBuffers(1, &vboDArcCol);
        vboDArcCol = 0;
    }
    if(vboDArcCol) {
        glDeleteBuffers(1, &vboDArcCol);
        vboDArcCol = 0;
    }
    if(vaoDArc) {
        glDeleteVertexArrays(1, &vaoDArc);
        vaoDArc = 0;
    }

    if(vboI1Col) {
        glDeleteBuffers(1, &vboI1Col);
        vboI1Col = 0;
    }
    if(vboI1Col) {
        glDeleteBuffers(1, &vboI1Col);
        vboI1Col = 0;
    }
    if(vaoI1) {
        glDeleteVertexArrays(1, &vaoI1);
        vaoI1 = 0;
    }

    if(vboI2Col) {
        glDeleteBuffers(1, &vboI2Col);
        vboI2Col = 0;
    }
    if(vboI2Col) {
        glDeleteBuffers(1, &vboI2Col);
        vboI2Col = 0;
    }
    if(vaoI2) {
        glDeleteVertexArrays(1, &vaoI2);
        vaoI2 = 0;
    }

    
    //Safe Release to Shader!!.
    if(gProgramShaderObj) {
        GLsizei iShaderCnt = 0;
        GLsizei iShaderNo = 0;
        
        glUseProgram(gProgramShaderObj);
        
        //Ask prog how many shader are attached to it
        glGetProgramiv(gProgramShaderObj,
                       GL_ATTACHED_SHADERS,
                       &iShaderCnt);
        
        GLuint *pShaders = (GLuint*)malloc(iShaderCnt * sizeof(GLuint));
        
        if(pShaders) {
            glGetAttachedShaders(gProgramShaderObj,
                                 iShaderCnt,
                                 &iShaderCnt,
                                 pShaders);
            
            for(iShaderNo = 0; iShaderNo < iShaderCnt; iShaderNo++) {
                
                glDetachShader(gProgramShaderObj, pShaders[iShaderNo]);
                pShaders[iShaderNo] = 0;
            }
            free(pShaders);
        }
        glDeleteProgram(gProgramShaderObj);
        gProgramShaderObj = 0;
        glUseProgram(0);
    }
    [super dealloc];
    
}

@end

//Callback Implementation!.
CVReturn MyDisplayLinkCallBack(CVDisplayLinkRef displayLink, const CVTimeStamp *pNow, const CVTimeStamp *pOutTime, CVOptionFlags flagsIn, CVOptionFlags *pFlagsOut, void *pDisplayLinkContext) {
    
    CVReturn result = [(GLView *)pDisplayLinkContext getFrameForTime:pOutTime];
    
    return(result);
}











