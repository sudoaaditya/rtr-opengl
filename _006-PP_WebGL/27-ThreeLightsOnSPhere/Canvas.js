//Global Vars!.
var canvas = null;
var gl = null;
var bFullScreen = null;
var canvasOriginalWidth = null;
var canvasOriginalHeight =  null;

const WebGLMacros = {
    AMC_ATTRIBUTE_POSITION:0,
    AMC_ATTRIBUTE_COLOR: 1,
    AMC_ATTRIBUTE_NORMAL:2,
    AMC_ATTRIBUTE_TEXCOORD0:3
}

var vertexShaderObj;
var fragmentShaderObj;
var shaderProgObj;

var sphereObj = null;
//UniformS!.
var modelUniform;
var viewUniform;
var projUnifrom;
var kaUniform;
var kdUniform;
var ksUniform;
var matShineUniform;
var lPressedUnifrom;

//One!
var laZeroUniform;
var ldZeroUniform;
var lsZeroUniform;
var lightZeroPosUniform;
//Two!
var laOneUniform;
var ldOneUniform;
var lsOneUniform;
var lightOnePosUniform;
//Three!
var laTwoUniform;
var ldTwoUniform;
var lsTwoUniform;
var lightTwoPosUniform;

//LightVars!
var bLighting = false;
var lightAngle = 0.0;
var Radius = 200.0;

//Light Parameters!.
//Red Light
var lightAmbientZero = [0.0, 0.0, 0.0];
var lightDiffuseZero = [1.0, 0.0, 0.0];
var lightSpecularZero = [1.0, 0.0, 0.0];
var lightPositionZero = [0.0, 0.0, 0.0, 1.0];
//Green
var lightAmbientOne = [0.0, 0.0, 0.0];
var lightDiffuseOne = [0.0, 1.0, 0.0];
var lightSpecularOne = [0.0, 1.0, 0.0];
var lightPositionOne = [0.0, 0.0, 0.0, 1.0];
//Blue
var lightAmbientTwo = [0.0, 0.0, 0.0];
var lightDiffuseTwo = [0.0, 0.0, 1.0];
var lightSpecularTwo = [0.0, 0.0, 1.0];
var lightPositionTwo = [0.0, 0.0, 0.0, 1.0];

