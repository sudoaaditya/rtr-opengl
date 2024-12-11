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
        bI1Reached = false;
        bI2Reached = false;
        bAReached = false;
        bDReached = false;
        bNReached = false;
        fTransSpeed = 0.003f;

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
                "gl_PointSize = 1.3f;" \
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
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    
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
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    [eaglContext presentRenderbuffer:GL_RENDERBUFFER];
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
    
    glDrawArrays(GL_POINTS, 0, 315);
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
