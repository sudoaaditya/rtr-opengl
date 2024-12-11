//
//  GLESView.m
//  04-BWShapes
//
//  Created by Ananth Chandrasekharan on 11/01/20.
//

#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>
#import "GLESView.h"
#import "vmath.h"
using namespace vmath;

enum {
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD0
};


@implementation GLESView {
    
    EAGLContext *eaglContext;
    
    GLuint frameBuffer;
    GLuint colorBuffer;
    GLuint depthBuffer;
    
    id displayLink;
    NSInteger animationFrameInterval;
    BOOL isAnimating;
    
    // Shader Variables
    GLuint gVertexShaderObj;
    GLuint gFragmentShaderObj;
    GLuint gProgramShaderObj;
    GLuint vaoTriangle, vboTrianglePos;
    GLuint vaoCircle, vboCirclePos;
    GLuint vaoLine, vboLinePos;
    GLuint mvpUniform;
    mat4 perspectiveProjMat;
    GLfloat circleVert[12580];

}

-(id) initWithFrame:(CGRect)frame {
    
    self = [super initWithFrame:frame];
    
    
    if(self) {
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)super.layer;
        eaglLayer.opaque = YES;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys: [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
        
        eaglContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        if(eaglContext == nil) {
            [self release];
            return(nil);
        }
        
        [EAGLContext setCurrentContext: eaglContext];
        
        //Frame Buffer! Thing!
        glGenFramebuffers(1, &frameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        //COlor Buffer!
        glGenRenderbuffers(1, &colorBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
        [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
        
        //Depth Buffer!.
        GLint backingWidth;
        GLint backingHeight;
        
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
        
        glGenRenderbuffers(1, &depthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
        
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            printf("FAILED :: TO create Compelete FBO!");
            glDeleteRenderbuffers(1, &depthBuffer);
            glDeleteRenderbuffers(1, &colorBuffer);
            glDeleteFramebuffers(1, &frameBuffer);
            
            return(nil);
        }
        else {
            printf("FBO Created Successfully!\n");
        }
        
        printf("GLES RENDERER :: %s\nGLES VERSION :: %s\nGLSL Version :: %s\n",
               glGetString(GL_RENDERER),
               glGetString(GL_VERSION),
               glGetString(GL_SHADING_LANGUAGE_VERSION));
        
        isAnimating = NO;
        animationFrameInterval = 60;
        
        //All Initialization in OpenGL Must be DOne Here!.
        [self initialize];
        
        //Gesture SetUP
        [self addGestures];
        
    }
    else {
        printf("Failed To Acquire the Instance form parent!\n");
    }
    
    return(self);
}

+(Class) layerClass {
    return ([CAEAGLLayer class]);
}

-(void) addGestures {
    
    //Gesture Recognition!
    //Single Tap
    UITapGestureRecognizer *singleTapRecog = [[UITapGestureRecognizer alloc] initWithTarget:self  action:@selector(onSingleTap:)];
    [singleTapRecog setNumberOfTapsRequired:1];
    [singleTapRecog setNumberOfTouchesRequired:1];
    [singleTapRecog setDelegate:self];
    [self addGestureRecognizer:singleTapRecog];
    
    //Double Tap
    UITapGestureRecognizer *doubleTapRecog = [[UITapGestureRecognizer alloc] initWithTarget:self  action:@selector(onDoubleTap:)];
    [doubleTapRecog setNumberOfTapsRequired:2];
    [doubleTapRecog setNumberOfTouchesRequired:1];
    [doubleTapRecog setDelegate:self];
    [self addGestureRecognizer:singleTapRecog];
    
    //On Double Tap, Single
    [singleTapRecog requireGestureRecognizerToFail:doubleTapRecog];
    
    //Swipe Gesture!
    UISwipeGestureRecognizer *swipeRecog = [[UISwipeGestureRecognizer alloc] initWithTarget:self action:@selector(onSwipe:)];
    [self addGestureRecognizer:swipeRecog];
    
    //Long Press!
    UILongPressGestureRecognizer *longPressRecog = [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(onLongPress:)];
    [self addGestureRecognizer:longPressRecog];
    
    
}

-(void) initialize {
    //For Shader Comiple
    GLint iShaderCompileStatus = 0;
    GLint iShaderInfoLogLen = 0;
    GLchar* szInfoLog = NULL;
    
    //For inCircle Calcus!
    GLfloat fx1, fx2, fx3, fy1, fy2, fy3;
    GLfloat fArea, fRad, fPer;
    GLfloat fdAB, fdBC, fdAC;
    GLfloat fxCord, fyCord;
    static bool bWrite = false;
    
    //OPENGL!
    //Shader Code : Define Vertex Shader Object
    gVertexShaderObj = glCreateShader(GL_VERTEX_SHADER);
    
    // Write Vertex Shader Code!.
    const GLchar *vertexShaderSourceCode =
                "#version 300 es" \
                "\n" \
                "in vec4 vPosition;" \
                "in vec4 vColor;" \
                "out vec4 out_color;" \
                "uniform mat4 u_mvp_matrix;" \
                "void main(void)" \
                "{" \
                "gl_Position = u_mvp_matrix * vPosition;" \
                "gl_PointSize = 1.0f;" \
                "out_color = vColor;" \
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
                
                printf("Vertex Shader Log::\n %s\n", szInfoLog);
                
                free(szInfoLog);
                [self release];
            }
        }
    }
    else {
        printf("Vertex Shader Compiled Successfully!!..\n");
    }
    
    //Fragment Shader : create Shader Objet!
    iShaderCompileStatus = 0;
    iShaderInfoLogLen = 0;
    szInfoLog = NULL;
    gFragmentShaderObj = glCreateShader(GL_FRAGMENT_SHADER);
    
    const GLchar *fragmentShaderSourceCode =
                "#version 300 es" \
                "\n" \
                "precision highp float;"
                "in vec4 out_color;" \
                "out vec4 FragColor;" \
                "void main(void)" \
                "{" \
                "   FragColor = out_color;" \
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
                
                printf("Fragment Shader Log :\n %s\n",szInfoLog);
                free(szInfoLog);
                [self release];
            }
        }
    }
    else {
        printf("Fragement ShaderCompiled Successfully!!..\n");
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
                
                printf("Program Link Log :\n %s\n",szProgLog);
                
                [self release];
            }
        }
    }
    else {
        printf("Program Linkage Successful!!\n");
    }
    
    mvpUniform = glGetUniformLocation(gProgramShaderObj, "u_mvp_matrix");
    //Related TO Triangle
    fx1 = 0.0f;
    fy1 = 1.0f;
    fx2 = -1.0f;
    fy2 = -1.0f;
    fx3 = 1.0f;
    fy3 = -1.0f;
    
    GLfloat TriangleVert[] = {fx1, fy1, 0.0f, fx2, fy2, 0.0f, fx3, fy3, 0.0f, fx1, fy1, 0.0f};
    
    glGenVertexArrays(1, &vaoTriangle);
    glBindVertexArray(vaoTriangle);
    glGenBuffers(1, &vboTrianglePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboTrianglePos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TriangleVert), TriangleVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 1.0f, 1.0f, 1.0f);
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
        printf("fdAB = %f\nfdBC = %f\nfdAC = %f\nfPer =  %f\nfArea = %f\nfRad = %f\n", fdAB, fdBC, fdAC, fPer, fArea, fRad);
    }
    
    fxCord = (GLfloat)(((fdBC*fx1) + (fx2*fdAC) + (fx3*fdAB)) / (fPer * 2));
    fyCord = (GLfloat)(((fdBC*fy1) + (fy2*fdAC) + (fy3*fdAB)) / (fPer * 2));
    
    if (!bWrite) {
        printf("\n\nfx1 = %f\tfy1 = %f\nfx2 = %f\tfy2 = %f\nfx3 = %f\tfy3 = %f\n\nfxCoord = %f\tfyCord = %f\n", fx1, fy1, fx2, fy2, fx3, fy3, fxCord, fyCord);
    }
    //GLfloat circleVert[12580];
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
    }
    
    glGenVertexArrays(1, &vaoCircle);
    glBindVertexArray(vaoCircle);
    glGenBuffers(1, &vboCirclePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboCirclePos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVert), circleVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 1.0f, 1.0f, 1.0f);
    glBindVertexArray(0);
    
    GLfloat lineVert[] = { 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f };
    
    glGenVertexArrays(1, &vaoLine);
    glBindVertexArray(vaoLine);
    glGenBuffers(1, &vboLinePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboLinePos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVert), lineVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttrib3f(AMC_ATTRIBUTE_COLOR, 1.0f, 1.0f, 1.0f);
    glBindVertexArray(0);
    
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    
    perspectiveProjMat = mat4::identity();
}
-(void) drawRect:(CGRect)rect {
    
}


