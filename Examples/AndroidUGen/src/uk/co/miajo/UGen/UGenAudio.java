package uk.co.miajo.UGen;

//import android.content.Context;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
//import android.media.MediaPlayer;
import android.util.Log;
import android.os.Process;


public class UGenAudio extends Thread 
{
	protected static final String TAG="UGen-Android";
	
	/**
	 * UGen++ AUDIO OUTPUT SETTINGS
	 * 
	 * Fairly lo-fi by default, there's still room for optimisation.  A good acceptance
	 * test is to waggle the notifications bar about while running - does it glitch much?
	 */
	private int numInChans = 1; // can choose in ctor but not afterwards 
	final int numOutChans = 1;
	public static int sampleRateInHz = 22050;
	
	// bufSizeFrames (size of audio buffer passed in from android) 
	final int bufSizeFrames = 1024;
	final int shortsPerSample = 1; // this is tied to what the NDK code does to pass audio to UGen++, can't change easily.
	final int bufSizeShorts = bufSizeFrames * numOutChans * shortsPerSample; 
	final int bufferMultiples = 2;
	
	short[] audioBuf = new short[bufSizeShorts];
	AudioRecord audioRecord; // input
	AudioTrack audioTrack;   // output
	
	/*
	 * Audio driver state variables.
	 * The cycle looks like this:
	 *   (1) before run() is invoked,     running==false and ended==true
	 *   (2) when run() is invoked,       running==true  and ended==false  <-- audio driver main loop
	 *   (3) when sendQuit() is invoked,  running==false and ended==false  <-- briefly, waiting for SC to tell us [/done /quit]
	 *   (4) when SC properly shuts down, running==false and ended==true
	 */
	private boolean running=false; // set to true when run() is invoked, set to false by ScService.stop() (as well as sending a /quit msg)      ???
	private boolean ended=true; // whether the audio thread really has stopped and tidied up or not

    static { System.loadLibrary("UGenAudio"); }
	    
    public native void createIOHost(double sampleRate, int numInputs, int numOutputs, int blockSize);
    public native void destroyIOHost();
    public native int processIOHost(short[] someAudioBuf);
 	public native int processIOHostOutputOnly(short[] someAudioBuf);
 	public native int sendTrigger(int index);
 	public native int setParameter(int index, float value);
 	public native int sendBytes(int index, int size, byte[] data);

 	public int setParameter(int index, double value)
 	{
 		return setParameter(index, (float)value);
 	}
 	
	public UGenAudio(String dllDirStr)
	{
		this(1,dllDirStr);
	}
	
	public UGenAudio(int numInChans,String dllDirStr)
	{
		this.numInChans = numInChans;
		
		createIOHost((double)sampleRateInHz, numInChans, numOutChans, bufSizeFrames);
	}
		
	public boolean isRunning()
	{
		return running;
	}
	
	public boolean isEnded()
	{
		return ended;
	}
	
	public void sendQuit()
	{
		running = false;
	}

		
	/**
	 * The main audio loop lives here- or at least the Java part of it.  Most of the actual
	 * action is done in C++
	 */
	@Override
	public void run()
	{
		Log.i(TAG, "Starting UGen++ audio thread");
		
		running = true;
		ended = false;
		
		@SuppressWarnings("all") // the ternary operator does not contain dead code
		int channelConfiguration = numOutChans==2?
					AudioFormat.CHANNEL_CONFIGURATION_STEREO
					:AudioFormat.CHANNEL_CONFIGURATION_MONO;
		
		int minSize = AudioTrack.getMinBufferSize(
				sampleRateInHz, 
				channelConfiguration, 
				AudioFormat.ENCODING_PCM_16BIT);
		
		
		if(numInChans != 0)
		{
			minSize = Math.max(minSize,
			      AudioRecord.getMinBufferSize(
					sampleRateInHz, 
					channelConfiguration, 
					AudioFormat.ENCODING_PCM_16BIT)
			      );
		}
		
		Log.i(TAG, "minSize = "+minSize);

		
//		setPriority(Thread.MAX_PRIORITY);
//		android.os.Process.setThreadPriority(android.os.Process.THREAD_PRIORITY_URGENT_AUDIO);
		Process.setThreadPriority(Process.THREAD_PRIORITY_URGENT_AUDIO);

//		android.os.Process.setThreadPriority(0);

		boolean gotRecord=false;
		// instantiate AudioTrack
		try{
			audioTrack = new AudioTrack(
					AudioManager.STREAM_MUSIC, 
					sampleRateInHz, 
					channelConfiguration, 
					AudioFormat.ENCODING_PCM_16BIT, 
					minSize * bufferMultiples, 
					AudioTrack.MODE_STREAM);
		}catch(IllegalArgumentException e){
			Log.e(TAG, "failed to create AudioTrack object: " + e.getMessage());
			e.printStackTrace();
		}
		if(numInChans != 0){
			// instantiate AudioRecord
			try{
				audioRecord = new AudioRecord(
						MediaRecorder.AudioSource.MIC, 
						sampleRateInHz, 
						channelConfiguration, 
						AudioFormat.ENCODING_PCM_16BIT, 
						minSize * bufferMultiples);
				gotRecord = (audioRecord.getState()==AudioRecord.STATE_INITIALIZED);
			}catch(IllegalArgumentException e){
				Log.e(TAG, "failed to create AudioRecord object: " + e.getMessage());
				e.printStackTrace();
			}
		}

		audioTrack.play(); // this must be done BEFORE we write data to it
		Thread.yield();
		
		int ndkReturnVal;
		if (gotRecord) 
		{
			Log.i(TAG, "Entering UGen++ audio main loop (input and output)");

			
			audioRecord.startRecording();
			Thread.yield();
			
			while(running)
			{
				// let the NDK make the sound!
				
				if(audioRecord.read(audioBuf, 0, bufSizeShorts) != bufSizeShorts)
				{
					//Log deactivated for now at least since we already get 
					//             W/AudioFlinger( 1353): RecordThread: buffer overflow
					//Log.w(TAG, "audioRecord.read didn't read a complete buffer-full");
				}
				
				ndkReturnVal = processIOHost(audioBuf);
				
				if(ndkReturnVal!=0) 
				{
					Log.e(TAG,"UGen++ returned non-zero value "+ndkReturnVal);
					running=false;
				}
				
				audioTrack.write(audioBuf, 0, bufSizeShorts);
				//Thread.yield();
			}
		} else {
			Log.i(TAG, "Entering UGen++ audio main loop (output only)");

			while(running)
			{
				// let the NDK make the sound!
				ndkReturnVal = processIOHostOutputOnly(audioBuf);
				
				if(ndkReturnVal!=0) 
				{
					Log.e(TAG,"UGen++ returned non-zero value "+ndkReturnVal);
					running=false;
				}
				
				audioTrack.write(audioBuf, 0, bufSizeShorts);
				//Thread.yield();
			}
		}

		Log.i(TAG, "Stopping UGen++ audio thread");
		
		audioTrack.stop();
		audioTrack.release();
		
		if(gotRecord)
		{
			audioRecord.stop();
			audioRecord.release();
		}
		
		ended = true;
	}
	

}
