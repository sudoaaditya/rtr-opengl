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

var vaoTriangle;
var vboTrianglePos;
var vboTriangleCol;
var vaoRectangle;
var vboRectanglePos;
var mvpUniform;
var perspectiveProjMat;

var fTriangleRot = 0.0; 
var fRectRot = 0.0;

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
            "in vec4 vColor;" +
            "uniform mat4 u_mvp_matrix;" +
            "out vec4 out_vColor;" +
            "void main(void) {" +
            "   gl_Position = u_mvp_matrix * vPosition;" +
            "   out_vColor = vColor;" +
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
            "in vec4 out_vColor;" +
            "out vec4 FragColor;" +
            "void main(void) {" +
            "   FragColor = out_vColor;" +
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
    gl.bindAttribLocation(shaderProgObj, WebGLMacros.AMC_ATTRIBUTE_COLOR, "vColor");

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

    //Vertices!.
    var triangleVertices = new Float32Array([
		0.0,1.0,0.0,
		-1.0,-1.0,0.0,
		1.0,-1.0,0.0
        ]);
        
    var triangleCol = new Float32Array([
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0
    ]);

    vaoTriangle = gl.createVertexArray();
    gl.bindVertexArray(vaoTriangle);
    vboTrianglePos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboTrianglePos);
    gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    vboTriangleCol = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboTriangleCol);
    gl.bufferData(gl.ARRAY_BUFFER, triangleCol, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    var RectVertices = new Float32Array([
        1.0, 1.0, 0.0,
        -1.0, 1.0, 0.0,
        -1.0, -1.0, 0.0,
        1.0, -1.0, 0.0
    ]);

    vaoRectangle = gl.createVertexArray();
    gl.bindVertexArray(vaoRectangle);
    vboRectanglePos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboRectanglePos);
    gl.bufferData(gl.ARRAY_BUFFER, RectVertices, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.vertexAttrib3f(WebGLMacros.AMC_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0);
    gl.bindVertexArray(null);

    //Set Color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    perspectiveProjMat = mat4.create();

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

    mat4.translate(modelViewMat, modelViewMat, [-2.0, 0.0, -5.0]);
    mat4.rotateY(modelViewMat, modelViewMat, degToRad(fTriangleRot));
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);
    gl.bindVertexArray(vaoTriangle);
    gl.drawArrays(gl.TRIANGLES, 0, 3);
    gl.bindVertexArray(null);

    mat4.identity(modelViewMat);
    mat4.identity(modelViewProjMat);

    mat4.translate(modelViewMat, modelViewMat, [2.0, 0.0, -5.0]);
    mat4.rotateY(modelViewMat, modelViewMat, degToRad(fRectRot));
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);
    gl.bindVertexArray(vaoRectangle);
    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    gl.bindVertexArray(null);

    gl.useProgram(null);

    update();
    requestAnimationFrame(display, canvas);
}

const update = function() {
console.log(fTriangleRot, fRectRot);

    fTriangleRot += 2.0;
    if(fTriangleRot >= 360.0) {
        fTriangleRot = -360.0;
    }

    fRectRot += 2.0;
    if(fRectRot >= 360.0) {
        fRectRot = -360.0;
    }
}

const degToRad = function(degAngle) {
    return( degAngle * Math.PI / 180);
}

const uninitialize = function() {
    //Code!.

    if(vboTrianglePos){
        gl.deleteBuffer(vboTrianglePos);
        vboTrianglePos = null;
    }
    if(vboTriangleCol) {
        gl.deleteBuffer(vboTriangleCol);
        vboTriangleCol = null;
    }
    if(vaoTriangle) {
        gl.deleteVertexArray(vaoTriangle);
        vaoTriangle = null;
    }

    
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

const mouseEvent = function() {
    //Code!
}

