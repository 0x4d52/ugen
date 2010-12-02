/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package uk.co.miajo.UGen;

import android.app.Activity;
import android.widget.TextView;
import android.os.Bundle;


public class AndroidUGen extends Activity
{
	private UGenAudio audioThread;
	
	// match the enum in the C++ code
	static final int Freq = 0;
	static final int Amp = 1;
	static final int On = 2;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
                
        audioThread = new UGenAudio("/data/data/uk.co.miajo.UGen/lib"); //??
        
        audioThread.setParameter(Freq, 500);
        audioThread.setParameter(Amp, 0.5);
        audioThread.setParameter(On, 1);
        
        audioThread.start();
        
        TextView  tv = new TextView(this);
        tv.setText( "Testing..." );
        setContentView(tv);
    }

	@Override
    public void onStop()
    {
		super.onStop();
		
		audioThread.sendQuit();
		
		while(!audioThread.isEnded())
		{
			try{
				Thread.sleep(50L);
			}
			catch(InterruptedException err)
			{
				err.printStackTrace();
				break;
			}
		}
		
		audioThread.destroyIOHost();
    }
}
