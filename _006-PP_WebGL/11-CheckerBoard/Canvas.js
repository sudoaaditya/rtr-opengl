//Global Vars!.
var canvas = null;
var gl = null;
var bFullScreen = null;
var canvasOriginalWidth = null;
var canvasOriginalHeight =  null;

const WebGLMacros = {
    AMC_ATTRIBUTE_POSITION:0,
    AMC_ATTRIBUTE_COLOR:1,
    AMC_ATTRIBUTE_NORMAL:2,
    AMC_ATTRIBUTE_TEXCOORD0:3
}

var vertexShaderObj;
var fragmentShaderObj;
var shaderProgObj;

var vaoRectangle;
var vboRectanglePos;
var vboRectangleTex;
var mvpUniform;
var samplerUnifrom;
var perspectiveProjMat;

//Checker Vars
var texChecker = 0;
const checkBoardWidth = 64;
const checkBoardHeight = 64;
var checkerBoardData = [checkBoardHeight * checkBoardWidth * 4];


//Request Animation Frame!
var requestAnimationFrame = 
    window.requestAnimationFrame ||
	window.webkitRequestAnimationFrame ||
	window.mozRequestAnimationFrame ||
	window.oRequestAnimationFrame ||
	window.msRequestAnimationFrame ||
	null;

const main = function() {
    //Acquire canvas!
    canvas = document.getElementById("AMC");
    if(!canvas){
        console.log("Failed To Acquire Canvas Element!..");
    }
    else {
        console.log("Canvas Element Acquired Successfully!..");
    }

    canvasOriginalWidth = canvas.width;
    canvasOriginalHeight = canvas.height;

    //Add Event Listener!.
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseEvent, false);
    window.addEventListener("resize", resize, false);

    //Init.
    initialize();

    resize();
    display();
}

const toggleFullScreen = function(){
    //code/
    var fullScreenEelemet = document.fullscreenElement || document.webkitFullscrenElement ||
                            document.mozFullScreenElement || document.msFullscreenElement || null;

    if(fullScreenEelemet == null) {
        if(canvas.requestFullscreen){
            canvas.requestFullscreen();
        }
        else if(canvas.mozRequestFullScreen) {
            canvas.mozRequestFullScreen();
        }
        else if(canvas.webkitRequestFullscreen) {
            canvas.webkitRequestFullscreen();
        }
        else if(canvas.msRequestFullscreen) {
            canvas.msRequestFullscreen();
        }

        bFullScreen = true;
    }
    else {
        if(document.exitFullscreen) {
            document.exitFullscreen();
        }
        else if(document.mozCancelFullScreen) {
            document.mozCancelFullScreen();
        }
        else if(document.webkitExitFullscreen) {
            document.webkitExitFullscreen();
        }
        else if(document.msExitFullscreen) {
            document.msExitFullscreen();
        }
        bFullScreen = false;
    }
}

const initialize = function() {
    //Code!.
    gl = canvas.getContext("webgl2");
    if(gl == null){
        console.log("Failed to get WebGL2 Context!");
    }
    else {
        console.log("Successfully Acquired the WebGL2 Context!!");
    }

    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    //Vertex Shader; 
    var vertexShaderSource = 
            "#version 300 es" +     //OpenGL ES 300 vaprun 2.0 WbeGL SO!
            "\n" +
            "in vec4 vPosition;" +
            "in vec2 vTexCoord;" +
            "uniform mat4 u_mvp_matrix;" +
            "out vec2 out_texCoord;" +
            "void main(void) {" +
            "   gl_Position = u_mvp_matrix * vPosition;" +
            "   out_texCoord = vTexCoord;" +
            "}";
    
    vertexShaderObj = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObj, vertexShaderSource);
    gl.compileShader(vertexShaderObj);
    if(gl.getShaderParameter(vertexShaderObj, gl.COMPILE_STATUS) == false) {
        var error = gl.getShaderInfoLog(vertexShaderObj);
        console.log(error)
        if(error.lenght > 0){
            alert(error);
            uninitialize();
        }
    }
    else {
        console.log("Vertex Shader Compiled Successfully!");
    }

    var fragmentShaderSource = 
            "#version 300 es" +
            "\n" +
            "precision highp float;" +
            "in vec2 out_texCoord;" +
            "uniform highp sampler2D u_sampler;" +
            "out vec4 FragColor;" +
            "void main(void) {" +
            "   FragColor = texture(u_sampler, out_texCoord);" +
            "}";

    fragmentShaderObj = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObj, fragmentShaderSource);
    gl.compileShader(fragmentShaderObj);
    if(gl.getShaderParameter(fragmentShaderObj, gl.COMPILE_STATUS) == false){
        var error = gl.getShaderInfoLog(fragmentShaderObj);
        if(error.lenght > 0){
            alert(error);
            uninitialize();
        }
    }
    else {
        console.log("Fragment Shader Compiled Successfully!");
    }

    //Shader Prog!.
    shaderProgObj = gl.createProgram();
    gl.attachShader(shaderProgObj, vertexShaderObj);
    gl.attachShader(shaderProgObj, fragmentShaderObj);

    //Pre-Link!
    gl.bindAttribLocation(shaderProgObj, WebGLMacros.AMC_ATTRIBUTE_POSITION, "vPosition");
    gl.bindAttribLocation(shaderProgObj, WebGLMacros.AMC_ATTRIBUTE_TEXCOORD0, "vTexCoord");

    gl.linkProgram(shaderProgObj);
    if(gl.getProgramParameter(shaderProgObj, gl.LINK_STATUS) == false) {
        var error = gl.getProgramInfoLog(shaderProgObj)
        if(error.lenght > 0){
            alert(error);
            uninitialize();
        }
    }
    else {
        console.log("Shader Program Linked Successfully!");
    }

    mvpUniform = gl.getUniformLocation(shaderProgObj, "u_mvp_matrix");
    samplerUnifrom = gl.getUniformLocation(shaderProgObj, "u_sampler");

    var rectTexCoords = new Float32Array([
        0.0,  0.0,
        0.0, 1.0,
        1.0, 1.0,
        1.0, 0.0
    ])


    vaoRectangle = gl.createVertexArray();
    gl.bindVertexArray(vaoRectangle);
    vboRectanglePos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboRectanglePos);
    gl.bufferData(gl.ARRAY_BUFFER, (4 * 4 * 3), gl.DYNAMIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vboRectangleTex = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboRectangleTex);
    gl.bufferData(gl.ARRAY_BUFFER, rectTexCoords, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD0, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD0);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    //Load Textures Call Here
    texChecker = loadTexture();
    console.log(texChecker);

    //Set Color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjMat = mat4.create();

}

