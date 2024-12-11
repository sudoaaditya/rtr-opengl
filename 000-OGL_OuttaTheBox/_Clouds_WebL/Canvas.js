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

var mvpUniform;
var samplerUnifrom;
var resolutionUniform;
var timeUniform;
var mouseUniform;
var yUnifrom;

var texNoise = 0;
var perspectiveProjMat;

var mouseData = {
    mX: 1.0,
    mY: 1.0,
    mZ: 1.0,
    mW: 1.0,
}
var timeData = 0.0;
var resolutionData = [];
var yValue = 0.0;

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
    "uniform mat4 u_mvp_matrix;" +
    "void main(void) {" +
    "   gl_Position = u_mvp_matrix * vPosition;" +
    "}";
    
    vertexShaderObj = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObj, vertexShaderSource);
    gl.compileShader(vertexShaderObj);
    if(gl.getShaderParameter(vertexShaderObj, gl.COMPILE_STATUS) == false) {
        var error = gl.getShaderInfoLog(vertexShaderObj);
        console.log(error)
        if(error.lenght > 0){
            console.log(error);
            uninitialize();
        }
    }
    else {
        console.log("Vertex Shader Compiled Successfully!");
    }

    //Fragment    
    fragmentShaderObj = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObj, fragmentShaderSource);
    gl.compileShader(fragmentShaderObj);
    if(gl.getShaderParameter(fragmentShaderObj, gl.COMPILE_STATUS) == false){
        var error = gl.getShaderInfoLog(fragmentShaderObj);
        console.log(error);
        if(error.lenght > 0){
            console.log(error);
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
    //Fragment Uniforms binding.
    resolutionUniform = gl.getUniformLocation(shaderProgObj, "iResolution");
    timeUniform = gl.getUniformLocation(shaderProgObj, "iTime");
    samplerUnifrom = gl.getUniformLocation(shaderProgObj, "iChannel0");
    mouseUniform = gl.getUniformLocation(shaderProgObj, "iMouse");
    yUnifrom = gl.getUniformLocation(shaderProgObj, "iYdir");

    //Load Textures Call Here
    //Changed by Tejas [START]
    //texNoise = loadTexture("Noise.png");
    texNoise = loadImageTexture("Noise.png");
    //Changed by Tejas [END]

    console.log(texNoise);

    var RectVertices = new Float32Array([
        3.0, 1.7, 0.0,
        -3.0, 1.7, 0.0,
        -3.0, -1.7, 0.0,
        3.0, -1.7, 0.0
    ]);

    vaoRectangle = gl.createVertexArray();
    gl.bindVertexArray(vaoRectangle);
    vboRectanglePos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboRectanglePos);
    gl.bufferData(gl.ARRAY_BUFFER, RectVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    //Set Color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjMat = mat4.create();

}

//Added by Tejas [START]

const loadImageTexture = function (fileName) {
    var texHolder = 0;
    texHolder = gl.createTexture();
    texHolder.image = new Image();
    texHolder.image.src = fileName;
    texHolder.image.onload = function () {
        gl.bindTexture(gl.TEXTURE_2D, texHolder);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, false);
        gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, false);
        gl.pixelStorei(gl.UNPACK_COLORSPACE_CONVERSION_WEBGL, gl.NONE)
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA8, gl.RGBA, gl.UNSIGNED_BYTE, texHolder.image);

        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);

        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
        gl.generateMipmap(gl.TEXTURE_2D);

        gl.bindTexture(gl.TEXTURE_2D, null);
    }

    return (texHolder);
}

//Added by Tejas [END]

/* 
const loadTexture = function (fileName) {
    var texHolder = 0;
    texHolder = gl.createTexture();
    texHolder.image = new Image();
    texHolder.image.src = fileName;
    texHolder.image.onload = function () {
        gl.bindTexture(gl.TEXTURE_2D, texHolder);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.R8, 256, 0, 0, gl.RED, gl.UNSIGNED_BYTE, null);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.bindTexture(gl.TEXTURE_2D, null);

    }

    return (texHolder);
}
 */
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
    const { mX, mY, mZ, mW } = mouseData
    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.useProgram(shaderProgObj);

    var modelViewMat = mat4.create();
    var modelViewProjMat = mat4.create();

    mat4.translate(modelViewMat, modelViewMat, [0.0, 0.0, -3.0]);
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);

    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);
    gl.uniform3fv(resolutionUniform, new Float32Array([parseFloat(canvas.width), parseFloat(canvas.height), 1.0]));
    gl.uniform4fv(mouseUniform, new Float32Array([mX, mY, mZ, mW]));
    gl.uniform1f(timeUniform, timeData);
    gl.uniform1f(yUnifrom, yValue)

    //Texture!
    gl.bindTexture(gl.TEXTURE_2D, texNoise);

    gl.uniform1i(samplerUnifrom, 0);

    gl.bindVertexArray(vaoRectangle);
    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    gl.bindVertexArray(null);

    gl.useProgram(null);

    updateTime();

    requestAnimationFrame(display, canvas);
}

const updateTime = function() {
    timeData += 0.01;
    yValue += 0.001;
    if(yValue >= 2.0) {
        yValue 
    }
    if (timeData >= 1000.0) {
        timeData = 0.0
    }
}

const uninitialize = function() {
    //Code!.

    if(vboRectanglePos){
        gl.deleteBuffer(vboRectanglePos);
        vboRectanglePos = null;
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

const mouseEvent = function(event) {
    //Code!
    mouseData.mX = event.clientX;
    mouseData.mY = event.clientY;
}
