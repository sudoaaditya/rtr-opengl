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
    GLuint vao_Rect, vbo_RectPos;
    GLuint mvpUniform;
    mat4 perspectiveProjMat;
    GLuint iTexSmiley;
    GLuint vbo_RectTex;
    GLuint samplerUniform;
    int iKeyPressed;

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
        iKeyPressed = 0;
        
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
                "in vec2 vTexCoord;" \
                "uniform mat4 u_mvp_matrix;" \
                "out vec2 out_texCoord;"    \
                "void main(void)" \
                "{" \
                "gl_Position =  u_mvp_matrix * vPosition ;" \
                "out_texCoord = vTexCoord;" \
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
                "in vec2 out_texCoord;" \
                "uniform sampler2D u_sampler;" \
                "out vec4 FragColor;" \
                "void main(void)" \
                "{" \
                "FragColor = texture(u_sampler, out_texCoord);" \
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
    glBindAttribLocation(gProgramShaderObj, AMC_ATTRIBUTE_TEXCOORD0, "vTexCoord");
    
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
    samplerUniform = glGetUniformLocation(gProgramShaderObj, "u_sampler");

    
    const GLfloat rectVert[] = {1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f};
    
    glGenVertexArrays(1, &vao_Rect);
    glBindVertexArray(vao_Rect);
    glGenBuffers(1, &vbo_RectPos);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_RectPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectVert), rectVert, GL_STATIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_POSITION, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_POSITION);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &vbo_RectTex);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_RectTex);
    glBufferData(GL_ARRAY_BUFFER, (4*2*sizeof(GLfloat)), NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(AMC_ATTRIBUTE_TEXCOORD0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(AMC_ATTRIBUTE_TEXCOORD0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    iTexSmiley = [self loadTexture:@"Smiley" :@"bmp" ];
    if(iTexSmiley) {
        printf("Smiley Texture Loaded Successfully!\n");
    }
    else {
        printf("Failed to Load Smiley Texture!\n");
    }

    
    perspectiveProjMat = mat4::identity();
}

-(GLuint) loadTexture:(NSString*) texFileName :(NSString*)texFileExt{
    //CreateFileName
    NSString *texFileNameWithPath = [[NSBundle mainBundle] pathForResource:texFileName ofType:texFileExt];
    
    UIImage *bmpImage = [[UIImage alloc] initWithContentsOfFile:texFileNameWithPath];
    
    if(!bmpImage) {
        NSLog(@"Cannot find %@", texFileNameWithPath);
        return(0);
    }
    
    CGImageRef cgImage = bmpImage.CGImage;
    
    int w = (int)CGImageGetWidth(cgImage);
    int h = (int)CGImageGetHeight(cgImage);
    
    CFDataRef imageData = CGDataProviderCopyData(CGImageGetDataProvider(cgImage));
    
    void *pixels = (void *)CFDataGetBytePtr(imageData);
    
    GLuint texHolder;
    
    glGenTextures(1, &texHolder);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, texHolder);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    CFRelease(imageData);
    return(texHolder);
    
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
    GLfloat TexArr[8];
    modelViewMat = mat4::identity();
    modelviewProjMat = mat4::identity();
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    
    glUseProgram(gProgramShaderObj);
    modelViewMat = translate(0.0f, 0.0f, -3.0f);
    
    modelviewProjMat = perspectiveProjMat * modelViewMat;
    
    glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, modelviewProjMat);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, iTexSmiley);
    glUniform1i(samplerUniform, 0);
    glBindVertexArray(vao_Rect);
    if(iKeyPressed == 1) {
        TexArr[0] = 0.5f;
        TexArr[1] = 0.5f;
        TexArr[2] = 0.0f;
        TexArr[3] = 0.5f;
        TexArr[4] = 0.0f;
        TexArr[5] = 0.0f;
        TexArr[6] = 0.5f;
        TexArr[7] = 0.0f;
    }
    else if(iKeyPressed == 2) {
        TexArr[0] = 1.0f;
        TexArr[1] = 1.0f;
        TexArr[2] = 0.0f;
        TexArr[3] = 1.0f;
        TexArr[4] = 0.0f;
        TexArr[5] = 0.0f;
        TexArr[6] = 1.0f;
        TexArr[7] = 0.0f;
    }
    else if(iKeyPressed == 3) {
        TexArr[0] = 2.0f;
        TexArr[1] = 2.0f;
        TexArr[2] = 0.0f;
        TexArr[3] = 2.0f;
        TexArr[4] = 0.0f;
        TexArr[5] = 0.0f;
        TexArr[6] = 2.0f;
        TexArr[7] = 0.0f;
    }
    else if(iKeyPressed == 4) {
        TexArr[0] = 0.5f;
        TexArr[1] = 0.5f;
        TexArr[2] = 0.5f;
        TexArr[3] = 0.5f;
        TexArr[4] = 0.5f;
        TexArr[5] = 0.5f;
        TexArr[6] = 0.5f;
        TexArr[7] = 0.5f;
    }
    
    if(iKeyPressed != 0){
        glBindBuffer(GL_ARRAY_BUFFER, vbo_RectTex);
        glBufferData(GL_ARRAY_BUFFER, sizeof(TexArr), TexArr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
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
    iKeyPressed++;
    if(iKeyPressed >= 5){
        iKeyPressed = 1;
    }
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
    if(iTexSmiley) {
        glDeleteTextures(1, &iTexSmiley);
        iTexSmiley = 0;
    }
    
    //VAo & vbo./c
    if(vbo_RectTex) {
        glDeleteBuffers(1, &vbo_RectTex);
        vbo_RectTex = 0;
    }
    if(vbo_RectPos) {
        glDeleteBuffers(1, &vbo_RectPos);
        vbo_RectPos = 0;
    }
    if(vao_Rect) {
        glDeleteVertexArrays(1, &vao_Rect);
        vao_Rect = 0;
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
