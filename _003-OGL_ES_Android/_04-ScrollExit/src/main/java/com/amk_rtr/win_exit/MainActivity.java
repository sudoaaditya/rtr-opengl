package com.amk_rtr.win_exit;

//Default Given Packages
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

// Added By ME!
import android.view.Window; //For Window Class.
import android.view.WindowManager; //For WindowManager Class.
import android.content.pm.ActivityInfo; //for ActivityInfo Class.
import android.graphics.Color; // for Color Class

public class MainActivity extends AppCompatActivity {

    private MyView myView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        //setContentView(R.layout.activity_main);

        // Get Rid Of Title Bar.
        this.supportRequestWindowFeature(Window.FEATURE_NO_TITLE);

        //Make FullScreen
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        //Forced Landscape Orientataion
        this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);

        //Set Black Background
        this.getWindow().getDecorView().setBackgroundColor(Color.BLACK);

        //Define Our OWN View
        myView = new MyView(this);

        //Set My View as Current One!
        setContentView(myView);
    }
    
    @Override
    protected void onPause(){
        super.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

}

