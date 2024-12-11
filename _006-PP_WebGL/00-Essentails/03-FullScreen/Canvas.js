//GLobal vars
var canvas = null;
var context = null;

const main = function() {
    //get canvas elemet!>
    canvas = document.getElementById("AMC");
    if(!canvas){
        console.log("Failed to Obtain Canvas!");
    }
    else {
        console.log("Canvas Obtained Successfully!!..");
    }

    console.log("Canvas Width:"+canvas.width+" & Height:"+canvas.height);
    
    //Obtain 2D context, i.e Render!
    context = canvas.getContext("2d");
    if(!context){
        console.log("Failed to Obtain 2D COntext!");
    }
    else {
        console.log("2D Context Obtained Successfully!!");
    }

    // Fill canvas with Black Color, 
    context.fillStyle = "black";
    context.fillRect(0, 0, canvas.width, canvas.height);

    drawText("Hello From Canvas!!..", "#f34126", "50px", canvas.width/2, canvas.height/2);

    //Register Event Handler Callback to the window!.
    window.addEventListener("keydown", keyDown, false); //Place Horizontally Central
    window.addEventListener("click", mouseEvent, false);    //Place Vertically Central

}

const drawText = function(text, color, font, x, y) {
    //Centerlized Text!.
    context.textAlign = "center";
    context.textBaseline = "middle";

    //text Font
    context.font = font+" Bahnschrift";

    //textColor!
    context.fillStyle = color;

    //display Centeralized Text!.
    context.fillText(text, x, y);
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
        }
    }
}

const keyDown = function(event) {
    switch(event.keyCode){
        case 70:
            toggleFullScreen();
            //repaint Window!
            drawText("Hello From Canvas", "#f34126", "50px");
            break;
    }
}

const mouseEvent = function(event) {
    //Nessecary Code!.
}


