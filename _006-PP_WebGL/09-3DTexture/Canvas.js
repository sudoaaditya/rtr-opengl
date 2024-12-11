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

var vaoPyramid;
var vboPyramidPos;
var vboPyramidTex;
var vaoCube;
var vboCubePos;
var vboCubeTex;
var mvpUniform;
var samplerUnifrom;
var perspectiveProjMat;

var fAnglePyramid = 0.0; 
var fAngleCube = 0.0;
var texStone = 0;
var texKundali = 0;

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

    //Load Textures Call Here
    texStone = loadTexture("Stone.png");
    texKundali = loadTexture("Kundali.png");

    //Vertices!.
    var pyramidVertices = new Float32Array([
		0.0, 1.0, 0.0,
		-1.0, -1.0, 1.0,
        1.0, -1.0, 1.0,
        
        0.0, 1.0, 0.0,
        1.0, -1.0, 1.0,
        1.0, -1.0, -1.0,

        0.0, 1.0, 0.0, 
        1.0, -1.0, -1.0,
        -1.0, -1.0, -1.0,

        0.0, 1.0, 0.0,
        -1.0, -1.0, -1.0,
        -1.0, -1.0, 1.0

        ]);

    var pyramidTexCoords = new Float32Array([
        0.5, 1.0, 
        0.0, 0.0,
        1.0, 0.0,

        0.5, 1.0,
        1.0, 0.0,
        0.0, 0.0,

        0.5, 1.0,
        1.0, 0.0,
        0.0, 0.0,

        0.5, 1.0,
        0.0, 0.0,
        1.0, 0.0
    ]);

    vaoPyramid = gl.createVertexArray();
    gl.bindVertexArray(vaoPyramid);
    vboPyramidPos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboPyramidPos);
    gl.bufferData(gl.ARRAY_BUFFER, pyramidVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    
    vboPyramidTex =  gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboPyramidTex);
    gl.bufferData(gl.ARRAY_BUFFER, pyramidTexCoords, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD0, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD0);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);


    var cubeVertexArr = new Float32Array([
        -1.0, 1.0, 1.0,-1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0,//ront
        1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0,//Right
		1.0, 1.0, -1.0, 1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 1.0, -1.0,//Back
		-1.0, 1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 1.0, -1.0, 1.0, 1.0,//Let
		-1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, -1.0,//Top
		-1.0, -1.0, 1.0, -1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, -1.0, 1.0//Bottom
    ]);

    var cubeTexCoords = new Float32Array([
        0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0
    ])


    vaoCube = gl.createVertexArray();
    gl.bindVertexArray(vaoCube);
    vboCubePos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboCubePos);
    gl.bufferData(gl.ARRAY_BUFFER, cubeVertexArr, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);

    vboCubeTex = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboCubeTex);
    gl.bufferData(gl.ARRAY_BUFFER, cubeTexCoords, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD0, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD0);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    //Set Color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    gl.enable(gl.DEPTH_TEST);
    gl.enable(gl.CULL_FACE)

    perspectiveProjMat = mat4.create();

}

const loadTexture = function(fileName) {
    var texHolder = 0;
    texHolder = gl.createTexture();
    texHolder.image = new Image();
    texHolder.image.src = fileName;
    texHolder.image.onload = function() {
        gl.bindTexture(gl.TEXTURE_2D, texHolder);
        gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, texHolder.image);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
        gl.bindTexture(gl.TEXTURE_2D, null);
    }
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
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(shaderProgObj);

    var modelViewMat = mat4.create();
    var modelViewProjMat = mat4.create();

    mat4.translate(modelViewMat, modelViewMat, [-2.5, 0.0, -6.0]);
    mat4.rotateY(modelViewMat, modelViewMat, degToRad(fAnglePyramid));
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);
    //Texture!
    gl.bindTexture(gl.TEXTURE_2D, texStone);
    gl.uniform1i(samplerUnifrom, 0);

    gl.bindVertexArray(vaoPyramid);
    gl.drawArrays(gl.TRIANGLES, 0, 12);
    gl.bindVertexArray(null);

    mat4.identity(modelViewMat);
    mat4.identity(modelViewProjMat);

    mat4.translate(modelViewMat, modelViewMat, [2.5, 0.0, -6.0]);
    mat4.rotateX(modelViewMat, modelViewMat, degToRad(fAngleCube));
    mat4.rotateY(modelViewMat, modelViewMat, degToRad(fAngleCube));
    mat4.rotateZ(modelViewMat, modelViewMat, degToRad(fAngleCube));
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);

    //Texture!
    gl.bindTexture(gl.TEXTURE_2D, texKundali);
    gl.uniform1i(samplerUnifrom, 0);

    gl.bindVertexArray(vaoCube);
    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
    gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);
    gl.bindVertexArray(null);

    gl.useProgram(null);

    update();
    requestAnimationFrame(display, canvas);
}

const update = function() {
//console.log(fAnglePyramid, fAngleCube);

    fAnglePyramid += 2.0;
    if(fAnglePyramid >= 360.0) {
        fAnglePyramid = -360.0;
    }

    fAngleCube += 2.0;
    if(fAngleCube >= 360.0) {
        fAngleCube = -360.0;
    }
}

const degToRad = function(degAngle) {
    return( degAngle * Math.PI / 180);
}

const uninitialize = function() {
    //Code!.

    if(vboPyramidPos){
        gl.deleteBuffer(vboPyramidPos);
        vboPyramidPos = null;
    }
    if(vboPyramidTex) {
        gl.deleteBuffer(vboPyramidTex);
        vboPyramidTex = null;
    }
    if(vaoPyramid) {
        gl.deleteVertexArray(vaoPyramid);
        vaoPyramid = null;
    }

    
    if(vboCubePos){
        gl.deleteBuffer(vboCubePos);
        vboCubePos = null;
    }
    if(vaoCube) {
        gl.deleteVertexArray(vaoCube);
        vaoCube = null;
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

