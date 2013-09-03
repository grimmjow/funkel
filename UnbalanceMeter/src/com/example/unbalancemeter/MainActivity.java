package com.example.unbalancemeter;

import java.io.ByteArrayOutputStream;
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
import android.text.method.ScrollingMovementMethod;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.TextView;

public class MainActivity extends Activity implements SensorEventListener {
	public final static String EXTRA_MESSAGE = "com.example.unbalancemeter.MESSAGE";
	
	private int CROP_WIDTH_PCNT_BEGIN = 0;
	private int CROP_WIDTH_PCNT_END = 100;
	private int CROP_HEIGHT_PCNT_BEGIN = 45;
	private int CROP_HEIGHT_PCNT_END = 55;
	private int cropWidthBegin;
	private int cropWidthEnd;
	private int cropHeightBegin;
	private int cropHeightEnd;
	
	private Camera camera;
    private CameraPreview mPreview;
    private int frameCount = 0;
    private Date frameStamp = new Date();
    private int darkeningSensitivity = 15;
    
    private boolean foundBlackMark;
    private int lastAvrg;
    private long previousStamp = System.nanoTime();
    private long duration;

	private TextView colorText;
	private TextView fpsText;
	private TextView durationText;
	private TextView spsText;
	private TextView sprText;
	private TextView rpsText;
	private ImageView imageView1;
	private TextView editText;
	
	private NumberFormat decimalFormat = NumberFormat.getInstance();
	
	private SensorManager mSensorManager;
	private Sensor mSensor;
	private float[] lastSample;
	private float[] zero;
    private Date sampleStamp = new Date();
	private int sampleCount;
	private int roundSampleCount;
	
	private List<Double> intenses = new ArrayList<Double>();


	
    @Override
    protected void onCreate(Bundle savedinstancestate) {
        super.onCreate(savedinstancestate);
        setContentView(R.layout.activity_main);

        // collect all GUI widgets
        spsText = (TextView) findViewById(R.id.spsEdit);
        sprText = (TextView) findViewById(R.id.sprEdit);
        colorText = (TextView) findViewById(R.id.colorText);
        fpsText = (TextView) findViewById(R.id.fpsText);
        durationText = (TextView) findViewById(R.id.durationText);
        rpsText = (TextView) findViewById(R.id.textRps);
        imageView1 = (ImageView) findViewById(R.id.plot);
        editText = (TextView) findViewById(R.id.editText1);
        
        decimalFormat.setMaximumFractionDigits(2);
        
        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        initCamera();
    }


    private void initCamera() {

        Log.i("test", "initCamera");
        camera = Camera.open(0);
        
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
    	parameters.setColorEffect(Parameters.EFFECT_MONO);
    	parameters.setPreviewFormat(ImageFormat.NV21);
    	
    	// zoom to the max
    	int maxZoom = parameters.getMaxZoom();
        parameters.setZoom(maxZoom);
        
        // apply properties
        camera.setParameters(parameters);
        camera.setDisplayOrientation(90);
        
		// Create our Preview view and set it as the content of our activity.
        Log.i("test", "start gui preview");
        
        // create callbackbuffer based on imageformat. We need this buffer
        // to retrieve preview image for calulation while having it displayed
        // on the GUI.
        byte[] callbackBuffer = new byte[imageSize.width * imageSize.height * ImageFormat.getBitsPerPixel(ImageFormat.NV21)/8 ];
        mPreview = new CameraPreview(this, camera, new PreviewCallback() {
			
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
		        	float rps = (1/(duration / 1000000000F));
		        	rpsText.setText(decimalFormat.format(rps));
		        	roundSampleCount = 0;
		        	drawCycle(lastSample);
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
		}, callbackBuffer);
        FrameLayout preview = (FrameLayout) findViewById(R.id.camera_preview);
        preview.addView(mPreview);    
		
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
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
    
    public void closeApp(View view) {
    	android.os.Process.killProcess(android.os.Process.myPid());
    }

    public void resetZero(View view) {
    	zero = null;
    }

    protected void onResume() {
        super.onResume();
        mSensorManager.registerListener(this, mSensor, SensorManager.SENSOR_DELAY_FASTEST);
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
		
//		if(lastSample != null) {
//			
//			NumberFormat instance = DecimalFormat.getInstance();
//			instance.setMinimumFractionDigits(8);
//
//			StringBuilder sb = new StringBuilder();
//			sb.append("0:").append(instance.format(lastSample[0] - values[0])).append("\t(").append(values[0]).append(")\n");
//			sb.append("1:").append(instance.format(lastSample[1] - values[1])).append("\t(").append(values[1]).append(")\n");
//			sb.append("2:").append(instance.format(lastSample[2] - values[2])).append("\t(").append(values[2]).append(")\n");
//			
//			accelText.setText(sb.toString());
//			
//		}
		
		if((new Date()).getTime() - sampleStamp.getTime() > 1000) {
	        spsText.setText(Integer.toString(sampleCount));
	        sampleStamp = new Date();		
			sampleCount = 0;
		} else {
			sampleCount++;
		}		
		
		roundSampleCount++;
		
		// have to copy, cause event.values is always the same buffer
		float[] copyOfValues = Arrays.copyOf(values, values.length);
		lastSample = copyOfValues;
		
		if(zero == null) {
			zero = copyOfValues;
		}
		
		
	}

	private void drawCycle(float[] sample) {

		if(zero == null) {
			return;
		}
		
		float res = 40;
		Bitmap bitmap = Bitmap.createBitmap(Math.round(res), Math.round(res), Config.RGB_565);
		
		Paint centerPaint = new Paint();
		centerPaint.setColor(Color.BLUE);
		
		Paint directionPaint = new Paint();
		directionPaint.setColor(Color.GREEN);
		
		Canvas canvas = new Canvas(bitmap);
		canvas.drawColor(Color.WHITE);
		int midX=(int) (res/2), midY=(int) (res/2);
		float dx=midX, dy=midY;
		canvas.drawPoint(midX, midY, centerPaint);
		dx += (zero[0] - sample[0]);
		dy += (zero[1] - sample[1]);
		if(intenses.size() == 40) {
			intenses.remove(0);
		}
		intenses.add(Math.sqrt((Math.sqrt(Math.abs(zero[0] - sample[0])) + Math.sqrt(Math.abs(zero[1] - sample[1])))));
		
		double intense =0D;
		for(double intenseSample : intenses) {
			intense += intenseSample;
		}
		
		editText.setText("intense: " + intense/intenses.size() + "\nz: " + (zero[2] - sample[2]));
		canvas.drawPoint(dx, dy, directionPaint);
		imageView1.setImageBitmap(bitmap);
		
	}	
}
