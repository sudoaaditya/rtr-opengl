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
    NSString *logFileNameWithPath = [NSString stringWithFormat:@"%@/_PerspectiveTriangleLog.txt", parentDirPath];
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
    
    [window setTitle:@"macOS:: Graph Paper!."];
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

    CVDisplayLinkRef displayLink;

    // Shader Variables
    GLuint gVertexShaderObj;
    GLuint gFragmentShaderObj;
    GLuint gProgramShaderObj;
    GLuint vao, vboAxes, vaoHor, vaoVert, vboLinesHor, vboLinesVert, vaoHorDwn, vboLinesHorDwn, vaoVertDwn, vboLinesVertDwn;
    GLuint vboCol1;
    GLuint vaoTriangle, vboTrianglePos, vboTriangleCol;
    GLuint vaoRectangle, vboRectanglePos, vboRectangleCol;
    GLuint vaoCircle, vboCirclePos, vboCircleCol;
    GLuint vaoOutCirc, vboOutCircPos, vboOutCircCol;
    GLuint mvpUniform;
    mat4 perspectiveProjMat;
    int circleLen;

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

    //For inCircle Calcus!
    GLfloat fx1, fx2, fx3, fy1, fy2, fy3;
	GLfloat fArea, fRad, fPer;
	GLfloat fdAB, fdBC, fdAC;
	GLfloat fxCord, fyCord;
	static GLfloat fAngle = 0.0f;
    static bool bWrite = false;
    
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
                    "gl_Position = vPosition * u_mvp_matrix;" \
                    "out_Color = vColor;" \
                    "}";;
    
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
    //Create Shader Program Shader Object!
    gProgramShaderObj = glCreateProgram();
    
    //Attach VS to Shader Prog
    glAttachShader(gProgramShaderObj, gVertexShaderObj);
    
    //Attach FS to Shader Prog
    glAttachShader(gProgramShaderObj, gFragmentShaderObj);
    
    //NOW BEFORE LINK : Prelinking Binding with Vertex Attribute
    glBindAttribLocation(gProgramShaderObj, AMC_ATTRIBUTE_POSITION, "vPosition");
    glBindAttribLocation(gProgramShaderObj, AMC_ATTRIBUTE_COLOR, "vColor");
    
    //Now Link The Program
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
    
    GLfloat axesVert[] = {1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f};

    GLfloat horLinesVert[80];
    float fSteps = 0.05;
    for(int i = 0; i< 20; i++) {
        for(int j = 0; j < 4; j++) {
            if(j == 0) {
                horLinesVert[(i*4) + j] = 1.5f;
            }
            else if(j == 2) {
                horLinesVert[(i * 4) + j] = -1.5f;
            }
            else {
                horLinesVert[(i * 4) + j] = fSteps;
            }
        }
        fSteps += 0.05f;
    }
    GLfloat horLinesVertDwn[80];
    fSteps = 0.05f;
    for(int i = 0; i< 20; i++) {
        for(int j = 0; j < 4; j++) {
            if(j == 0) {
                horLinesVertDwn[(i*4) + j] = 1.5f;
            }
            else if(j == 2) {
                horLinesVertDwn[(i * 4) + j] = -1.5f;
            }
            else {
                horLinesVertDwn[(i * 4) + j] = -fSteps;
            }
        }
        fSteps += 0.05f;
    }
    GLfloat vertLinesVert[120];
    fSteps = 0.05f;
    for(int i = 0; i< 30; i++) {
        for(int j = 0; j < 4; j++) {
            if(j == 1) {
                vertLinesVert[(i*4) + j] = 1.0f;
            }
            else if(j == 3) {
                vertLinesVert[(i * 4) + j] = -1.0f;
            }
            else {
                vertLinesVert[(i * 4) + j] = fSteps;
            }
        }
        fSteps += 0.05f;
    }
    GLfloat vertLinesVertDwn[120];
    fSteps = 0.05f;
    for(int i = 0; i< 30; i++) {
        for(int j = 0; j < 4; j++) {
            if(j == 1) {
                vertLinesVertDwn[(i*4) + j] = 1.0f;
            }
            else if(j == 3) {
                vertLinesVertDwn[(i * 4) + j] = -1.0f;
            }
            else {
                vertLinesVertDwn[(i * 4) + j] = -fSteps;
            }
        }
        fSteps += 0.05f;
    }
    GLfloat axesCol[] = {0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.1f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
    //Axes
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vboAxes);
    glBindBuffer(GL_ARRAY_BUFFER, vboAxes);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesVert), axesVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glGenBuffers(1, &vboCol1);
    glBindBuffer(GL_ARRAY_BUFFER, vboCol1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(axesCol), axesCol, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);   
    //Up Horizontal!
    glGenVertexArrays(1, &vaoHor);
    glBindVertexArray(vaoHor);
    glGenBuffers(1, &vboLinesHor);
    glBindBuffer(GL_ARRAY_BUFFER, vboLinesHor);
    glBufferData(GL_ARRAY_BUFFER, sizeof(horLinesVert), horLinesVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);       
    glBindVertexArray(0); 

    glGenVertexArrays(1, &vaoHorDwn);
    glBindVertexArray(vaoHorDwn);
    glGenBuffers(1, &vboLinesHorDwn);
    glBindBuffer(GL_ARRAY_BUFFER, vboLinesHorDwn);
    glBufferData(GL_ARRAY_BUFFER, sizeof(horLinesVertDwn), horLinesVertDwn, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);       
    glBindVertexArray(0);

    glGenVertexArrays(1, &vaoVert);
    glBindVertexArray(vaoVert);
    glGenBuffers(1, &vboLinesVert);
    glBindBuffer(GL_ARRAY_BUFFER, vboLinesVert);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertLinesVert), vertLinesVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);        
    glBindVertexArray(0); 

    glGenVertexArrays(1, &vaoVertDwn);
    glBindVertexArray(vaoVertDwn);
    glGenBuffers(1, &vboLinesVertDwn);
    glBindBuffer(GL_ARRAY_BUFFER, vboLinesVertDwn);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertLinesVertDwn), vertLinesVertDwn, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 0.0f, 0.0f, 1.0f);       
    glBindVertexArray(0); 

    //Related TO Triangle
    fx1 = 0.0f;
	fy1 = 1.0f;
	fx2 = -1.0f;
	fy2 = -1.0f;
	fx3 = 1.0f;
	fy3 = -1.0f;
    
    GLfloat TriangleVert[] = {fx1, fy1, 0.0f, fx2, fy2, 0.0f, fx3, fy3, 0.0f, fx1, fy1, 0.0f};
    GLfloat TriangleColor[] = {1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f};

    glGenVertexArrays(1, &vaoTriangle);
    glBindVertexArray(vaoTriangle);
    glGenBuffers(1, &vboTrianglePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboTrianglePos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleVert), TriangleVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glGenBuffers(1, &vboTriangleCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboTriangleCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleColor), TriangleColor, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);   
    
    //Rectangle!.
    GLfloat rectVert[] = {fx3, fy1, 0.0f, fx2, fy1, 0.0f, fx2, fy2, 0.0f, fx3, fy3, 0.0f, fx3, fy1, 0.0f};
    GLfloat rectCol[] = {1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f};

    glGenVertexArrays(1, &vaoRectangle);
    glBindVertexArray(vaoRectangle);
    glGenBuffers(1, &vboRectanglePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboRectanglePos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVert), rectVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    glGenBuffers(1, &vboRectangleCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboRectangleCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectCol), rectCol, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);  

    //InCircle cha Rada!.
    //diatances of each side of triangle by distance formula
	fdAB = sqrtf(((fx2 - fx1)*(fx2 - fx1)) + ((fy2 - fy1)*(fy2 - fy1)));
	fdBC = sqrtf(((fx3 - fx2)*(fx3 - fx2)) + ((fy3 - fy2)*(fy3 - fy2)));
	fdAC = sqrtf(((fx3 - fx1)*(fx3 - fx1)) + ((fy3 - fy1)*(fy3 - fy1)));

	//perimeter of triangle >> A+B+C and we need half of it for area 
	fPer = ((fdAB + fdAB + fdBC) / 2);

	//are of T = sqrt(P(P-A)(P-B)(P-C))
	fArea = sqrtf(fPer*(fPer - fdAB)*(fPer - fdBC)*(fPer - fdAC));

	//Radius of inCircle = AreaOf T/Perimete Of T
	fRad = (fArea / fPer);

	if (!bWrite) {
		fprintf(fptr, "fdAB = %f\nfdBC = %f\nfdAC = %f\nfPer =  %f\nfArea = %f\nfRad = %f\n", fdAB, fdBC, fdAC, fPer, fArea, fRad);
	}

	fxCord = (GLfloat)(((fdBC*fx1) + (fx2*fdAC) + (fx3*fdAB)) / (fPer * 2));
	fyCord = (GLfloat)(((fdBC*fy1) + (fy2*fdAC) + (fy3*fdAB)) / (fPer * 2));

	if (!bWrite) {
		fprintf(fptr, "\n\nfx1 = %f\tfy1 = %f\nfx2 = %f\tfy2 = %f\nfx3 = %f\tfy3 = %f\n\nfxCoord = %f\tfyCord = %f\n", fx1, fy1, fx2, fy2, fx3, fy3, fxCord, fyCord);
        fflush(fptr);
	}
    GLfloat circleVert[12580];
    GLfloat circleCol[18870];
    int i, j;
    float circleSteps = 0.0f;
    for(i = 0; i < 6290; i++) {
        for(j = 0; j < 2; j++) {
            if(j==0)
                circleVert[ (i*2) + j] =  fxCord + cosf(circleSteps)*fRad;
            else
                circleVert[ (i*2) + j] =  fyCord + sinf(circleSteps)*fRad;
        }
        circleSteps += 0.001f;
        circleCol[(i*2) + 0] = 1.0f;
        circleCol[(i*2) + 1] = 1.0f;
        circleCol[(i*2) + 2] = 0.0f;
    }
    circleLen = sizeof(circleVert);
    
    glGenVertexArrays(1, &vaoCircle);
    glBindVertexArray(vaoCircle);
    glGenBuffers(1, &vboCirclePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboCirclePos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVert), circleVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glGenBuffers(1, &vboCircleCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboCircleCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleCol), circleCol, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);     
    glBindVertexArray(0);

    GLfloat outCircVert[12580];
    GLfloat outCircColor[18870];
    circleSteps = 0.0f;
    for(i = 0; i< 6280; i++) {
        for(j = 0; j < 2; j++) {
            if(j == 0)
                outCircVert[ (i*2) + j] = 1.42f*cosf(circleSteps);
            else
                outCircVert[ (i*2) + j] = 1.42f*sinf(circleSteps);
        }
        circleSteps += 0.001f;
        circleSteps += 0.001f;
        outCircColor[(i*2) + 0] = 1.0f;
        outCircColor[(i*2) + 1] = 1.0f;
        outCircColor[(i*2) + 2] = 0.0f;
    }

    glGenVertexArrays(1, &vaoOutCirc);
    glBindVertexArray(vaoOutCirc);
    glGenBuffers(1, &vboOutCircPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboOutCircPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(outCircVert), outCircVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glGenBuffers(1, &vboOutCircCol);
    glBindBuffer(GL_ARRAY_BUFFER, vboOutCircCol);
    glBufferData(GL_ARRAY_BUFFER, sizeof(outCircColor), outCircColor, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_COLOR, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_COLOR);
    glBindBuffer(GL_ARRAY_BUFFER, 0);     
    glBindVertexArray(0);


    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

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
    
    mat4 modelViewMat;
    mat4 modelviewProjMat;

    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gProgramShaderObj);

    modelViewMat = translate(0.0f, 0.0f, -1.0f);

    modelviewProjMat = perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);

    glLineWidth(3.0f);
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glBindVertexArray(0);

    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();
    modelViewMat = translate(0.0f, 0.0f, -2.0f);
    modelviewProjMat = perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);

    glLineWidth(0.8f);
    glBindVertexArray(vaoHor);
    for(int i = 0; i< 40; i += 2) {
        glDrawArrays(GL_LINES, i, 2);
    }
    glBindVertexArray(0);

    glBindVertexArray(vaoHorDwn);
    for(int i = 0; i< 40; i += 2) {
        glDrawArrays(GL_LINES, i, 2);
    }
    glBindVertexArray(0);

    glBindVertexArray(vaoVert);
    for(int i = 0; i< 60; i += 2) {
        glDrawArrays(GL_LINES, i, 2);
    }
    glBindVertexArray(0);

    glBindVertexArray(vaoVertDwn);
    for(int i = 0; i< 60; i += 2) {
        glDrawArrays(GL_LINES, i, 2);
    }
    glBindVertexArray(0);

    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();
    modelViewMat = translate(0.0f, 0.0f, -3.5f);
    modelviewProjMat = perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);

    glLineWidth(2.9f);
    glBindVertexArray(vaoTriangle);
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 1, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glBindVertexArray(0);

    glBindVertexArray(vaoRectangle);
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 1, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glDrawArrays(GL_LINES, 3, 2);
    glBindVertexArray(0);

    glBindVertexArray(vaoCircle);
    glPointSize(1.0f);
    glDrawArrays(GL_POINTS, 0, circleLen);  
    glBindVertexArray(0);

    glBindVertexArray(vaoOutCirc);
    glPointSize(1.0f);
    glDrawArrays(GL_POINTS, 0, circleLen);  
    glBindVertexArray(0);

    glUseProgram(0);

    CGLFlushDrawable((CGLContextObj)[[self openGLContext] CGLContextObj]);
    CGLUnlockContext((CGLContextObj)[[self openGLContext] CGLContextObj]);
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
    if(vboAxes) {
        glDeleteBuffers(1, &vboAxes);
        vboAxes = 0;
    }
    if(vboLinesHor) {
        glDeleteBuffers(1, &vboLinesHor);
        vboLinesHor = 0;
    }
    if(vboLinesHorDwn) {
        glDeleteBuffers(1, &vboLinesHorDwn);
        vboLinesHorDwn = 0;
    }
    if(vboLinesVert) {
        glDeleteBuffers(1, &vboLinesVert);
        vboLinesVert = 0;
    }
    if(vboLinesVertDwn) {
        glDeleteBuffers(1, &vboLinesVertDwn);
        vboLinesVertDwn = 0;
    }
    if(vao) {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
    if(vaoHor) {
        glDeleteVertexArrays(1, &vaoHor);
        vaoHor = 0;
    }
    if(vaoHorDwn) {
        glDeleteVertexArrays(1, &vaoHorDwn);
        vaoHorDwn = 0;
    }
    if(vaoVert) {
        glDeleteVertexArrays(1, &vaoVert);
        vaoVert = 0;
    }
    if(vaoVertDwn) {
        glDeleteVertexArrays(1, &vaoVertDwn);
        vaoVertDwn = 0;
    }


    glUseProgram(gProgramShaderObj);
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



