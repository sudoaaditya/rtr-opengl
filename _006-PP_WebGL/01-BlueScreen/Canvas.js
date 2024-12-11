//GLobal vars
var canvas = null;
var gl = null;  //To MaintainParity!
var canvasOriginalWidth = null;
var canvasOriginalHeight = null;
var bFullScreen = false; //FullScreen Var

//requestAnimationFrame for rendering!.
var requestAnimationFrame = window.requestAnimationFrame || window.webkitRequestAnimationFrame ||
                            window.msRequestAnimationFrame || window.mozRequestAnimationFrame || 
                            window.oRequestAnimationFrame || null;

// TO Cancel rendering if needed in bacground processsing!
var cancelAnimationFrame = window.cancelAnimationFrame || window.webkitCancelAnimationFrame ||
                            window.webkitCancelRequestAnimationFrame || window.mozCancelAnimationFrame ||
                            window.mozCancelRequestAnimationFrame || window.oCancelRequestAnimationFrame ||
                            window.oCancelAnimationFrame || window.CancelRequestAnimationFrame ||
                            window.msCancelAnimationFrame || null;

const main = function() {
    //get canvas elemet!>
    canvas = document.getElementById("AMC");
    if(!canvas){
        console.log("Failed to Obtain Canvas!");
    }
    else {
        console.log("Canvas Obtained Successfully!!..");
    }

    canvasOriginalWidth = canvas.width;
    canvasOriginalHeight = canvas.height;

    //Register Event Handler Callback to the window!.
    window.addEventListener("keydown", keyDown, false); 
    window.addEventListener("click", mouseEvent, false);
    window.addEventListener("resize", resize, false);    

    //Init WebGL.
    initialize();

    //Start WarmUP Drawing
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

const initialize =  function() {
    //Get Context
    gl = canvas.getContext("webgl2");
    if(gl === null){
        console.log("Failed to get WebGL2 Context!");
        return;
    }
    else {
        console.log("Successfully Acquired the WebGL2 Context!!");
    }

    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    //set Blue Color!
    gl.clearColor(0.0, 0.0, 1.0, 1.0);
}

const resize = function() {
    if(bFullScreen === true) {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    else {
        canvas.width = canvasOriginalWidth;
        canvas.height = canvasOriginalHeight;
    }

    //Set viewport 
    gl.viewport(0, 0, canvas.width, canvas.height);
}


const display = function() {
    //Code
    gl.clear(gl.COLOR_BUFFER_BIT);

    requestAnimationFrame(display, canvas);
}

const keyDown = function(event) {
    switch(event.keyCode){
        case 70:
            toggleFullScreen();
            break;
    }
}

const mouseEvent = function(event) {
    //Nessecary Code!.
}


