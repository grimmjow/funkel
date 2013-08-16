package com.example.unbalancemeter;

import java.io.ByteArrayOutputStream;
import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ImageFormat;
import android.graphics.Paint;
import android.graphics.PathEffect;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.hardware.Camera.Parameters;
import android.hardware.Camera.PreviewCallback;
import android.hardware.Camera.Size;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeech.OnInitListener;
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity implements OnInitListener, SensorEventListener {
	public final static String EXTRA_MESSAGE = "com.example.unbalancemeter.MESSAGE";
	
	private int CROP_WIDTH_PCNT_BEGIN = 45;
	private int CROP_WIDTH_PCNT_END = 55;
	private int CROP_HEIGHT_PCNT_BEGIN = 45;
	private int CROP_HEIGHT_PCNT_END = 55;
	private int cropWidthBegin;
	private int cropWidthEnd;
	private int cropHeightBegin;
	private int cropHeightEnd;
	
	private TextToSpeech tts;
	private Camera camera;
    private CameraPreview mPreview;
    private int frameCount = 0;
    private Date frameStamp = new Date();
    private int darkeningSensitivity = 4;
    
    private boolean foundBlackMark;
    private int lastAvrg;
    private long previousStamp = System.nanoTime();
    private long duration;

	private TextView colorText;
	private TextView fpsText;
	private TextView durationText;
	private TextView accelText;
	private TextView spsText;
	private TextView sprText;
	private ImageView imageView1;
	
	private SensorManager mSensorManager;
	private Sensor mSensor;
	private float[] lastSample;
    private Date sampleStamp = new Date();
	private int sampleCount;
	private int roundSampleCount;
	private int significantSampleCount;
	private int significantSample;
	private List<float[]> samples = new ArrayList<float[]>();

	
    @Override
    protected void onCreate(Bundle savedinstancestate) {
        super.onCreate(savedinstancestate);
        setContentView(R.layout.activity_main);
//        tts = new TextToSpeech(this, this);

        accelText = (TextView) findViewById(R.id.accelText);
        spsText = (TextView) findViewById(R.id.spsEdit);
        sprText = (TextView) findViewById(R.id.sprEdit);

        colorText = (TextView) findViewById(R.id.colorText);
        fpsText = (TextView) findViewById(R.id.fpsText);
        durationText = (TextView) findViewById(R.id.durationText);
        
        imageView1 = (ImageView) findViewById(R.id.plot);
        
        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        initCamera();
    }


    private void initCamera() {

        Log.i("test", "initCamera");
        camera = Camera.open(0);
        
        TextView editText = (TextView) findViewById(R.id.editText1);
        editText.setMovementMethod(new ScrollingMovementMethod());
        StringBuilder sb = new StringBuilder();

        int[] fpsRange = camera.getParameters().getSupportedPreviewFpsRange().get(0);
        
        for(int[] range : camera.getParameters().getSupportedPreviewFpsRange()) {
        	sb.append("Range: ("+range.length+") " + range[0] + "-" + range[1] + "\n");
        	if(fpsRange[0] < range[0] || fpsRange[1] < range[1]) {
        		fpsRange = range;
        	}
        }
        
        Size tmpImageSize = camera.getParameters().getSupportedPreviewSizes().get(0);
        for(Size size : camera.getParameters().getSupportedPreviewSizes()) {
        	sb.append("Size: w:" + size.width + ", h:" + size.height + "\n");
        	if(tmpImageSize.height > size.height || tmpImageSize.width > size.width) {
        		tmpImageSize = size;
        	}
        }
        final Size imageSize = tmpImageSize;
        
        List<String> supportedSceneModes = camera.getParameters().getSupportedSceneModes();
        if(supportedSceneModes != null) {
	        for(String sceneMode : supportedSceneModes) {
	        	sb.append("SceneMode: " + sceneMode + "\n");
	        }
        }
        for(String colorEffect : camera.getParameters().getSupportedColorEffects()) {
        	sb.append("colorEffect: " + colorEffect + "\n");
        }
        for(int previewFormat : camera.getParameters().getSupportedPreviewFormats()) {
        	sb.append("previewFormat: " + Integer.toString(previewFormat) + "\n");
        }
        for(String focusModes : camera.getParameters().getSupportedFocusModes()) {
        	sb.append("focusModes: " + focusModes + "\n");
        }    	
        editText.setText(sb.toString());
        
        Log.i("test", "configure camera");
    	Parameters parameters = camera.getParameters();
    	parameters.setPreviewFpsRange(fpsRange[0], fpsRange[1]);
    	parameters.setPreviewSize(imageSize.width, imageSize.height);
    	cropWidthBegin = ((imageSize.width * CROP_WIDTH_PCNT_BEGIN)/100);
    	cropWidthEnd = ((imageSize.width * CROP_WIDTH_PCNT_END)/100);
    	cropHeightBegin = ((imageSize.height * CROP_HEIGHT_PCNT_BEGIN)/100);
    	cropHeightEnd = ((imageSize.height * CROP_HEIGHT_PCNT_END)/100);
    	
//    	parameters.setFocusMode(Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
//    	parameters.setSceneMode(Parameters.SCENE_MODE_SPORTS);
//    	parameters.setAntibanding(Parameters.ANTIBANDING_OFF);
//    	parameters.setColorEffect(Parameters.EFFECT_MONO);
    	parameters.setPreviewFormat(ImageFormat.NV21);
        camera.setParameters(parameters);
        camera.setDisplayOrientation(90);
        
		// Create our Preview view and set it as the content of our activity.
//        Log.i("test", "start gui preview");
//        mPreview = new CameraPreview(this, camera);
//        FrameLayout preview = (FrameLayout) findViewById(R.id.camera_preview);
//        preview.addView(mPreview);

        byte[] callbackBuffer = new byte[imageSize.width * imageSize.height * ImageFormat.getBitsPerPixel(ImageFormat.NV21)/8 ];
        camera.setPreviewCallbackWithBuffer(new PreviewCallback() {
			
			@Override
			public void onPreviewFrame(byte[] data, Camera camera) {
				
				if((new Date()).getTime() - frameStamp.getTime() > 1000) {
			        fpsText.setText(Integer.toString(frameCount));
					frameStamp = new Date();		
					frameCount = 0;
				} else {
					frameCount++;
				}
				
				YuvImage yuvimage = new YuvImage(data, ImageFormat.NV21, imageSize.width, imageSize.height, null);
				
				ByteArrayOutputStream baos = new ByteArrayOutputStream();
				yuvimage.compressToJpeg(new Rect(cropWidthBegin, cropHeightBegin, 
						cropWidthEnd, cropHeightEnd), 100, baos);
				byte[] jdata = baos.toByteArray();

	            BitmapFactory.Options bitmapFatoryOptions = new BitmapFactory.Options();
	            bitmapFatoryOptions.inPreferredConfig = Bitmap.Config.RGB_565;
				Bitmap bmp = BitmapFactory.decodeByteArray(jdata, 0, jdata.length, bitmapFatoryOptions);

				double sum = 0;
				for(int x = 0; x < bmp.getWidth();x++) {
					for(int y = 0; y < bmp.getHeight();y++) {
						sum -= (double) bmp.getPixel(x, y);
					}
				}
				int avg = (int) (sum/(bmp.getWidth() * bmp.getHeight()));
		        colorText.setText(Integer.toString(avg));
		        
		        int darkening = 0;
		        if(lastAvrg > 0) {
		        	darkening = (avg * 100) / lastAvrg;
		        }
		        
		        if((darkening > (100 + darkeningSensitivity)) && !foundBlackMark) {
		        	foundBlackMark = true;
		        	duration = System.nanoTime() - previousStamp;
		        	sprText.setText(Integer.toString(roundSampleCount));
		        	roundSampleCount = 0;
		        	drawCycle(samples);
		        	samples.clear();
		        } if(darkening < (100 - darkeningSensitivity)) {
		        	if(foundBlackMark) {
		        		previousStamp = System.nanoTime();
		        	}
		        	foundBlackMark = false;
		        }
	        	durationText.setText(duration + "ns ("+foundBlackMark+", "+darkening+"%)");
	        	lastAvrg = avg;
		        camera.addCallbackBuffer(data);
		        
			}
		});
        camera.addCallbackBuffer(callbackBuffer);
        Log.i("test", "start preview");
        camera.startPreview();
		
	}


	@Override
	protected void onDestroy() {
		super.onDestroy();
		if(tts != null) {
			tts.shutdown();
		}
		if(camera != null) {
			camera.release();
		}
	}


	@Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    /** Called when the user clicks the Send button */
    public void sendMessage(View view) {
//    	Intent intent = new Intent(this, DisplayMessageActivity.class);
//    	EditText editText = (EditText) findViewById(R.id.edit_message);
//    	String message = editText.getText().toString();
//    	intent.putExtra(EXTRA_MESSAGE, message);
//        startActivity(intent);
//        tts.speak(message, TextToSpeech.QUEUE_ADD, null);
    }
    
    /** Called when the user clicks the Send button */
    public void closeApp(View view) {
    	android.os.Process.killProcess(android.os.Process.myPid());
    }

	@Override
	public void onInit(int status) {
		if (status == TextToSpeech.SUCCESS) {
		Toast.makeText(this,
		"Text-To-Speech engine is initialized", Toast.LENGTH_LONG).show();
		}
		else if (status == TextToSpeech.ERROR) {
		Toast.makeText(this,
		"Error occurred while initializing Text-To-Speech engine", Toast.LENGTH_LONG).show();
		}
		
	}

    protected void onResume() {
        super.onResume();
        mSensorManager.registerListener(this, mSensor, SensorManager.SENSOR_DELAY_NORMAL);
    }

    protected void onPause() {
        super.onPause();
        mSensorManager.unregisterListener(this);
    }

	@Override
	public void onAccuracyChanged(Sensor arg0, int arg1) {
		// TODO Auto-generated method stub
		
	}


	@Override
	public void onSensorChanged(SensorEvent event) {
		
		float[] values = event.values;
		
		if(lastSample != null) {
			
			NumberFormat instance = DecimalFormat.getInstance();
			instance.setMinimumFractionDigits(8);

			StringBuilder sb = new StringBuilder();
			sb.append("0:").append(instance.format(lastSample[0] - values[0])).append("\t(").append(values[0]).append(")\n");
			sb.append("1:").append(instance.format(lastSample[1] - values[1])).append("\t(").append(values[1]).append(")\n");
			sb.append("2:").append(instance.format(lastSample[2] - values[2])).append("\t(").append(values[2]).append(")\n");
			
			accelText.setText(sb.toString());
			
		}
		
		
		
		if((new Date()).getTime() - sampleStamp.getTime() > 1000) {
	        spsText.setText(Integer.toString(sampleCount));
	        sampleStamp = new Date();		
			sampleCount = 0;
		} else {
			sampleCount++;
		}		
		
		roundSampleCount++;
		
		// have to copy, cause event.values is always the same buffer
		float[] copyOf = Arrays.copyOf(values, values.length);
		samples.add(copyOf);
		lastSample = copyOf;
		
		
	}    
	
	private void drawCycle(List<float[]> samples) {

		float minX=0F, maxX=0F, minY=0F, maxY=0F, resX=0F, resY=0F, res=0F;
		
		for(float[] sample : samples) {

			resX += sample[0];
			resX += sample[1];
			
		}		
		
		if(resX < 0F) resX *= -1;
		if(resY < 0F) resY *= -1;
		
		resX = resX * 2F;
		resY = resY * 2F;
		resX = Math.max(resX, 20);
		resY = Math.max(resY, 20);
		res = Math.max(resY, resX);
		
		Bitmap bitmap = Bitmap.createBitmap(Math.round(res), Math.round(res), Config.RGB_565);
		
		Paint xPaint = new Paint();
		xPaint.setColor(Color.BLUE);
		
		Paint yPaint = new Paint();
		yPaint.setColor(Color.GREEN);
		
		Paint xyPaint = new Paint();
		xyPaint.setColor(Color.RED);
		
		Canvas canvas = new Canvas(bitmap);
		canvas.drawColor(Color.WHITE);
		int midX=(int) (res/2), midY=(int) (res/2);
		int x=midX, y=midY;
		for(float[] sample : samples) {
			x += sample[0];
			y += sample[1];
			canvas.drawPoint(x, midY, xPaint);
			canvas.drawPoint(midX, y, yPaint);
			canvas.drawPoint(x, y, xyPaint);
		}
		imageView1.setImageBitmap(bitmap);
	}
	
}
