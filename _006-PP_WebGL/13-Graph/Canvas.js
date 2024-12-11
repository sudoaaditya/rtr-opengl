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

var vaoAxes, vboAxesPos;
var vaoHorUp, vboHorUpPos, vaoHorDwn, vboHorDwnPos;
var vaoRt, vboRtPos, vaoLft, vboLftPos;
var mvpUniform;
var perspectiveProjMat;

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
            "out vec4 out_color;" +
            "uniform mat4 u_mvp_matrix;" +
            "void main(void) {" +
            "   gl_Position = u_mvp_matrix * vPosition;" +
            "   out_color = vColor;" +
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
            "in vec4 out_color;" +
            "out vec4 FragColor;" +
            "void main(void) {" +
            "   FragColor = out_color;" +
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

    //VAOs & VBOs Posting
    var axesVert = new Float32Array([1.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, -1.0, 0.0]);
        var horLinesVert = [];
        var fSteps = 0.05;
        for(var i = 0; i< 10; i++) {
            for(var j = 0; j < 4; j++) {
                if(j == 0) {
                    horLinesVert[(i*4) + j] = 1.0;
                }
                else if(j == 2) {
                    horLinesVert[(i * 4) + j] = -1.0;
                }
                else {
                    horLinesVert[(i * 4) + j] = fSteps;
                }
            }
            fSteps += 0.05;
        }
        var horLinesVertDwn = [];
        fSteps = 0.05;
        for(var i = 0; i< 10; i++) {
            for(var j = 0; j < 4; j++) {
                if(j == 0) {
                    horLinesVertDwn[(i*4) + j] = 1.0;
                }
                else if(j == 2) {
                    horLinesVertDwn[(i * 4) + j] = -1.0;
                }
                else {
                    horLinesVertDwn[(i * 4) + j] = -fSteps;
                }
            }
            fSteps += 0.05;
        }
        var vertLinesVert = [80];
        fSteps = 0.05;
        for(var i = 0; i< 20; i++) {
            for(var j = 0; j < 4; j++) {
                if(j == 1) {
                    vertLinesVert[(i*4) + j] = 1.0;
                }
                else if(j == 3) {
                    vertLinesVert[(i * 4) + j] = -1.0;
                }
                else {
                    vertLinesVert[(i * 4) + j] = fSteps;
                }
            }
            fSteps += 0.05;
        }
        var vertLinesVertDwn = [80];
        fSteps = 0.05;
        for(var i = 0; i< 20; i++) {
            for(var j = 0; j < 4; j++) {
                if(j == 1) {
                    vertLinesVertDwn[(i*4) + j] = 1.0;
                }
                else if(j == 3) {
                    vertLinesVertDwn[(i * 4) + j] = -1.0;
                }
                else {
                    vertLinesVertDwn[(i * 4) + j] = -fSteps;
                }
            }
            fSteps += 0.05;
        }
            console.log(horLinesVert);
            console.log(horLinesVertDwn);
            console.log(vertLinesVert);
            console.log(vertLinesVertDwn);
            
        //Axes
        vaoAxes = gl.createVertexArray();
        gl.bindVertexArray(vaoAxes);
        vboAxesPos = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboAxesPos);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(axesVert), gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        gl.vertexAttrib3f(WebGLMacros.AMC_ATTRIBUTE_COLOR, 1.0, 0.0, 0.0);
        gl.bindVertexArray(null);
        //Hor UP
        vaoHorUp = gl.createVertexArray();
        gl.bindVertexArray(vaoHorUp);
        vboHorUpPos = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboHorUpPos);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(horLinesVert), gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 2, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        gl.vertexAttrib3f(WebGLMacros.AMC_ATTRIBUTE_COLOR, 0.0, 1.0, 0.0);
        gl.bindVertexArray(null);
        //Hor Down
        vaoHorDwn = gl.createVertexArray();
        gl.bindVertexArray(vaoHorDwn);
        vboHorDwnPos = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboHorDwnPos);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(horLinesVertDwn), gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 2, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        gl.vertexAttrib3f(WebGLMacros.AMC_ATTRIBUTE_COLOR, 0.0, 1.0, 0.0);
        gl.bindVertexArray(null);
        //Left 
        vaoLft = gl.createVertexArray();
        gl.bindVertexArray(vaoLft);
        vboLftPos = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboLftPos);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertLinesVert), gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 2, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        gl.vertexAttrib3f(WebGLMacros.AMC_ATTRIBUTE_COLOR, 0.0, 1.0, 0.0);
        gl.bindVertexArray(null);
        //Right
        vaoRt = gl.createVertexArray();
        gl.bindVertexArray(vaoRt);
        vboRtPos = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboRtPos);
        gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertLinesVertDwn), gl.STATIC_DRAW);
        gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 2, gl.FLOAT, false, 0, 0);
        gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
        gl.vertexAttrib3f(WebGLMacros.AMC_ATTRIBUTE_COLOR, 0.0, 1.0, 0.0);
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
    var i = 0;

    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(shaderProgObj);

    var modelViewMat = mat4.create();
    var modelViewProjMat = mat4.create();

    mat4.translate(modelViewMat, modelViewMat, [0.0, 0.0, -1.0]);
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);

    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);

    gl.lineWidth(5);
    gl.bindVertexArray(vaoAxes);
    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.bindVertexArray(null);

    //gl.lineWidth(0.8);
    gl.bindVertexArray(vaoHorUp);
    for(i = 0; i < 20; i += 2) {
        gl.drawArrays(gl.LINES, i, 2);
    }
    gl.bindVertexArray(null);

    gl.bindVertexArray(vaoHorDwn);
    for(i = 0; i < 20; i += 2) {
        gl.drawArrays(gl.LINES, i, 2);
    }
    gl.bindVertexArray(null);

    gl.bindVertexArray(vaoLft);
    for(i = 0; i < 40; i += 2) {
        gl.drawArrays(gl.LINES, i, 2);
    }
    gl.bindVertexArray(null);

    gl.bindVertexArray(vaoRt);
    for(i = 0; i < 40; i += 2) {
        gl.drawArrays(gl.LINES, i, 2);
    }
    gl.bindVertexArray(null);

    gl.bindVertexArray(null);
    gl.useProgram(null);

    requestAnimationFrame(display, canvas);
}


const uninitialize = function() {
    //Code!.

    if(vboHorUpPos){
        gl.deleteBuffer(vboHorUpPos);
        vboHorUpPos = null;
    }
    if(vboHorDwnPos){
        gl.deleteBuffer(vboHorDwnPos);
        vboHorDwnPos = null;
    }
    if(vboLftPos){
        gl.deleteBuffer(vboLftPos);
        vboLftPos = null;
    }
    if(vboRtPos){
        gl.deleteBuffer(vboRtPos);
        vboRtPos = null;
    }
    if(vboAxesPos){
        gl.deleteBuffer(vboAxesPos);
        vboAxesPos = null;
    }
    if(vaoAxes) {
        gl.deleteVertexArray(vaoAxes);
        vaoAxes = null;
    }
    if(vaoHorUp) {
        gl.deleteVertexArray(vaoHorUp);
        vaoHorUp = null;
    }
    if(vaoHorDwn) {
        gl.deleteVertexArray(vaoHorDwn);
        vaoHorDwn = null;
    }
    if(vaoLft) {
        gl.deleteVertexArray(vaoLft);
        vaoLft = null;
    }
    if(vaoRt) {
        gl.deleteVertexArray(vaoRt);
        vaoRt = null;
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