const makeCheckerIamge = function() {
    var i, j, c;

    for(i = 0; i < checkBoardHeight; i++) {
        for(j = 0; j < checkBoardWidth; j++) {

            c = ((i & 8) ^ (j & 8)) * 255;

            checkerBoardData[(i * 64 + j) * 4 + 0] = c;
            checkerBoardData[(i * 64 + j) * 4 + 1] = c;
            checkerBoardData[(i * 64 + j) * 4 + 2] = c;
            checkerBoardData[(i * 64 + j) * 4 + 3] = 0xff;//255; 
        }
    }
}

const loadTexture = function(fileName) {
    var texHolder = 0;

    makeCheckerIamge();

    console.log(checkerBoardData);
    
    
    texHolder = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, texHolder);
    gl.pixelStorei(gl.UNPACK_ALIGNMENT, 1);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, checkBoardWidth, checkBoardHeight, 0, gl.RGBA, gl.UNSIGNED_BYTE, new Uint8Array(checkerBoardData));

    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.bindTexture(gl.TEXTURE_2D, null);

    return(texHolder);
}

const resize = function() {
    if(bFullScreen == true){
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    else {
        canvas.width = canvasOriginalWidth;
        canvas.height = canvasOriginalHeight;
    }

    gl.viewport(0, 0, canvas.width, canvas.height);

    mat4.perspective(perspectiveProjMat, 45.0, parseFloat(canvas.width)/parseFloat(canvas.height), 0.1, 100.0);
}

const display = function() {
    var rectVertArr = [];
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(shaderProgObj);

    var modelViewMat = mat4.create();
    var modelViewProjMat = mat4.create();

    mat4.translate(modelViewMat, modelViewMat, [0.0, 0.0, -3.0]);
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);


    
    //Texture!
    gl.bindTexture(gl.TEXTURE_2D, texChecker);
    gl.uniform1i(samplerUnifrom, 0);

    //ONE!
    gl.bindVertexArray(vaoRectangle);
    rectVertArr[0] = 0.0;
    rectVertArr[1] = 1.0;
    rectVertArr[2] = 0.0;
    rectVertArr[3] = -2.0;
    rectVertArr[4] = 1.0;
    rectVertArr[5] = 0.0;
    rectVertArr[6] = -2.0;
    rectVertArr[7] = -1.0;
    rectVertArr[8] = 0.0;
    rectVertArr[9] = 0.0;
    rectVertArr[10] = -1.0;
    rectVertArr[11] = 0.0;

    gl.bindBuffer(gl.ARRAY_BUFFER, vboRectanglePos);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(rectVertArr), gl.DYNAMIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);


    //Two  
    rectVertArr[0] = 2.41421;
    rectVertArr[1] = 1.0;
    rectVertArr[2] = -1.41421;
    rectVertArr[3] = 1.0;
    rectVertArr[4] = 1.0;
    rectVertArr[5] = 0.0;
    rectVertArr[6] = 1.0;
    rectVertArr[7] = -1.0;
    rectVertArr[8] = 0.0;
    rectVertArr[9] = 2.41421;
    rectVertArr[10] = -1.0;
    rectVertArr[11] = -1.41421;

    gl.bindBuffer(gl.ARRAY_BUFFER, vboRectanglePos);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(rectVertArr), gl.DYNAMIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);

    gl.bindVertexArray(null);

    gl.useProgram(null);

    requestAnimationFrame(display, canvas);
}


const uninitialize = function() {
    //Code!.

    if(vboRectanglePos){
        gl.deleteBuffer(vboRectanglePos);
        vboRectanglePos = null;
    }
    if(vboRectangleTex){
        gl.deleteBuffer(vboRectangleTex);
        vboRectangleTex = null;
    }
    if(vaoRectangle) {
        gl.deleteVertexArray(vaoRectangle);
        vaoRectangle = null;
    }

    if(shaderProgObj) {
        if(fragmentShaderObj) {
            gl.detachShader(shaderProgObj, fragmentShaderObj);
            gl.deleteShader(fragmentShaderObj);
            fragmentShaderObj = null;
        }

        if(vertexShaderObj) {
            gl.detachShader(shaderProgObj, vertexShaderObj);
            gl.deleteShader(vertexShaderObj);
            vertexShaderObj = null;
        }

        gl.deleteProgram(shaderProgObj);
        shaderProgObj = null;
    }
}

const keyDown = function(event) {
    
    switch(event.keyCode) {
        case 27:
            uninitialize();
            window.close();
            break;

        case 70:
            toggleFullScreen();
            break;

    }
}

const mouseEvent = function() {
    //Code!
}

