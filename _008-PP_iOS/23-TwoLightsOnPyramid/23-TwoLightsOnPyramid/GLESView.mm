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



//Light Parameters Right Pars:: Red Light!.
GLfloat lightAmbientRed[] = {0.0f, 0.0f, 0.0f, 0.0f};
GLfloat lightDiffuseRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightSpecularRed[] = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightPositionRed[] = {2.0f, 0.0f, 0.0f, 1.0f};
//Light For Left Pars:: Blue Light
GLfloat lightAmbientBlue[] = {0.0f, 0.0f, 0.0f, 0.0f};
GLfloat lightDiffuseBlue[] = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightSpecularBlue[] = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightPositionBlue[] = {-2.0f, 0.0f, 0.0f, 1.0f};

GLfloat materialAmbient[] = {0.0f, 0.0f, 0.0f, 0.0f};
GLfloat materialDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat materialShininess = 128.0f;




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
    mat4 perspectiveProjMat;
    
    //Uniforms
    GLuint modelUniform;
    GLuint viewUniform;
    GLuint projUnifrom;
    
    GLuint laOneUniform;
    GLuint ldOneUniform;
    GLuint lsOneUniform;
    GLuint lightPosOneUniform;
    
    GLuint laTwoUniform;
    GLuint ldTwoUniform;
    GLuint lsTwoUniform;
    GLuint lightPosTwoUniform;
    
    GLuint kaUniform;
    GLuint kdUniform;
    GLuint ksUniform;
    GLuint matShineUniform;
    GLuint lPressedUnifrom;
    
    //Animations Vars
    GLfloat fRotAngle;
    bool bLighting;
    GLuint vaoPyramid, vboPyramidPos, vboPyramidNormal;

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
                "precision highp float;" \
                "precision lowp int;" \
                "in vec4 vPosition;" \
                "in vec3 vNormal;" \
                "uniform mat4 u_modelMat;" \
                "uniform mat4 u_viewMat;" \
                "uniform mat4 u_projMat;" \
                "uniform int u_lPressed;" \
                "uniform vec3 u_laOne;" \
                "uniform vec3 u_ldOne;" \
                "uniform vec3 u_lsOne;" \
                "uniform vec4 u_lightPosOne;" \
                "uniform vec3 u_laTwo;" \
                "uniform vec3 u_ldTwo;" \
                "uniform vec3 u_lsTwo;" \
                "uniform vec4 u_lightPosTwo;" \
                "uniform vec3 u_ka;" \
                "uniform vec3 u_kd;" \
                "uniform vec3 u_ks;" \
                "uniform float u_matShine;" \
                "out vec3 phong_ads_light;" \
                "void main(void) {" \
                "   if(u_lPressed == 1) {" \
                "       vec4 eyeCoords = u_viewMat * u_modelMat * vPosition;" \
                "       vec3 t_normal = normalize(mat3(u_viewMat * u_modelMat) * vNormal);" \
                "       vec3 lightDirectionOne = normalize(vec3(u_lightPosOne - eyeCoords));" \
                "       float tn_dot_ldOne = max(dot(lightDirectionOne, t_normal), 0.0);" \
                "       vec3 reflectionVectorOne = reflect(-lightDirectionOne, t_normal);" \
                "       vec3 lightDirectionTwo = normalize(vec3(u_lightPosTwo - eyeCoords));" \
                "       float tn_dot_ldTwo = max(dot(lightDirectionTwo, t_normal), 0.0);" \
                "       vec3 reflectionVectorTwo = reflect(-lightDirectionTwo, t_normal);" \
                "       vec3 viewerVector = normalize(vec3(-eyeCoords.xyz));" \
                "       vec3 ambientOne = u_laOne * u_ka;" \
                "       vec3 diffuseOne = u_ldOne * u_kd * tn_dot_ldOne;" \
                "       vec3 specularOne = u_lsOne * u_ks * pow(max(dot(reflectionVectorOne, viewerVector), 0.0), u_matShine);" \
                "       vec3 ambientTwo = u_laTwo * u_ka;" \
                "       vec3 diffuseTwo = u_ldTwo * u_kd * tn_dot_ldTwo;" \
                "       vec3 specularTwo = u_lsTwo * u_ks * pow(max(dot(reflectionVectorTwo, viewerVector), 0.0), u_matShine);" \
                "       phong_ads_light = ambientOne + ambientTwo +diffuseOne + diffuseTwo +specularOne + specularTwo;" \
                "   }" \
                "   else {" \
                "       phong_ads_light = vec3(1.0, 1.0, 1.0);" \
                "   }" \
                "   gl_Position = u_projMat * u_viewMat * u_modelMat * vPosition;"  \
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
                "precision highp float;" \
                "precision lowp int;" \
                "out vec4 FragColor;" \
                "in vec3 phong_ads_light;"
                "void main(void)" \
                "{" \
                "FragColor = vec4(phong_ads_light, 1.0);" \
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
    
    modelUniform = glGetUniformLocation(gProgramShaderObj, "u_modelMat");
    viewUniform = glGetUniformLocation(gProgramShaderObj, "u_viewMat");
    projUnifrom = glGetUniformLocation(gProgramShaderObj, "u_projMat");
    laOneUniform = glGetUniformLocation(gProgramShaderObj, "u_laOne");
    ldOneUniform = glGetUniformLocation(gProgramShaderObj, "u_ldOne");
    lsOneUniform = glGetUniformLocation(gProgramShaderObj, "u_ls");
    lightPosOneUniform = glGetUniformLocation(gProgramShaderObj, "u_lightPosOne");
    
    laTwoUniform = glGetUniformLocation(gProgramShaderObj, "u_laTwo");
    ldTwoUniform = glGetUniformLocation(gProgramShaderObj, "u_ldTwo");
    lsTwoUniform = glGetUniformLocation(gProgramShaderObj, "u_lsTwo");
    lightPosTwoUniform = glGetUniformLocation(gProgramShaderObj, "u_lightPosTwo");
    
    kaUniform = glGetUniformLocation(gProgramShaderObj, "u_ka");
    kdUniform = glGetUniformLocation(gProgramShaderObj, "u_kd");
    ksUniform = glGetUniformLocation(gProgramShaderObj, "u_ks");
    matShineUniform = glGetUniformLocation(gProgramShaderObj, "u_matShine");
    lPressedUnifrom = glGetUniformLocation(gProgramShaderObj, "u_lPressed");
    
    //Create Array of vertices to be sent over GPU
    const GLfloat pyramidVert[] = { 0.0f, 1.0f, 0.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
        0.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,
        0.0f, 1.0f, 0.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f};
    
    const GLfloat pyramidNormals[] = {0.0f, 0.447214f, 0.894427f, 0.0f, 0.447214f, 0.894427f, 0.0f, 0.447214f, 0.894427f,
        0.894427f, 0.447214f, 0.0f, 0.894427f, 0.447214f, 0.0f, 0.894427f, 0.447214f, 0.0f,
        0.0f, 0.447214f, -0.894427f, 0.0f, 0.447214f, -0.894427f, 0.0f, 0.447214f, -0.894427f,
        -0.894427f, 0.447214f, 0.0f, -0.894427f, 0.447214f, 0.0f, -0.894427f, 0.447214f, 0.0f
    };
    
    
    glGenVertexArrays(1, &vaoPyramid);
    glBindVertexArray(vaoPyramid);
    
    glGenBuffers(1, &vboPyramidPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPyramidPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVert), pyramidVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &vboPyramidNormal);
    glBindBuffer(GL_ARRAY_BUFFER, vboPyramidNormal);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), pyramidNormals, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_NORMAL);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
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
    mat4 modelMat;
    mat4 viewMat;
    mat4 rotateMat;
    mat4 translateMat;
    
    //Initialization of matrices
    modelMat = mat4::identity();
    viewMat = mat4::identity();
    rotateMat = mat4::identity();
    translateMat = mat4::identity();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    
    glUseProgram(gProgramShaderObj);
    
    translateMat = translate(0.0f, 0.0f, -5.0f);
    rotateMat= rotate(fRotAngle, 0.0f, 1.0f, 0.0f);
    modelMat = translateMat * rotateMat;
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMat);
    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMat);
    glUniformMatrix4fv(projUnifrom, 1, GL_FALSE, perspectiveProjMat);
    
    if(bLighting == true) {
        glUniform3fv(laOneUniform, 1, lightAmbientRed);
        glUniform3fv(ldOneUniform, 1, lightDiffuseRed);
        glUniform3fv(lsOneUniform, 1, lightSpecularRed);
        glUniform4fv(lightPosOneUniform, 1, lightPositionRed);
        
        glUniform3fv(laTwoUniform, 1, lightAmbientBlue);
        glUniform3fv(ldTwoUniform, 1, lightDiffuseBlue);
        glUniform3fv(lsTwoUniform, 1, lightSpecularRed);
        glUniform4fv(lightPosTwoUniform, 1, lightPositionBlue);
        
        glUniform3fv(kaUniform, 1, materialAmbient);
        glUniform3fv(kdUniform, 1, materialDiffuse);
        glUniform3fv(ksUniform, 1, materialSpecular);
        glUniform1f(matShineUniform, materialShininess);
        glUniform1i(lPressedUnifrom, 1);
    }
    else {
        glUniform1i(lPressedUnifrom, 0);
    }
    
    glBindVertexArray(vaoPyramid);
    glDrawArrays(GL_TRIANGLES, 0, 12);
    glBindVertexArray(0);
    
    glUseProgram(0);
    
    [self update];
    //Unuse Program.
    glUseProgram(0);

    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
}

-(void) update {
    
    fRotAngle += 0.8f;
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
    if(vboPyramidPos) {
        glDeleteBuffers(1, &vboPyramidPos);
        vboPyramidPos = 0;
    }
    if(vboPyramidNormal) {
        glDeleteBuffers(1, &vboPyramidNormal);
        vboPyramidNormal = 0;
    }
    if(vaoPyramid) {
        glDeleteVertexArrays(1, &vaoPyramid);
        vaoPyramid = 0;
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
