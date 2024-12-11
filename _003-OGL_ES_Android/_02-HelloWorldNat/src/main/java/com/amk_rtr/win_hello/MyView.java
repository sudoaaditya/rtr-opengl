package com.amk_rtr.win_hello;

// Added By ME!
import androidx.appcompat.widget.AppCompatTextView; //for AppCompatTextView Class.
import android.content.Context; //for Context Class.
import android.view.Gravity;    //for Gravity Class.
import android.graphics.Color;  //for Color Class.


public class MyView extends AppCompatTextView {

    public MyView(Context drawingContext) {
        
        super(drawingContext);

        setTextColor(Color.rgb(0, 255, 0));
        setTextSize(60);
        setGravity(Gravity.CENTER);
        setText("Hello World!!..");
    }
}