//Material
var materialAmbient = [0.0, 0.0, 0.0];
var materialDiffuse = [1.0, 1.0, 1.0];
var materialSpecular = [1.0, 1.0, 1.0];
var materialShininess = 128.0;


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
        console.log("Failed to get WebGL2 Context!!");
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
            "precision mediump float;" +
            "precision lowp int;" +
            "in vec4 vPosition;" +
            "in vec3 vNormal;" +
            "uniform mat4 u_modelMat;" +
            "uniform mat4 u_viewMat;" +
            "uniform mat4 u_projMat;" +
            "uniform int u_lPressed;" +
            "uniform vec4 u_lightPosZero;" +
            "uniform vec4 u_lightPosOne;" +
            "uniform vec4 u_lightPosTwo;" +
            "out vec3 t_normal;"    +
            "out vec3 viewerVector;" +
            "out vec3 lightDirectionZero;" +
            "out vec3 lightDirectionOne;" +
            "out vec3 lightDirectionTwo;" +
            "void main(void) {" +
            "   if(u_lPressed == 1) {" +
            "       vec4 eyeCoords = u_viewMat * u_modelMat * vPosition;" +
            "       t_normal = mat3(u_viewMat * u_modelMat) * vNormal;" +
            "       lightDirectionZero = vec3(u_lightPosZero - eyeCoords);" +
            "       lightDirectionOne = vec3(u_lightPosOne - eyeCoords);" +
            "       lightDirectionTwo = vec3(u_lightPosTwo - eyeCoords);" +
            "       viewerVector = vec3(-eyeCoords.xyz);" +
            "   }" +
            "   gl_Position = u_projMat * u_viewMat * u_modelMat * vPosition;"  +
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
            "precision mediump float;" +
            "precision lowp int;" +
            "uniform vec3 u_laOne;" +
            "uniform vec3 u_ldOne;" +
            "uniform vec3 u_lsOne;" +
            "uniform vec3 u_laZero;" +
            "uniform vec3 u_ldZero;" +
            "uniform vec3 u_lsZero;" +
            "uniform vec3 u_laTwo;" +
            "uniform vec3 u_ldTwo;" +
            "uniform vec3 u_lsTwo;" +
            "uniform vec3 u_ka;" +
            "uniform vec3 u_kd;" +
            "uniform vec3 u_ks;" +
            "uniform float u_matShine;" +
            "uniform int u_lPressed;" +
            "out vec4 FragColor;" +
            "in vec3 t_normal;" +
            "in vec3 lightDirectionZero;" +
            "in vec3 lightDirectionOne;" +
            "in vec3 lightDirectionTwo;" +
            "in vec3 viewerVector;" +
            "void main(void) {" +
            "   vec3 phong_ads_light;" +
            "   if(u_lPressed == 1) {" +
            "       vec3 n_tNormal = normalize(t_normal);" +
            "       vec3 n_lightDirectionZero = normalize(lightDirectionZero);" +
            "       vec3 n_lightDirectionOne = normalize(lightDirectionOne);" +
            "       vec3 n_lightDirectionTwo = normalize(lightDirectionTwo);" +
            "       vec3 n_viewerVec = normalize(viewerVector);" +

            "       float tn_dot_ld_zero = max(dot(n_lightDirectionZero, n_tNormal), 0.0);" +
            "       vec3 reflectionVectorZero = reflect(-n_lightDirectionZero, n_tNormal);" +
            "       vec3 ambientZero = u_laZero * u_ka;" +
            "       vec3 diffuseZero = u_ldZero * u_kd * tn_dot_ld_zero;" +
            "       vec3 specularZero= u_lsZero * u_ks * pow(max(dot(reflectionVectorZero, n_viewerVec), 0.0), u_matShine);" +
            "       float tn_dot_ld_one = max(dot(n_lightDirectionOne, n_tNormal), 0.0);" +
            "       vec3 reflectionVectorOne = reflect(-n_lightDirectionOne, n_tNormal);" +
            "       vec3 ambientOne = u_laOne * u_ka;" +
            "       vec3 diffuseOne = u_ldOne * u_kd * tn_dot_ld_one;" +
            "       vec3 specularOne= u_lsOne * u_ks * pow(max(dot(reflectionVectorOne, n_viewerVec), 0.0), u_matShine);" +
            "       float tn_dot_ld_two = max(dot(n_lightDirectionTwo, n_tNormal), 0.0);" +
            "       vec3 reflectionVectorTwo = reflect(-n_lightDirectionTwo, n_tNormal);" +
            "       vec3 ambientTwo = u_laTwo * u_ka;" +
            "       vec3 diffuseTwo = u_ldTwo * u_kd * tn_dot_ld_two;" +
            "       vec3 specularTwo= u_lsTwo * u_ks * pow(max(dot(reflectionVectorTwo, n_viewerVec), 0.0), u_matShine);" +
            "       phong_ads_light = ambientZero + ambientOne + ambientTwo + diffuseOne + diffuseZero + diffuseTwo + specularOne + specularZero + specularTwo;" +
            "   }" +
            "   else {" +
            "       phong_ads_light = vec3(1.0, 1.0, 1.0);" +
            "   }" +
            "   FragColor = vec4(phong_ads_light, 1.0);" +
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
    gl.bindAttribLocation(shaderProgObj, WebGLMacros.AMC_ATTRIBUTE_NORMAL, "vNormal");

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

    modelUniform = gl.getUniformLocation(shaderProgObj, "u_modelMat");
    viewUniform = gl.getUniformLocation(shaderProgObj, "u_viewMat");
    projUnifrom = gl.getUniformLocation(shaderProgObj, "u_projMat");
    kaUniform = gl.getUniformLocation(shaderProgObj, "u_ka");
    kdUniform = gl.getUniformLocation(shaderProgObj, "u_kd");
    ksUniform = gl.getUniformLocation(shaderProgObj, "u_ks");
    matShineUniform = gl.getUniformLocation(shaderProgObj, "u_matShine");
    lPressedUnifrom = gl.getUniformLocation(shaderProgObj, "u_lPressed");
    laZeroUniform = gl.getUniformLocation(shaderProgObj, "u_laZero");
    ldZeroUniform = gl.getUniformLocation(shaderProgObj, "u_ldZero");
    lsZeroUniform = gl.getUniformLocation(shaderProgObj, "u_lsZero");
    lightZeroPosUniform = gl.getUniformLocation(shaderProgObj, "u_lightPosZero");
    laOneUniform = gl.getUniformLocation(shaderProgObj, "u_laOne");
    ldOneUniform = gl.getUniformLocation(shaderProgObj, "u_ldOne");
    lsOneUniform = gl.getUniformLocation(shaderProgObj, "u_lsOne");
    lightOnePosUniform = gl.getUniformLocation(shaderProgObj, "u_lightPosOne");
    laTwoUniform = gl.getUniformLocation(shaderProgObj, "u_laTwo");
    ldTwoUniform = gl.getUniformLocation(shaderProgObj, "u_ldTwo");
    lsTwoUniform = gl.getUniformLocation(shaderProgObj, "u_lsTwo");
    lightTwoPosUniform = gl.getUniformLocation(shaderProgObj, "u_lightPosTwo");
    
    //Generating Sphere
    sphereObj = new Mesh();
    makeSphere(sphereObj, 2.0, 100, 100);
    //console.log(sphereObj);
    
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);
    //gl.enable(gl.CULL_FACE);

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

    gl.useProgram(shaderProgObj);

    var modelMat = mat4.create();
    var viewMat = mat4.create();
    mat4.translate(modelMat, modelMat, [0.0, 0.0, -5.0]);

    gl.uniformMatrix4fv(modelUniform, false, modelMat);
    gl.uniformMatrix4fv(viewUniform, false, viewMat);
    gl.uniformMatrix4fv(projUnifrom, false, perspectiveProjMat);

    if(bLighting === true) {
        gl.uniform3fv(kaUniform, materialAmbient);
        gl.uniform3fv(kdUniform, materialDiffuse);
        gl.uniform3fv(ksUniform, materialSpecular);
        gl.uniform1f(matShineUniform, materialShininess);

        gl.uniform3fv(laZeroUniform, lightAmbientZero);
        gl.uniform3fv(ldZeroUniform, lightDiffuseZero);
        gl.uniform3fv(lsZeroUniform, lightSpecularZero);
        //ALong X. so change Y & z
        lightPositionZero[1] = Radius * Math.sin(lightAngle);
        lightPositionZero[2] = Radius * Math.cos(lightAngle);
        gl.uniform4fv(lightZeroPosUniform, lightPositionZero);

        gl.uniform3fv(laOneUniform, lightAmbientOne);
        gl.uniform3fv(ldOneUniform, lightDiffuseOne);
        gl.uniform3fv(lsOneUniform, lightSpecularOne);
        //ALong Y. so change Z & X
        lightPositionOne[0] = Radius * Math.sin(lightAngle);
        lightPositionOne[2] = Radius * Math.cos(lightAngle);
        gl.uniform4fv(lightOnePosUniform, lightPositionOne);
        
        gl.uniform3fv(laTwoUniform, lightAmbientTwo);
        gl.uniform3fv(ldTwoUniform, lightDiffuseTwo);
        gl.uniform3fv(lsTwoUniform, lightSpecularTwo);
        //ALong Z. so change X& Y
        lightPositionTwo[0] = Radius * Math.sin(lightAngle);
        lightPositionTwo[1] = Radius * Math.cos(lightAngle);
        gl.uniform4fv(lightTwoPosUniform, lightPositionTwo);
        
        gl.uniform1i(lPressedUnifrom, 1);
    }
    else {
        gl.uniform1i(lPressedUnifrom, 0);
    }
    sphereObj.draw();
    gl.useProgram(null);

    update();
    requestAnimationFrame(display, canvas);
}


const update = function() {
    lightAngle += 0.03;
    if(lightAngle >= 360.0) {
        lightAngle = 0.0;
    }
}

const uninitialize = function() {
    //Code!.
    if(sphereObj) {
        sphereObj.deallocate();
        sphereObj = null;
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

        case 76:
            bLighting = bLighting === true ? false : true;
            break;
    }
}

const mouseEvent = function() {
    //Code!
}

