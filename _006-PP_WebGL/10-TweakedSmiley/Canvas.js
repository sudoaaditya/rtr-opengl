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
var texSmiley = 0;
var iKeyPressed = 0;
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

    //Load Textures Call Here
    texSmiley = loadTexture("Smiley.png");
    console.log(texSmiley);

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

    vboRectangleTex = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, vboRectangleTex);
    gl.bufferData(gl.ARRAY_BUFFER, 4*4*2, gl.DYNAMIC_DRAW);
    gl.vertexAttribPointer(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD0, 2, gl.FLOAT, false, 0, 0);
    gl.enableVertexAttribArray(WebGLMacros.AMC_ATTRIBUTE_TEXCOORD0);
    gl.bindBuffer(gl.ARRAY_BUFFER, null);
    gl.bindVertexArray(null);

    //Set Color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

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
    var rectTexCoords = [];
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(shaderProgObj);

    var modelViewMat = mat4.create();
    var modelViewProjMat = mat4.create();

    mat4.translate(modelViewMat, modelViewMat, [0.0, 0.0, -3.0]);
    //mat4.rotateZ(modelViewMat, modelViewMat, -9.0)
    mat4.multiply(modelViewProjMat, perspectiveProjMat, modelViewMat);
    gl.uniformMatrix4fv(mvpUniform, false, modelViewProjMat);

    gl.bindVertexArray(vaoRectangle);
    if(iKeyPressed === 1) {
        rectTexCoords[0] = 0.5;
        rectTexCoords[1] = 0.5;
        rectTexCoords[2] = 0.0;
        rectTexCoords[3] = 0.5;
        rectTexCoords[4] = 0.0;
        rectTexCoords[5] = 0.0;
        rectTexCoords[6] = 0.5;
        rectTexCoords[7] = 0.0;
    }
    else if(iKeyPressed === 2) {
        rectTexCoords[0] = 1.0;
        rectTexCoords[1] = 1.0;
        rectTexCoords[2] = 0.0;
        rectTexCoords[3] = 1.0;
        rectTexCoords[4] = 0.0;
        rectTexCoords[5] = 0.0;
        rectTexCoords[6] = 1.0;
        rectTexCoords[7] = 0.0;
    }
    else if(iKeyPressed === 3) {
        rectTexCoords[0] = 2.0;
        rectTexCoords[1] = 2.0;
        rectTexCoords[2] = 0.0;
        rectTexCoords[3] = 2.0;
        rectTexCoords[4] = 0.0;
        rectTexCoords[5] = 0.0;
        rectTexCoords[6] = 2.0;
        rectTexCoords[7] = 0.0;
    }
    else if(iKeyPressed === 4) {
        rectTexCoords[0] = 0.5;
        rectTexCoords[1] = 0.5;
        rectTexCoords[2] = 0.5;
        rectTexCoords[3] = 0.5;
        rectTexCoords[4] = 0.5;
        rectTexCoords[5] = 0.5;
        rectTexCoords[6] = 0.5;
        rectTexCoords[7] = 0.5;
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, vboRectangleTex);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(rectTexCoords), gl.DYNAMIC_DRAW);
    gl.bindBuffer(gl.ARRAY_BUFFER, null)

    //Texture!
    gl.bindTexture(gl.TEXTURE_2D, texSmiley);
    gl.uniform1i(samplerUnifrom, 0);

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

        //Key Code 1 & numpad 1
        case 49:
        case 97:
            iKeyPressed = 1;
            break;

        //Keycode for 2 & numpad 2
        case 50:
        case 98:
            iKeyPressed = 2;
            break;

        //KeyCode for 3 & numpad 3
        case 51:
        case 99:
            iKeyPressed = 3;
            break;

        case 52:
        case 100:
            iKeyPressed = 4;
            break;

        default:
            iKeyPressed = 0;
            break;
    }
}

const mouseEvent = function() {
    //Code!
}