-(void) layoutSubviews {
    
    GLint width;
    GLint height;
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    [eaglContext renderbufferStorage:GL_RENDERBUFFER fromDrawable: (CAEAGLLayer *)self.layer];
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
    
    //Depth BUffer Regenration!
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Failed to Update FBO!!.");
        glCheckFramebufferStatus(GL_FRAMEBUFFER);
    }
    else {
        printf("FBO Updated Successfully!\n");
    }
    
    
    if(height == 0){
        height = 1;
    }
    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);
    
    GLfloat fWidth = (GLfloat)width;
    GLfloat fHeight = (GLfloat)height;
    
    perspectiveProjMat = perspective(45.0f, fWidth/fHeight, 0.1f, 100.0f);
    
    [self drawView: nil];
}

-(void) drawView: (id) sender {
    [EAGLContext setCurrentContext:eaglContext];
    
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    mat4 modelViewMat;
    mat4 modelviewProjMat;
    
    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    
    glUseProgram(gProgramShaderObj);
    
    modelViewMat = translate(0.0f, 0.0f, -4.0f);
    modelviewProjMat = perspectiveProjMat * modelViewMat;
    
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);
    
    glLineWidth(2.0f);
    glBindVertexArray(vaoTriangle);
    glDrawArrays(GL_LINES, 0, 2);
    glDrawArrays(GL_LINES, 1, 2);
    glDrawArrays(GL_LINES, 2, 2);
    glBindVertexArray(0);
    
    glBindVertexArray(vaoCircle);
    glDrawArrays(GL_POINTS, 0, 6290);
    glBindVertexArray(0);
    
    glBindVertexArray(vaoLine);
    glDrawArrays(GL_LINES, 0, 2);
    glBindVertexArray(0);
    
    glUseProgram(0);
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
}

