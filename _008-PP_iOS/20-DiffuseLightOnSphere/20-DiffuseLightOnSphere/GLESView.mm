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
#import "sphere.h"

using namespace vmath;

//Global Arrays!
GLfloat lightPos[] = {0.0f, 0.0f, 2.0f, 1.0f};

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
    GLuint vaoSphere, vboSpherePos, vboSphereNorm, vboSphereEle;
    GLuint mvUniform;
    GLuint projMatUniform;
    GLuint ldUniform;
    GLuint kdUniform;
    GLuint lightPosUniform;
    GLuint lPressedUniform;
    mat4 perspectiveProjMat;
    
    float sphereVertices[1146];
    float shpereNormals[1146];
    float sphereTextures[764];
    unsigned short shpereElements[2280];
    
    int iNumEle;
    int iNumVert;
    
    bool bLighting;
    bool bAnimation;
    GLfloat fRotAngle;

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
        bLighting = false;
        bAnimation = false;
        fRotAngle = 0.0f;
        
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
    [self addGestureRecognizer:doubleTapRecog];
    
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
    
    //OPENGL!
    //Shader Code : Define Vertex Shader Object
    gVertexShaderObj = glCreateShader(GL_VERTEX_SHADER);
    
    // Write Vertex Shader Code!.
    const GLchar *vertexShaderSourceCode =
                "#version 300 es" \
                "\n" \
                "precision mediump int;" \
                "in vec4 vPosition;" \
                "in vec3 vNormal;" \
                "uniform mat4 u_mvMatrix;" \
                "uniform mat4 u_projMatrix;" \
                "uniform vec3 u_ld;" \
                "uniform vec3 u_kd;" \
                "uniform int u_lKeyPressed;" \
                "uniform vec4 u_lightPosition;" \
                "out vec3 diffuseColor;" \
                "void main(void) {" \
                "if(u_lKeyPressed == 1) {" \
                "   vec4 eyeCoords = u_mvMatrix * vPosition;" \
                "   mat3 normMatrix = mat3(transpose(inverse(u_mvMatrix)));" \
                "   vec3 tNorm = normalize(normMatrix * vNormal);" \
                "   vec3 s = vec3(u_lightPosition - eyeCoords);" \
                "   diffuseColor = u_ld * u_kd * max(dot(s, tNorm), 0.0);" \
                "}" \
                "gl_Position = u_projMatrix * u_mvMatrix * vPosition;" \
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
                "precision mediump int;" \
                "out vec4 FragColor;" \
                "uniform int u_lKeyPressed;" \
                "in vec3 diffuseColor;" \
                "void main(void) {" \
                "if(u_lKeyPressed == 1) {" \
                "   FragColor = vec4(diffuseColor, 1.0);" \
                "}" \
                "else {" \
                "   FragColor = vec4(1.0, 1.0, 1.0, 1.0);" \
                "}"\
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
    glBindAttribLocation(gProgramShaderObj, AMC_ATTRIBUTE_NORMAL, "vNormal");

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
    
    mvUniform = glGetUniformLocation(gProgramShaderObj, "u_mvMatrix");
    projMatUniform = glGetUniformLocation(gProgramShaderObj, "u_projMatrix");
    ldUniform = glGetUniformLocation(gProgramShaderObj, "u_ld");
    kdUniform = glGetUniformLocation(gProgramShaderObj, "u_kd");
    lPressedUniform = glGetUniformLocation(gProgramShaderObj, "u_lKeyPressed");
    lightPosUniform = glGetUniformLocation(gProgramShaderObj, "u_lightPosition");
    
    //Get Sphere Data!.
    getSphereVertexData(sphereVertices,shpereNormals,sphereTextures,shpereElements);
    iNumVert = getNumberOfSphereVertices();
    iNumEle = getNumberOfSphereElements();
    
    //Let There BE Sphere!.
    glGenVertexArrays(1, &vaoSphere);
    glBindVertexArray(vaoSphere);
    
    glGenBuffers(1, &vboSpherePos);
    glBindBuffer(GL_ARRAY_BUFFER, vboSpherePos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &vboSphereNorm);
    glBindBuffer(GL_ARRAY_BUFFER, vboSphereNorm);
    glBufferData(GL_ARRAY_BUFFER, sizeof(shpereNormals), shpereNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &vboSphereEle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereEle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(shpereElements), shpereElements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
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
    
    //Declare MAtrices
    mat4 modelViewMat;
    mat4 modelviewProjMat;
    mat4 translateMat;
    mat4 rotateMat;
    
    //Initialization of matrices
    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();
    translateMat = mat4::identity();
    rotateMat = mat4::identity();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    
    glUseProgram(gProgramShaderObj);
    
    translateMat = translate(0.0f, 0.0f, -3.0f);
    modelViewMat *= translateMat;
    rotateMat = rotate(fRotAngle, 1.0f, 0.0f, 0.0f);
    rotateMat *= rotate(fRotAngle, 0.0f, 1.0f, 0.0f);
    rotateMat *= rotate(fRotAngle, 0.0f, 0.0f, 1.0f);
    modelViewMat *= rotateMat;
    glUniformMatrix4fv(mvUniform, 1, GL_FALSE, modelViewMat);
    glUniformMatrix4fv(projMatUniform, 1, GL_FALSE, perspectiveProjMat);
    
    if(bLighting) {
        glUniform1i(lPressedUniform, 1);
        glUniform3f(ldUniform, 1.0f, 1.0f, 1.0f);
        glUniform3f(kdUniform, 0.5f, 0.5f, 0.5f);
        glUniform4fv(lightPosUniform, 1, lightPos);
    }
    else {
        glUniform1i(lPressedUniform, 0);
    }
    
    glBindVertexArray(vaoSphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereEle);
    glDrawElements(GL_TRIANGLES, iNumEle, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    //Unuse Program.
    glUseProgram(0);

    if(bAnimation == true) {
        [self update];
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
}

-(void) update {
    
    fRotAngle += 0.2f;
    if(fRotAngle >= 360.0f) {
        fRotAngle = 0.0f;
    }
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
    bLighting = bLighting == true ? false : true;
}

-(void) onDoubleTap:(UITapGestureRecognizer *) event {
    bAnimation = bAnimation == true ? false : true;
}

-(void) onSwipe:(UISwipeGestureRecognizer *) event {
    [self release];
    exit(0);
}

-(void) onLongPress:(UILongPressGestureRecognizer *) event {
    
}

-(void) dealloc {
    
    //Uninitialize
    glUseProgram(gProgramShaderObj);
    if(vboSphereEle) {
        glDeleteBuffers(1, &vboSphereEle);
        vboSphereEle = 0;
    }
    if(vboSphereNorm) {
        glDeleteBuffers(1, &vboSphereNorm);
        vboSphereNorm = 0;
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
