package com.example.random;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

public class Random extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_random);
        TextView tv = (TextView)findViewById(R.id.random_textview);
        tv.setText( "Random Number: " + getRandom() );
    }

    public native double getRandom();

    static {
        System.loadLibrary("random");
    }
}
