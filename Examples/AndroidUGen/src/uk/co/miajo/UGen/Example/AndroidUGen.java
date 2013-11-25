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

package uk.co.miajo.UGen.Example;

import uk.co.miajo.UGen.Example.R;
import uk.co.miajo.UGen.UGenAudio;
import android.app.Activity;
import android.os.Bundle;
import android.view.View.OnClickListener;
import android.view.View;
import android.widget.CheckBox;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.SeekBar.OnSeekBarChangeListener;

public class AndroidUGen extends Activity implements OnSeekBarChangeListener, OnClickListener {
	private UGenAudio audioThread;
	private CheckBox onOff;
	private SeekBar freqSlider, ampSlider;
	private TextView freqText, ampText;

	// match the enum in the C++ code
	static final int Freq = 0;
	static final int Amp = 1;
	static final int On = 2;

	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		/* widgets setup */
		
		// find views
		freqSlider = (SeekBar) findViewById(R.id.freqslider);
		ampSlider  = (SeekBar) findViewById(R.id.ampslider);
		onOff      = (CheckBox) findViewById(R.id.onOffbutton);
		freqText   = (TextView) findViewById(R.id.freq_label);
		ampText    = (TextView) findViewById(R.id.amp_label);

		// set listeners
		freqSlider.setOnSeekBarChangeListener(this);
		ampSlider.setOnSeekBarChangeListener(this);
		onOff.setOnClickListener(this);
		
		// set values
		freqText.setText("Frequency");
		ampText.setText("Amplitude");
		onOff.setChecked(false);

		/* UGen setup */
		
		audioThread = new UGenAudio("/data/data/uk.co.miajo.UGen/lib"); // ?? Context.getFilesDir().getPath()
		
		// initial values
		freqSlider.setProgress(2000);
		ampSlider.setProgress(10);
		
		audioThread.start();
	}

	@Override
	public void onStop() {
		super.onStop();

		audioThread.sendQuit();

		while (!audioThread.isEnded()) {
			try {
				Thread.sleep(50L);
			} catch (InterruptedException err) {
				err.printStackTrace();
				break;
			}
		}

		audioThread.destroyIOHost();
	}

	@Override
	public void onProgressChanged(SeekBar seekBar, int sliderVal, boolean fromUser) 
	{
		if (seekBar == freqSlider) {
			// set frequency to slider value 0-8000 (scale and shift (200 - 1000Hz)
			audioThread.setParameter(Freq, ((float) Integer.valueOf(sliderVal) * 0.1) + 200.0);
		} else if (seekBar == ampSlider) {
			// set amplitude to slider value 0-100 (up to 0.9)
			audioThread.setParameter(Amp, (float) Integer.valueOf(sliderVal) * 0.009);
		}
	}

	@Override
	public void onClick(View v) 
	{
		if (v == onOff) 
		{
			// turn audio on/off with checkbox
			audioThread.setParameter(On, onOff.isChecked() ? 1 : 0);
		}
	}
	
	/* unused methods */
	@Override
	public void onStartTrackingTouch(SeekBar seekBar) {
	}

	@Override
	public void onStopTrackingTouch(SeekBar seekBar) {
	}
}
