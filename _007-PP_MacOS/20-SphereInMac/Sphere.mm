//
//  BlueWindow.m
//
//
//  Created by Aaditya Kashid on 06/01/20.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CVDisplayLink.h>
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>
#import "../vmath/vmath.h"
#import "../sphere/sphere.h"
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
    NSString *logFileNameWithPath = [NSString stringWithFormat:@"%@/_OGLPPLog.txt", parentDirPath];
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
    
    [window setTitle:@"macOS:: OpenGL PP."];
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

    float sphereVertices[1146];
    float shpereNormals[1146];
    float sphereTextures[764];
    unsigned short shpereElements[2280];

    int iNumElements;
    int iNumVertices;

    GLuint vaoSphere, vboSphereNormals, vboSpherePos, vboSphereTexture, vboSphereElements;
    GLuint mvpUniform;
    mat4 perspectiveProjMat;

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
    "uniform mat4 u_mvp_matrix;" \
    "void main(void)" \
    "{" \
    "gl_Position = u_mvp_matrix * vPosition;" \
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
    "out vec4 FragColor;" \
    "void main(void)" \
    "{" \
    "FragColor = vec4(1.0, 1.0, 0.0, 1.0);" \
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


    //Get Sphere Data!.
    getSphereVertexData(sphereVertices,shpereNormals,sphereTextures,shpereElements);
    iNumVertices = getNumberOfSphereVertices();
    iNumElements = getNumberOfSphereElements();
    
    //Let There BE Sphere!.
    glGenVertexArrays(1, &vaoSphere);
    glBindVertexArray(vaoSphere);

    glGenBuffers(1, &vboSpherePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboSpherePos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vboSphereNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboSphereNormals);
    glBufferData(GL_ARRAY_BUFFER, sizeof(shpereNormals), shpereNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vboSphereTexture);
    glBindBuffer(GL_ARRAY_BUFFER, vboSphereTexture);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphereTextures), sphereTextures, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &vboSphereElements);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereElements);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(shpereElements), shpereElements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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
    
    //Declare MAtrices
    mat4 modelViewMat;
    mat4 modelviewProjMat;
    mat4 translateMat;
    //Initialization of matrices
    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();
    translateMat = mat4::identity();
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(gProgramShaderObj);
    
    translateMat = translate(0.0f, 0.0f, -3.0f);
    modelViewMat *= translateMat;
    modelviewProjMat = perspectiveProjMat * modelViewMat;
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);

    glBindVertexArray(vaoSphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,vboSphereElements);
    glDrawElements(GL_TRIANGLES, iNumElements,GL_UNSIGNED_SHORT, 0);
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
    glUseProgram(gProgramShaderObj);
    if(vboSphereElements) {
        glDeleteBuffers(1, &vboSphereElements);
        vboSphereElements = 0;
    }
    if(vboSphereTexture) {
        glDeleteBuffers(1, &vboSphereTexture);
        vboSphereTexture = 0;
    }
    if(vboSphereNormals) {
        glDeleteBuffers(1, &vboSphereNormals);
        vboSphereNormals = 0;
    }
    if(vboSpherePos) {
        glDeleteBuffers(1, &vboSpherePos);
        vboSpherePos = 0;
    }
    if(vaoSphere) {
        glDeleteVertexArrays(1, &vaoSphere);
        vaoSphere = 0;
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











