//Global Vars!.
var canvas = null;
var gl = null;
var bFullScreen = null;
var canvasOriginalWidth = null;
var canvasOriginalHeight =  null;

//Color Vars
//Saffron 255 153 51
const SAFFRON_R = 1.0;
const SAFFRON_G = 0.600;
const SAFFRON_B = 0.20;
//White 255 255 255
const WHITE_R = 1.0;
const WHITE_G = 1.0;
const WHITE_B = 1.0;
//Green 18 136 7
const GREEN_R = 0.070;
const GREEN_G = 0.533;
const GREEN_B = 0.027;
//Blue 0 0 136
const BLUE_R = 0.0;
const BLUE_G = 0.0;
const BLUE_B = 0.533;


const WebGLMacros = {
    AMC_ATTRIBUTE_POSITION:0,
    AMC_ATTRIBUTE_COLOR:1,
    AMC_ATTRIBUTE_NORMAL:2,
    AMC_ATTRIBUTE_TEXCOORD0:3
}

var vertexShaderObj;
var fragmentShaderObj;
var shaderProgObj;
var vaoI1, vaoN, vaoD, vaoI2, vaoA, vaoDArc;
var vboI1Pos, vboI1Col, vboNPos, vboNCol, vboDPos, vboDCol, vboI2Pos, vboI2Col, vboAPos, vboACol, vboDArcPos, vboDArcCol;
var mvpUniform;
var perspectiveProjMat;

//Animation paraameters;
var bI1Reached = false, bNReached = false, bDReached = false,bI2Reached = false, bAReached = false, bPlaneReached = false;
var fTransSpeed = 0.003;
var dArcCol = [];

//matrices!
var modelViewMat;
var modelViewProjMat;

//Animations!
var bXTrans = -0.8;
var bYTrans = 2.0;
var bXTransA = 0.8;
var bYTransI = -2.0;


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
            "   gl_PointSize = 1.5f;" +
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
    //I
    var I1Vert = new Float32Array([-1.5, 0.5, 0.0, -1.5, -0.5, 0.0]);
    var I1Col = new Float32Array([SAFFRON_R, SAFFRON_G, SAFFRON_B, GREEN_R, GREEN_G, GREEN_B]);
    vaoI1 = gl.createVertexArray();
    gl.bindVertexArray(vaoI1);
    vboI1Pos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboI1Pos);
    gl.bufferData(gl.ARRAY_BUFFER, I1Vert, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    vboI1Col = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboI1Col);
    gl.bufferData(gl.ARRAY_BUFFER, I1Col, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    //N
    var NVert = new Float32Array([-1.1, -0.5, 0.0, -1.1, 0.5, 0.0, -0.6, -0.5, 0.0, -0.6, 0.5, 0.0]);
    var NCol = new Float32Array([GREEN_R,GREEN_G,GREEN_B,SAFFRON_R,SAFFRON_G,SAFFRON_B,GREEN_R,GREEN_G,GREEN_B,SAFFRON_R,SAFFRON_G,SAFFRON_B]);
    vaoN = gl.createVertexArray();
    gl.bindVertexArray(vaoN);
    vboNPos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboNPos);
    gl.bufferData(gl.ARRAY_BUFFER, NVert, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    vboNCol = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboNCol);
    gl.bufferData(gl.ARRAY_BUFFER, NCol, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    //D ARC
    var dArcVert = [];
    var dArcCol = [];
    var R = GREEN_R, G = GREEN_G, B = GREEN_B; 
    var i , j;
    var angle = 3 * Math.PI/2;
    for(i=0; i < 315; i++) {
        for(j = 0; j < 3; j++) {
            if(j == 0)
                dArcVert[(i*3)+j] = -0.1+Math.cos (angle)*0.5;
            else if(j == 1) 
                dArcVert[(i*3)+j] = Math.sin(angle)*0.5;
            else 
                dArcVert[(i*3) +j] = 0.0;
            
            dArcCol[(i*3)+0] = R;
            dArcCol[(i*3)+1] = G;
            dArcCol[(i*3)+2] = B;
        }
        G += 0.00021;
		if(G == SAFFRON_G){
			G = SAFFRON_G;
		}
		R += 0.0029;
		if( R == SAFFRON_R){
			R = SAFFRON_R;
		}
		B += 0.00054;
		if(B == SAFFRON_B) {
			B = SAFFRON_B;
		}
        angle += 0.01;
    }
    vaoDArc = gl.createVertexArray();
    gl.bindVertexArray(vaoDArc);
    vboDArcPos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboDArcPos);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(dArcVert), gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    vboDArcCol = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboDArcCol);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(dArcCol), gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);
    
    //D
    var dLineVert = new Float32Array([-0.1, 0.5, 0.0, -0.21, 0.5, 0.0, -0.2, 0.5, 0.0, -0.2, -0.5, 0.0, -0.21, -0.5, 0.0, -0.1, -0.5, 0.0]);
    var dLineCol = new Float32Array([SAFFRON_R, SAFFRON_G, SAFFRON_B, SAFFRON_R, SAFFRON_G, SAFFRON_B, SAFFRON_R, SAFFRON_G, SAFFRON_B, GREEN_R, GREEN_G, GREEN_B, GREEN_R, GREEN_G, GREEN_B, GREEN_R, GREEN_G, GREEN_B]);
    vaoD = gl.createVertexArray();
    gl.bindVertexArray(vaoD);
    vboDPos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboDPos);
    gl.bufferData(gl.ARRAY_BUFFER, dLineVert, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    vboDCol = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboDCol);
    gl.bufferData(gl.ARRAY_BUFFER, dLineCol, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    //I20
    var I2Vert = new Float32Array([0.7,0.5,0.0,0.7,-0.5,0.0]);
    vaoI2 = gl.createVertexArray();
    gl.bindVertexArray(vaoI2);
    vboI2Pos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboI2Pos);
    gl.bufferData(gl.ARRAY_BUFFER, I2Vert, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    vboI2Col = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboI2Col);
    gl.bufferData(gl.ARRAY_BUFFER, I1Col, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    //A
    var aVert = new Float32Array([1.0, -0.5, 0.0, 1.3, 0.5, 0.0, 1.6, -0.5, 0.0, 1.15, 0.025, 0.0, 1.45, 0.025, 0.0, 1.15, 0.0, 0.0, 1.45, 0.0, 0.0,  1.15, -0.02, 0.0, 1.45, -0.02, 0.0] );
    var aCol = new Float32Array([GREEN_R,GREEN_G,GREEN_B, SAFFRON_R,SAFFRON_G,SAFFRON_B, GREEN_R,GREEN_G,GREEN_B, SAFFRON_R,SAFFRON_G,SAFFRON_B, SAFFRON_R,SAFFRON_G,SAFFRON_B, 1.0, 1.0, 1.0,  1.0, 1.0, 1.0,  GREEN_R,GREEN_G,GREEN_B,GREEN_R,GREEN_G,GREEN_B]);
    vaoA = gl.createVertexArray();
    gl.bindVertexArray(vaoA);
    vboAPos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboAPos);
    gl.bufferData(gl.ARRAY_BUFFER, aVert, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_POSITION);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    vboACol = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboACol);
    gl.bufferData(gl.ARRAY_BUFFER, aCol, gl.STATIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_COLOR, 3, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_COLOR);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
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
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    modelViewMat = mat4.create();
    modelViewProjMat = mat4.create();

    gl.useProgram(shaderProgObj);
    gl.lineWidth(5);
    
    drawI1();
	
	if(bAReached === true) {
		drawN();
	}
	if(bI2Reached === true) {
		drawD();
	}
	if(bNReached ===true) {
		drawI2();
	}
    if(bI1Reached === true) {
		drawA();
    }
        
    gl.useProgram(null);

    requestAnimationFrame(display, canvas);
}


