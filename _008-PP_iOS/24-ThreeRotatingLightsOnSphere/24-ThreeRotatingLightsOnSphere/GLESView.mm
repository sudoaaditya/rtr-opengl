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


enum {
    AMC_ATTRIBUTE_POSITION = 0,
    AMC_ATTRIBUTE_COLOR,
    AMC_ATTRIBUTE_NORMAL,
    AMC_ATTRIBUTE_TEXCOORD0
};

//Light Parameters!.
//Red Light
GLfloat lightAmbientZero[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuseZero[] = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightSpecularZero[] = {1.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightPositionZero[] = {0.0f, 0.0f, 0.0f, 1.0f};
//Green
GLfloat lightAmbientOne[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuseOne[] = {0.0f, 1.0f, 0.0f, 1.0f};
GLfloat lightSpecularOne[] = {0.0f, 1.0f, 0.0f, 1.0f};
GLfloat lightPositionOne[] = {0.0f, 0.0f, 0.0f, 1.0f};
//Blue
GLfloat lightAmbientTwo[] = {0.0f, 0.0f, 0.0f, 1.0f};
GLfloat lightDiffuseTwo[] = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightSpecularTwo[] = {0.0f, 0.0f, 1.0f, 1.0f};
GLfloat lightPositionTwo[] = {0.0f, 0.0f, 0.0f, 1.0f};

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
    GLuint vaoSphere, vboSpherePos, vboSphereNorm, vboSphereEle;
    mat4 perspectiveProjMat;
    
    //Uniforms
    GLuint modelUniform;
    GLuint viewUniform;
    GLuint projUnifrom;
    GLuint kaUniform;
    GLuint kdUniform;
    GLuint ksUniform;
    GLuint matShineUniform;
    GLuint lPressedUnifrom;
    //Light)
    GLuint laZeroUniform;
    GLuint ldZeroUniform;
    GLuint lsZeroUniform;
    GLuint lightZeroPosUniform;
    //Light 1
    GLuint laOneUniform;
    GLuint ldOneUniform;
    GLuint lsOneUniform;
    GLuint lightOnePosUniform;
    //Light 2
    GLuint laTwoUniform;
    GLuint ldTwoUniform;
    GLuint lsTwoUniform;
    GLuint lightTwoPosUniform;
    
    
    float sphereVertices[1146];
    float shpereNormals[1146];
    float sphereTextures[764];
    unsigned short shpereElements[2280];
    
    int iNumEle;
    int iNumVert;
    
    bool bLighting;
    bool bAnimation;
    GLfloat lightAngle, Radius;

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
        lightAngle = 0.0f;
        Radius = 200.0f;
        
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
                "uniform vec4 u_lightPosZero;" \
                "uniform vec4 u_lightPosOne;" \
                "uniform vec4 u_lightPosTwo;" \
                "out vec3 t_normal;"    \
                "out vec3 viewerVector;" \
                "out vec3 lightDirectionZero;" \
                "out vec3 lightDirectionOne;" \
                "out vec3 lightDirectionTwo;" \
                "void main(void) {" \
                "   if(u_lPressed == 1) {" \
                "       vec4 eyeCoords = u_viewMat * u_modelMat * vPosition;" \
                "       t_normal = mat3(u_viewMat * u_modelMat) * vNormal;" \
                "       lightDirectionZero = vec3(u_lightPosZero - eyeCoords);" \
                "       lightDirectionOne = vec3(u_lightPosOne - eyeCoords);" \
                "       lightDirectionTwo = vec3(u_lightPosTwo - eyeCoords);" \
                "       viewerVector = vec3(-eyeCoords.xyz);" \
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
                "uniform vec3 u_laOne;" \
                "uniform vec3 u_ldOne;" \
                "uniform vec3 u_lsOne;" \
                "uniform vec3 u_laZero;" \
                "uniform vec3 u_ldZero;" \
                "uniform vec3 u_lsZero;" \
                "uniform vec3 u_laTwo;" \
                "uniform vec3 u_ldTwo;" \
                "uniform vec3 u_lsTwo;" \
                "uniform vec3 u_ka;" \
                "uniform vec3 u_kd;" \
                "uniform vec3 u_ks;" \
                "uniform float u_matShine;" \
                "uniform int u_lPressed;" \
                "out vec4 FragColor;" \
                "in vec3 t_normal;" \
                "in vec3 lightDirectionZero;" \
                "in vec3 lightDirectionOne;" \
                "in vec3 lightDirectionTwo;" \
                "in vec3 viewerVector;" \
                "void main(void) {" \
                "   vec3 phong_ads_light;" \
                "   if(u_lPressed == 1) {" \
                "       vec3 n_tNormal = normalize(t_normal);" \
                "       vec3 n_lightDirectionZero = normalize(lightDirectionZero);" \
                "       vec3 n_lightDirectionOne = normalize(lightDirectionOne);" \
                "       vec3 n_lightDirectionTwo = normalize(lightDirectionTwo);" \
                "       vec3 n_viewerVec = normalize(viewerVector);" \
    
                "       float tn_dot_ld_zero = max(dot(n_lightDirectionZero, n_tNormal), 0.0);" \
                "       vec3 reflectionVectorZero = reflect(-n_lightDirectionZero, n_tNormal);" \
                "       vec3 ambientZero = u_laZero * u_ka;" \
                "       vec3 diffuseZero = u_ldZero * u_kd * tn_dot_ld_zero;" \
                "       vec3 specularZero= u_lsZero * u_ks * pow(max(dot(reflectionVectorZero, n_viewerVec), 0.0), u_matShine);" \
    
                "       float tn_dot_ld_one = max(dot(n_lightDirectionOne, n_tNormal), 0.0);" \
                "       vec3 reflectionVectorOne = reflect(-n_lightDirectionOne, n_tNormal);" \
                "       vec3 ambientOne = u_laOne * u_ka;" \
                "       vec3 diffuseOne = u_ldOne * u_kd * tn_dot_ld_one;" \
                "       vec3 specularOne= u_lsOne * u_ks * pow(max(dot(reflectionVectorOne, n_viewerVec), 0.0), u_matShine);" \
    
                "       float tn_dot_ld_two = max(dot(n_lightDirectionTwo, n_tNormal), 0.0);" \
                "       vec3 reflectionVectorTwo = reflect(-n_lightDirectionTwo, n_tNormal);" \
                "       vec3 ambientTwo = u_laTwo * u_ka;" \
                "       vec3 diffuseTwo = u_ldTwo * u_kd * tn_dot_ld_two;" \
                "       vec3 specularTwo= u_lsTwo * u_ks * pow(max(dot(reflectionVectorTwo, n_viewerVec), 0.0), u_matShine);" \
    
                "       phong_ads_light = ambientZero + ambientOne + ambientTwo + diffuseOne + diffuseZero + diffuseTwo + specularOne + specularZero + specularTwo;" \
                "   }" \
                "   else {" \
                "       phong_ads_light = vec3(1.0, 1.0, 1.0);" \
                "   }" \
                "   FragColor = vec4(phong_ads_light, 1.0);" \
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
    kaUniform = glGetUniformLocation(gProgramShaderObj, "u_ka");
    kdUniform = glGetUniformLocation(gProgramShaderObj, "u_kd");
    ksUniform = glGetUniformLocation(gProgramShaderObj, "u_ks");
    matShineUniform = glGetUniformLocation(gProgramShaderObj, "u_matShine");
    lPressedUnifrom = glGetUniformLocation(gProgramShaderObj, "u_lPressed");
    laZeroUniform = glGetUniformLocation(gProgramShaderObj, "u_laZero");
    ldZeroUniform = glGetUniformLocation(gProgramShaderObj, "u_ldZero");
    lsZeroUniform = glGetUniformLocation(gProgramShaderObj, "u_lsZero");
    lightZeroPosUniform = glGetUniformLocation(gProgramShaderObj, "u_lightPosZero");
    laOneUniform = glGetUniformLocation(gProgramShaderObj, "u_laOne");
    ldOneUniform = glGetUniformLocation(gProgramShaderObj, "u_ldOne");
    lsOneUniform = glGetUniformLocation(gProgramShaderObj, "u_lsOne");
    lightOnePosUniform = glGetUniformLocation(gProgramShaderObj, "u_lightPosOne");
    laTwoUniform = glGetUniformLocation(gProgramShaderObj, "u_laTwo");
    ldTwoUniform = glGetUniformLocation(gProgramShaderObj, "u_ldTwo");
    lsTwoUniform = glGetUniformLocation(gProgramShaderObj, "u_lsTwo");
    lightTwoPosUniform = glGetUniformLocation(gProgramShaderObj, "u_lightPosTwo");

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
    mat4 modelMat;
    mat4 viewMat;
    
    //Initialization of matrices
    modelMat = mat4::identity();
    viewMat = mat4::identity();

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    
    glUseProgram(gProgramShaderObj);
    
    modelMat = translate(0.0f, 0.0f, -2.0f);
    glUniformMatrix4fv(modelUniform, 1, GL_FALSE, modelMat);
    glUniformMatrix4fv(viewUniform, 1, GL_FALSE, viewMat);
    glUniformMatrix4fv(projUnifrom, 1, GL_FALSE, perspectiveProjMat);
    
    if(bLighting == true) {
        glUniform3fv(kaUniform, 1, materialAmbient);
        glUniform3fv(kdUniform, 1, materialDiffuse);
        glUniform3fv(ksUniform, 1, materialSpecular);
        glUniform1f(matShineUniform, materialShininess);
        
        glUniform3fv(laZeroUniform, 1, lightAmbientZero);
        glUniform3fv(ldZeroUniform, 1, lightDiffuseZero);
        glUniform3fv(lsZeroUniform, 1, lightSpecularZero);
        //ALong X. so change Y & z
        lightPositionZero[1] = Radius * (GLfloat)sinf(lightAngle);
        lightPositionZero[2] = Radius * (GLfloat)cosf(lightAngle);
        glUniform4fv(lightZeroPosUniform, 1, lightPositionZero);
        
        glUniform3fv(laOneUniform, 1, lightAmbientOne);
        glUniform3fv(ldOneUniform, 1, lightDiffuseOne);
        glUniform3fv(lsOneUniform, 1, lightSpecularOne);
        //ALong Y. so change Z & X
        lightPositionOne[0] = Radius * (GLfloat)sinf(lightAngle);
        lightPositionOne[2] = Radius * (GLfloat)cosf(lightAngle);
        glUniform4fv(lightOnePosUniform, 1, lightPositionOne);
        
        glUniform3fv(laTwoUniform, 1, lightAmbientTwo);
        glUniform3fv(ldTwoUniform, 1, lightDiffuseTwo);
        glUniform3fv(lsTwoUniform, 1, lightSpecularTwo);
        //ALong Z. so change X& Y
        lightPositionTwo[0] = Radius * (GLfloat)sinf(lightAngle);
        lightPositionTwo[1] = Radius * (GLfloat)cosf(lightAngle);
        glUniform4fv(lightTwoPosUniform, 1, lightPositionTwo);
        
        glUniform1i(lPressedUnifrom, 1);
    }
    else {
        glUniform1i(lPressedUnifrom, 0);
    }
    
    glBindVertexArray(vaoSphere);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboSphereEle);
    glDrawElements(GL_TRIANGLES, iNumEle, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
    
    //Unuse Program.
    glUseProgram(0);
    
    if(bAnimation) {
        [self update];
    }

    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
}

-(void) update {
    lightAngle += 0.05f;
    if(lightAngle >= 360.0f) {
        lightAngle = 0.0f;
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
