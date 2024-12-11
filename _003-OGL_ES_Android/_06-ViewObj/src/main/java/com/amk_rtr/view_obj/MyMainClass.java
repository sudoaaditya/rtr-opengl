package com.amk_rtr.view_obj;

//Default Pacages
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

//Packages by ME!
import android.view.Window; //for Window class
import android.view.WindowManager;  //for WindowManager class
import android.content.pm.ActivityInfo; //for ActivityiNfo class
import android.graphics.Color;   // for Color class
//for View
import androidx.appcompat.widget.AppCompatTextView; //for AppCompatTextView class
import android.content.Context; //for Context class
import android.view.Gravity;    // fior Gravity calss

public class MyMainClass extends AppCompatActivity {

    @Override
    public void onCreate(Bundle savedState) {
        super.onCreate(savedState);

        this.supportRequestWindowFeature(Window.FEATURE_NO_TITLE);
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        this.getWindow().getDecorView().setBackgroundColor(Color.BLACK);

        AppCompatTextView myView = new AppCompatTextView(this);

        myView.setTextColor(Color.rgb(0, 255, 0));
        myView.setTextSize(50);
        myView.setGravity(Gravity.CENTER);
        myView.setText("Hello World!!..");

        setContentView(myView);

    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

}
