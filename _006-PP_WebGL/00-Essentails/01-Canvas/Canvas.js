
const main = function() {
    //get canvas elemet!>
    var canvas = document.getElementById("AMC");
    if(!canvas){
        console.log("Failed to Obtain Canvas!");
    }
    else {
        console.log("Canvas Obtained Successfully!!..");
    }

    console.log("Canvas Width:"+canvas.width+" & Height:"+canvas.height);
    
    //Obtain 2D context, i.e Render!
    var context = canvas.getContext("2d");
    if(!context){
        console.log("Failed to Obtain 2D COntext!");
    }
    else {
        console.log("2D Context Obtained Successfully!!");
    }

    // Fill canvas with Black Color, 
    context.fillStyle = "black";
    context.fillRect(0, 0, canvas.width, canvas.height);

    //Centerlized Text!.
    context.textAlign = "center";
    context.textBaseline = "middle";

    //text
    var string = "Hello From Canvas!.";
    context.font = "48px Bahnschrift";

    //textColor!
    context.fillStyle = "red";

    //display Centeralized Text!.
    context.fillText(string, canvas.width/2, canvas.height/2);
}