-(void) startAnimation {
    if(!isAnimating) {
        displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget: self selector:@selector(drawView:)];
        
        [displayLink setPreferredFramesPerSecond: animationFrameInterval];
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        
        isAnimating = YES;
    }
}

-(void) stopAnimation {
    if(isAnimating) {
        [displayLink invalidate];
        displayLink = nil;
        
        isAnimating = NO;
    }
}


-(BOOL) acceptsFirstResponder{
    return(YES);
}

-(void) touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    //TO Start Our Code for accepting the
}

-(void) onSingleTap:(UITapGestureRecognizer *) event {
    
}

-(void) onDoubleTap:(UITapGestureRecognizer *) event {
    
}

-(void) onSwipe:(UISwipeGestureRecognizer *) event {
    [self release];
    exit(0);
}

-(void) onLongPress:(UILongPressGestureRecognizer *) event {
    
}

-(void) dealloc {
    
    //Uninitialize
    if(vboTrianglePos) {
        glDeleteBuffers(1, &vboTrianglePos);
        vboTrianglePos = 0;
    }
    //DeleteVAO
    if(vaoTriangle) {
        glDeleteVertexArrays(1, &vaoTriangle);
        vaoTriangle = 0;
    }
    if(vboCirclePos) {
        glDeleteBuffers(1, &vboCirclePos);
        vboCirclePos = 0;
    }
    //DeleteVAO
    if(vaoCircle) {
        glDeleteVertexArrays(1, &vaoCircle);
        vaoCircle = 0;
    }
    if(vboLinePos) {
        glDeleteBuffers(1, &vboLinePos);
        vboLinePos = 0;
    }
    if(vaoLine) {
        glDeleteVertexArrays(1, &vaoLine);
        vaoLine = 0;
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
    
    if(depthBuffer) {
        glDeleteRenderbuffers(1, &depthBuffer);
        depthBuffer = 0;
    }
    if(colorBuffer){
        glDeleteRenderbuffers(1, &colorBuffer);
        colorBuffer = 0;
    }
    
    if(frameBuffer) {
        glDeleteFramebuffers(1, &frameBuffer);
        frameBuffer = 0;
    }
    
    if([EAGLContext currentContext] == eaglContext) {
        [EAGLContext setCurrentContext:nil];
    }
    
    if(eaglContext) {
        eaglContext = nil;
    }
    
    [super dealloc];
}



@end