const drawI1 = function() {

    mat4.identity(modelViewMat);
    mat4.identity(modelViewProjMat);

    mat4.translate(modelViewMat, modelViewMat, [bXTrans, 0.0, -2.5]);
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);

    //Draw I
    gl.lineWidth(1.0);
    gl.bindVertexArray(vaoI1);
    gl.drawArrays(gl.LINES, 0, 2);
    gl.bindVertexArray(null);

    bXTrans += fTransSpeed;
	if(bXTrans >= 0.0){
		bXTrans = 0.0;
		bI1Reached = true;
	}
}

const drawN = function() {

    mat4.identity(modelViewMat);
    mat4.identity(modelViewProjMat);

    mat4.translate(modelViewMat, modelViewMat, [0.0, bYTrans, -2.5]);
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);

    //Draw N
    gl.bindVertexArray(vaoN);
    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 1, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.bindVertexArray(null);

    bYTrans -= fTransSpeed;
	if(bYTrans <= 0.0) {
		bYTrans = 0.0;
		bNReached = true;
	}

}
const drawD = function() {
    
    mat4.identity(modelViewMat);
    mat4.identity(modelViewProjMat);

    mat4.translate(modelViewMat, modelViewMat, [0.0, 0.0, -2.5]);
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);

    //D
    gl.bindVertexArray(vaoD);
    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 2, 2);
    gl.drawArrays(gl.LINES, 4, 2);
    gl.bindVertexArray(null);
        
    gl.bindVertexArray(vaoDArc);
    gl.drawArrays(gl.POINTS, 0, 945);
    gl.bindVertexArray(null);
        
}
const drawI2 = function() {
    mat4.identity(modelViewMat);
    mat4.identity(modelViewProjMat);

    mat4.translate(modelViewMat, modelViewMat, [0.0, bYTransI, -2.5]);
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);

    //Draw I
    gl.bindVertexArray(vaoI2);
    gl.drawArrays(gl.LINES, 0, 2);
    gl.bindVertexArray(null);

    bYTransI += fTransSpeed;
	if(bYTransI >= 0.0) {
		bYTransI = 0.0;
		bI2Reached = true;
	}
    
}
const drawA = function() {
    mat4.identity(modelViewMat);
    mat4.identity(modelViewProjMat);

    mat4.translate(modelViewMat, modelViewMat, [bXTransA, 0.0, -2.5]);
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);

    //Draw  A
    gl.bindVertexArray(vaoA);
    //Middle Strip
    gl.drawArrays(gl.LINES, 3, 2);
    gl.drawArrays(gl.LINES, 5, 2);
    gl.drawArrays(gl.LINES, 7, 2);
    //Lines of A
    gl.drawArrays(gl.LINES, 0, 2);
    gl.drawArrays(gl.LINES, 1, 2);
    gl.bindVertexArray(null);

    bXTransA -= fTransSpeed;
	if(bXTransA <= 0.0){
	    bXTransA = 0.0;
		bAReached = true;
	}

}

const uninitialize = function() {
    //Code!.

    if(vboTrianglePos){
        gl.deleteBuffer(vboTrianglePos);
        vboTrianglePos = null;
    }
    if(vaoTriangle) {
        gl.deleteVertexArray(vaoTriangle);
        vaoTriangle = null;
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

