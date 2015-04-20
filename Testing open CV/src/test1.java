import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.DisplayMode;
import java.awt.FlowLayout;
import java.awt.Graphics;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.awt.Image;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionAdapter;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferByte;
import java.awt.image.ImageObserver;
import java.io.*;
import java.awt.event.KeyAdapter;

import static java.lang.System.*;

import javax.imageio.ImageIO;
import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.BoxLayout;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;



import javax.swing.JPanel;
import javax.swing.JSlider;
import javax.swing.JTextField;
import javax.swing.KeyStroke;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;

import org.opencv.core.Core;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.MatOfByte;
import org.opencv.imgproc.Imgproc;
import org.opencv.core.Point;
import org.opencv.core.Size;
import org.opencv.highgui.Highgui;
import org.opencv.photo.*;
import org.opencv.imgproc.*;
import org.opencv.core.*;

class MyCanvas extends JComponent {
	BufferedImage img;	
	int width;
	int height;

	public MyCanvas(BufferedImage img1){
		img = img1;		
	}
	public void updateImage(BufferedImage newImg){
		img = newImg;
		this.repaint();
	}

	public void paint(Graphics g) {
		//		  Image img1 = Toolkit.getDefaultToolkit().getImage("./images/uglyWhale.jpg");
		g.drawImage(img, 0, 0, this);
		g.finalize();
	}
}


class UpAction extends AbstractAction{
	public void actionPerformed( ActionEvent tf){
		out.println("up");
		test1.shiftCenter(0, -100);

	}	
}
class DownAction extends AbstractAction{
	public void actionPerformed( ActionEvent tf){
		out.println("down");
		test1.shiftCenter(0, 100);

	}	
}

class LeftAction extends AbstractAction{
	public void actionPerformed( ActionEvent tf){
		out.println("left");
		test1.shiftCenter(-100, 0);

	}	
}

class RightAction extends AbstractAction{
	public void actionPerformed( ActionEvent tf){
		out.println("right");
		test1.shiftCenter(100,0);

	}	
}

class RectGood implements ActionListener{
	public void actionPerformed(ActionEvent e) {
		out.println("happy button pressed");
		test1.runGrabCutFromRect();
	}
}

class DrawBackground implements ActionListener{
	public void actionPerformed(ActionEvent e){
		out.println("BGButton pressed");
		test1.setMaskToDraw(1);
	}
}

class DrawForeground implements ActionListener{
	public void actionPerformed(ActionEvent e){
		out.println("FGButton pressed");
		test1.setMaskToDraw(2);
	}
}

class Erase implements ActionListener{
	public void actionPerformed(ActionEvent e){
		out.println("Erase button pressed");
		test1.setMaskToDraw(0);
	}
}

class MarkupDone implements ActionListener{
	public void actionPerformed(ActionEvent e){
		out.println("done button pressed");
		test1.runGrabCutFromMask();
	}
}

class Finish implements ActionListener{
	public void actionPerformed(ActionEvent e){
		out.println("donezo");
		test1.exportMaskAndQuit();
	}
}


class RectMouseListener implements MouseListener{
	public void mouseClicked(MouseEvent e){
		out.println(e.getX() + "  "+ e.getY());
		test1.clickResponse(e.getX(), e.getY());
	}

	@Override
	public void mouseEntered(MouseEvent arg0) {
		out.println("mouse Entered");

	}

	@Override
	public void mouseExited(MouseEvent arg0) {
		out.println("mouse Exited");
	}

	@Override
	public void mousePressed(MouseEvent arg0) {
		out.println("mouse Pressed");

	}

	@Override
	public void mouseReleased(MouseEvent arg0) {
		out.println("mouse released");
	}
}

class EditMouseMotionListener extends MouseMotionAdapter{
	public void mouseDragged(MouseEvent e){
		test1.drawOnMask(e.getX(),e.getY());
		//out.println("dragging");
	}
}

class EditMouseReleaseListener extends MouseAdapter{
	public void mouseReleased(MouseEvent e){
		out.println("mouse released");
		test1.reRender();
	}
}



public class test1{
	private static String sourceDir;
	private static String resultDir;
	private static String name;
	
	private	static int screenWidth=100;
	private	static int screenHeight=100;
	private static Mat lowRes;
	private	static Mat baseImage;
	private static Mat original;
	private static Mat masterMask;
	private static Mat bgEditMask; // for people to draw on
	private static Mat fgEditMask; // ditto
	private static int drawOn; // 1 means bg, 2 means fg,0 means erase

	private	static double baseSF;
	private	static double currSF;
	private	static MyCanvas canvas;
	private	static double baseCenterX;
	private	static double baseCenterY;
	private	static double currCenterX;
	private	static double currCenterY;


	//rectangle coordinates for first run of grabcut
	private static int xtop = 0;
	private static int ytop = 0;
	private static int xbot = 0;
	private static int ybot = 0;

	public static void exportMaskAndQuit(){
		Mat finalMask = Mat.zeros(masterMask.size(), masterMask.type());
		
		Mat fg = new Mat(); // foreground
		Mat pfg = new Mat(); // probable foreground
		
		Mat ones = Mat.ones(masterMask.size(),masterMask.type());
		Mat temp = new Mat();
		
		Core.multiply(ones, new Scalar(Imgproc.GC_FGD), temp);
		Core.compare(masterMask, temp, fg,Core.CMP_EQ);

		//		Core.multiply(fg,new Scalar(100), temp);
		//		displayImageSimple(temp);


		Core.multiply(ones, new Scalar(Imgproc.GC_PR_FGD), temp);
		Core.compare(masterMask, temp, pfg,Core.CMP_EQ);
		
		finalMask.setTo(new Scalar(255), fg);
		finalMask.setTo(new Scalar(255), pfg);
		String out = resultDir + name +"_GCmask.jpg";
		Highgui.imwrite(out, finalMask);
		
		out = resultDir + name+ "_scaled.jpg";
		Highgui.imwrite(out,original);
		
		//Mat m = Highgui.imread("./processed_images/whaleGraygc.jpg");
		//displayImageSimple(m);
	}
	
	public static void runGrabCutFromMask(){
		Mat bgdModel = new Mat();
		Mat fgdModel = new Mat();
		bgdModel = Mat.zeros(1,65,CvType.CV_64FC1);
		fgdModel = Mat.zeros(1,65,CvType.CV_64FC1);
		//		bgdModel = new Mat();
		//		fgdModel = new Mat();
		Mat mask = masterMask.clone();
		mask.setTo(new Scalar(Imgproc.GC_BGD), bgEditMask);
		mask.setTo(new Scalar(Imgproc.GC_FGD), fgEditMask);
		Imgproc.grabCut(original,mask ,new Rect(0,0,1,1), bgdModel, fgdModel, 5,Imgproc.GC_INIT_WITH_MASK);
		masterMask = mask.clone();

		Mat bg = new Mat(); // background
		Mat fg = new Mat(); // foreground
		Mat pbg = new Mat(); // probable background
		Mat pfg = new Mat(); // probable foreground

		Mat ones = Mat.ones(mask.size(),mask.type());
		Mat temp = new Mat();
		Core.multiply(ones, new Scalar(Imgproc.GC_BGD), temp);
		Core.compare(mask, temp, bg,Core.CMP_EQ);

		//		Core.multiply(bg,new Scalar(100), temp);
		//		displayImageSimple(temp);


		Core.multiply(ones, new Scalar(Imgproc.GC_FGD), temp);
		Core.compare(mask, temp, fg,Core.CMP_EQ);

		//		Core.multiply(fg,new Scalar(100), temp);
		//		displayImageSimple(temp);


		Core.multiply(ones, new Scalar(Imgproc.GC_PR_BGD), temp);
		Core.compare(mask, temp, pbg,Core.CMP_EQ);

		//		Core.multiply(pbg,new Scalar(100), temp);
		//		displayImageSimple(temp);


		Core.multiply(ones, new Scalar(Imgproc.GC_PR_FGD), temp);
		Core.compare(mask, temp, pfg,Core.CMP_EQ);
		//		
		//		Core.multiply(pfg,new Scalar(100), temp);
		//		displayImageSimple(temp);


		Mat a = original.clone();
		ones = Mat.ones(original.size(), original.type());
		Core.add(a, new Scalar(0,0,100),a,bg);
		Core.add(a, new Scalar(0,0,100), a, pbg);
		Core.add(a, new Scalar(0,100,0),a, pfg);

		setupGUIForEdits(a);		
	}

	public static void setMaskToDraw(int maskID){
		drawOn = maskID;
	}

	public static void drawOnMask(int x, int y){
		x = (int)((currCenterX - baseCenterX+x)/currSF);
		y = (int)((currCenterY - baseCenterY+y)/currSF);
		out.println("x: " + x +" y: " +y);


		if(drawOn == 1){
			bgEditMask.put(y, x, 1);
		}
		if(drawOn == 2){
			fgEditMask.put(y, x, 1);
		}
		if(drawOn == 0){
			bgEditMask.put(y, x, 0);
			fgEditMask.put(y, x, 0);
		}		
	}

	public static void runGrabCutFromRect(){
		//Mat mask = rectangleMask.clone();
		Mat mask = new Mat();
		//mask.setTo(new Scalar(125));
		//		mask.put(1,1,1);
		//		mask.put(1, 2, 1);
		//		mask.put(1,3,3);
		int rxt = (int) ((xtop/currSF)/10);
		int ryt = (int)((ytop/currSF)/10);
		int rxb = (int) ((xbot/currSF)/10);
		int ryb = (int) ((ybot/currSF)/10);
		Rect rect = new Rect(rxt,ryt,rxb-rxt,ryb-ryt);
		Mat bgdModel = new Mat();
		//bgdModel.setTo(new Scalar(255,255,255));
		Mat fgdModel = new Mat();
		//fgdModel.setTo(new Scalar(255,255,255));
		bgdModel = Mat.zeros(1,65,CvType.CV_64FC1);
		fgdModel = Mat.zeros(1,65,CvType.CV_64FC1);
		//		bgdModel = new Mat();
		//		fgdModel = new Mat();

		Imgproc.grabCut(lowRes,mask ,rect, bgdModel, fgdModel, 5,Imgproc.GC_INIT_WITH_RECT);
		out.println("grabcut done");
		out.println(mask.cols());
		
		Imgproc.resize(mask,mask,original.size());
		
		
		
		
		

		Mat bg = new Mat(); // background
		Mat fg = new Mat(); // foreground
		Mat pbg = new Mat(); // probable background
		Mat pfg = new Mat(); // probable foreground

		Mat ones = Mat.ones(mask.size(),mask.type());
		Mat temp = new Mat();

		//		Core.multiply(bg,new Scalar(100), temp);
		//		displayImageSimple(temp);


		Core.multiply(ones, new Scalar(Imgproc.GC_FGD), temp);
		Core.compare(mask, temp, fg,Core.CMP_EQ);

		//		Core.multiply(fg,new Scalar(100), temp);
		//		displayImageSimple(temp);


		Core.multiply(ones, new Scalar(Imgproc.GC_PR_BGD), temp);
		Core.compare(mask, temp, pbg,Core.CMP_EQ);

		//		Core.multiply(pbg,new Scalar(100), temp);
		//		displayImageSimple(temp);


		Core.multiply(ones, new Scalar(Imgproc.GC_PR_FGD), temp);
		Core.compare(mask, temp, pfg,Core.CMP_EQ);
		//		
		//		Core.multiply(pfg,new Scalar(100), temp);
		//		displayImageSimple(temp);


		mask.setTo(new Scalar(Imgproc.GC_BGD),fg);
		masterMask = mask.clone();
		
		Core.multiply(ones, new Scalar(Imgproc.GC_BGD), temp);
		Core.compare(mask, temp, bg,Core.CMP_EQ);

		
		Mat a = original.clone();
		//ones = Mat.ones(original.size(), original.type());
		Core.add(a, new Scalar(0,0,100),a,bg);
		Core.add(a, new Scalar(0,0,100), a, pbg);
		Core.add(a, new Scalar(0,100,0),a, pfg);

		setupGUIForEdits(a);

		//Core.multiply(bg,new Scalar(100), temp);
		//displayImageSimple(a);
		//Highgui.imwrite("./processed_images/colorWhaleSegmented1.jpg", a);

		//		Mat b = new Mat();
		//		Core.multiply(mask, new Scalar(50), b);
		//		displayImageSimple(b);

		//		Mat a = original.clone();
		//		a=a.setTo(new Scalar(0,0,0),mask);  
		//		displayImageSimple(a);
		//displayOpenCVImgMat(a);
	}

	public static void displayImageSimple(Mat m){
		Mat rs = new Mat();
		double sf;
		double sfh, sfv;
		sfh = 0.9*screenWidth/m.cols();
		sfv = 0.9*screenHeight/m.rows();

		if(sfh<sfv){
			sf = sfh;
		}else{
			sf = sfv;
		}
		Imgproc.resize(m, rs, new Size(m.cols()*sf,m.rows()*sf));


		JFrame f1 = new JFrame();
		f1.setBounds(0, 0, (int)(screenWidth*0.9), (int)(screenHeight*0.9));
		f1.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		MyCanvas pic = new MyCanvas(mat2BuffImg(rs));
		f1.getContentPane().add(pic);
		f1.setVisible(true);		


	}

	public static void reRender(){ // now with masks
		Mat scaledImg = new Mat();
		Mat temp = baseImage.clone();

		temp.setTo(new Scalar(0,0,0), bgEditMask);
		temp.setTo(new Scalar(255,255,255),fgEditMask);

		Size newSize = new Size(baseImage.cols()*currSF,baseImage.rows()*currSF);


		Imgproc.resize(temp, scaledImg, newSize);
		BufferedImage img = mat2BuffImg(scaledImg);
		int x = (int)(currCenterX-baseCenterX);
		int y = (int)( currCenterY - baseCenterY);
		int dx = (int)(2*baseCenterX);
		int dy = (int)(2*baseCenterY);

		if(x<0){
			currCenterX = baseCenterX;
		}
		if(y<0){
			currCenterY = baseCenterY;
		}
		if(x+dx>img.getWidth()){
			currCenterX = img.getWidth() - (dx/2+1);
		}
		if(y+dy>img.getHeight()){
			currCenterY = img.getHeight() - (dy/2+1);
		}

		BufferedImage cropped = img.getSubimage((int)(currCenterX-baseCenterX), (int)( currCenterY - baseCenterY), (int)(2*baseCenterX), (int)(2*baseCenterY));

		canvas.updateImage(cropped);
	}

	public static void updateScale(int sliderValue){
		double oldSF = currSF;
		currSF = (sliderValue/20.0 + 1)*baseSF; //will range from 1x to 6x zoom
		currCenterX = currCenterX * currSF/oldSF;
		currCenterY = currCenterY * currSF/oldSF;
		reRender();		
	}
	public static void shiftCenter(double centerShiftX, double centerShiftY){
		currCenterX = currCenterX + centerShiftX;
		currCenterY = currCenterY + centerShiftY;
		reRender();
	}

	public static Mat createRectMaskMat(Mat m, int xt, int yt, int xb, int yb){
		Mat rectMask = Mat.zeros(m.rows(),m.cols(),CvType.CV_8U);
		//positions come in in terms of pixels of the image window. Use the scale factor to find real positions.
		int rxt = (int) (xt/currSF);
		int ryt = (int)(yt/currSF);
		int rxb = (int) (xb/currSF);
		int ryb = (int)(yb/currSF);
		//byte ones[] = {1,1,1};
		out.println("sf: " + currSF + " rxt: "+rxt + " ryt: "+ ryt + " rxb: "+rxb+" ryb: "+ryb);

		//for test
		//				for(int i = 1000; i <= 3000; i++){
		//					for(int j =2000; j <= 3000;j++){
		//						rectMask.put(i, j, 255);
		//					}
		//				}

		for(int i = rxt; i <= rxb;i++){
			for(int j = ryt; j <=ryb;j++){
				rectMask.put(j, i, 1);
			}
		}


		//				for(int i = rxt; i <= rxb;i++){
		//					rectMask.put(ryt, i, 255);
		//					rectMask.put(ryb, i, 255);
		//				}
		//				for(int j = ryt; j<= ryb;j++){
		//					rectMask.put(j, rxt, 255);
		//					rectMask.put(j, rxb, 255);
		//				}		
		return rectMask;
	}

	static int clickCount = 0;

	public static void clickResponse (int x, int y){
		clickCount++;
		if(clickCount % 2 == 1){
			xtop = x;
			ytop = y;
		}else{
			xbot = x;
			ybot = y;
			Mat mask =  createRectMaskMat(original,xtop,ytop,xbot,ybot);
			//rectangleMask = mask;
			//Mat zeros = Mat.zeros(original.rows(), original.cols(),original.type());
			Mat a = original.clone();
			Mat c = Mat.zeros(original.rows(),original.cols(), original.type());
			c = c.setTo(new Scalar (40,40,40),mask);

			Mat rectMasked =  new Mat();
			Core.add(a,c,rectMasked);

			Mat scaled = new Mat();
			Size newSize = new Size(rectMasked.cols()*baseSF,rectMasked.rows()*baseSF);
			Imgproc.resize(rectMasked, scaled, newSize);
			canvas.updateImage(mat2BuffImg(scaled));
			out.println("rectangle drawn");
		}		
	}

	public static void initChangeListener(JSlider slider){
		slider.addChangeListener(new ChangeListener() {
			public void stateChanged(ChangeEvent evt) {
				JSlider slider = (JSlider) evt.getSource();
				if (!slider.getValueIsAdjusting()) {
					int value = slider.getValue();
					updateScale(value);
				}
			}
		});
	}

	public static BufferedImage mat2BuffImg(Mat m){
		MatOfByte matOfByte = new MatOfByte();
		Highgui.imencode(".png",m,matOfByte);
		byte[] byteArray = matOfByte.toArray();
		BufferedImage image = null;
		try{
			InputStream in = new ByteArrayInputStream(byteArray);
			image = ImageIO.read(in);
		}catch(Exception e){
			e.printStackTrace();
		}
		return image;
	}

	public static void setupKeyBindings(JPanel p){
		UpAction upAction = new UpAction();
		p.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke('w'), "moveup");
		p.getActionMap().put("moveup", upAction);

		DownAction downAction = new DownAction();
		p.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke('s'), "movedown");
		p.getActionMap().put("movedown", downAction);

		LeftAction leftAction = new LeftAction();
		p.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke('a'), "moveleft");
		p.getActionMap().put("moveleft", leftAction);

		RightAction rightAction = new RightAction();
		p.getInputMap(JComponent.WHEN_IN_FOCUSED_WINDOW).put(KeyStroke.getKeyStroke('d'), "moveright");
		p.getActionMap().put("moveright", rightAction);

	}

	public static void setupGUIForEdits( Mat m){ // m should already be masked
		baseImage = m;
		Mat scaledImage = new Mat();
		Imgproc.resize(m, scaledImage, new Size(m.cols()*baseSF,m.rows()*baseSF));
		BufferedImage bi = mat2BuffImg(scaledImage);

		//initialize edit masks
		bgEditMask = Mat.zeros(m.rows(),m.cols(),CvType.CV_8U);
		fgEditMask = bgEditMask.clone();


		JFrame window = new JFrame();
		window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		window.setBounds(0, 0, screenWidth, screenHeight);

		// main panel
		JPanel mainPane = new JPanel();
		mainPane.setLayout(new BoxLayout(mainPane,BoxLayout.PAGE_AXIS));

		//the slider
		JPanel sliderPanel = new JPanel(new BorderLayout());	
		JSlider scaleFactor = new JSlider(0,100,0);
		initChangeListener(scaleFactor);
		sliderPanel.add(scaleFactor, BorderLayout.NORTH);
		sliderPanel.setMaximumSize(new Dimension((int)Math.round(screenWidth*.5),(int)Math.round(screenHeight*.2)));

		//buttons 
		JButton BGButton = new JButton("Background");
		BGButton.setActionCommand("background button clicked");
		BGButton.addActionListener(new DrawBackground() );

		JButton FGButton = new JButton("Foreground");
		FGButton.setActionCommand("foreground button clicked");
		FGButton.addActionListener(new DrawForeground());

		JButton eraseButton = new JButton("erase");
		eraseButton.setActionCommand("erase");
		eraseButton.addActionListener(new Erase());

		JButton doneButton = new JButton("Re-Run");
		doneButton.setActionCommand("done");
		doneButton.addActionListener(new MarkupDone());
		
		JButton reallyDoneButton = new JButton("That's It!");
		reallyDoneButton.setActionCommand("whale detected");
		reallyDoneButton.addActionListener(new Finish());

		JPanel buttonPanel = new JPanel(new FlowLayout());
		buttonPanel.add(BGButton);
		buttonPanel.add(FGButton);
		buttonPanel.add(eraseButton);
		buttonPanel.add(doneButton);
		buttonPanel.add(reallyDoneButton);

		sliderPanel.add(buttonPanel,BorderLayout.CENTER);



		canvas = new MyCanvas(bi);
		canvas.addMouseMotionListener(new EditMouseMotionListener());
		canvas.addMouseListener(new EditMouseReleaseListener());
		JPanel picturePanel = new JPanel(new BorderLayout());
		picturePanel.add(canvas,BorderLayout.CENTER);
		//		picturePanel.setMaximumSize(new Dimension((int)Math.round(screenWidth*.9),(int)Math.round(screenHeight*0.8)));
		picturePanel.setMaximumSize(new Dimension(bi.getWidth(),bi.getHeight()));
		//
		//		baseCenterX = bi.getWidth()/2.0;
		//		baseCenterY = bi.getHeight()/2.0;
		//		currCenterX = baseCenterX;
		//		currCenterY = baseCenterY;


		mainPane.add(picturePanel, BorderLayout.NORTH);
		mainPane.add(sliderPanel, BorderLayout.SOUTH);

		setupKeyBindings(mainPane);

		window.getContentPane().add(mainPane);
		window.setVisible(true);		
	}

	public static void displayOpenCVImgMat(Mat m){
		//
		//		// screen size stuff
		//		GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
		//		GraphicsDevice gs = ge.getDefaultScreenDevice();
		//		DisplayMode[] dmodes = gs.getDisplayModes();
		//
		//		for (int i = 0; i < dmodes.length; i++) {
		//			screenWidth = dmodes[i].getWidth();
		//			screenHeight = dmodes[i].getHeight();
		//		}
		//		baseImage = m;
		//
		//		double sf = (screenWidth*0.9)/m.cols();
		//		baseSF = sf;
		//		currSF = sf;
		//		Size imgSize = new Size(m.cols()*sf, m.rows()*sf);
		//		Mat rImg = new Mat();
		//		Imgproc.resize(m, rImg, imgSize);
		//
		//		BufferedImage bi = mat2BuffImg(rImg);
		//		setupGUIForEdits(bi);
		//		out.println("image displayed");
	}

	public static void setupGUIforRect(BufferedImage bi){
		JFrame window = new JFrame();
		window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		window.setBounds(0, 0, screenWidth, screenHeight);

		// main panel
		JPanel mainPane = new JPanel();
		mainPane.setLayout(new BoxLayout(mainPane,BoxLayout.PAGE_AXIS));

		//the buttons

		JButton happyButton = new JButton("Happy");
		happyButton.setActionCommand("done");
		happyButton.addActionListener(new RectGood());


		//button panel
		JPanel buttonPanel = new JPanel(new BorderLayout());
		buttonPanel.add(happyButton,BorderLayout.EAST);
		buttonPanel.setMaximumSize(new Dimension((int)Math.round(screenWidth*.5),(int)Math.round(screenHeight*.1)));


		canvas = new MyCanvas(bi);	
		JPanel picturePanel = new JPanel(new BorderLayout());
		picturePanel.add(canvas,BorderLayout.CENTER);
		picturePanel.setMaximumSize(new Dimension(bi.getWidth(),bi.getHeight()));
		canvas.addMouseListener(new RectMouseListener());
		//mouse listener 


		baseCenterX = bi.getWidth()/2.0;
		baseCenterY = bi.getHeight()/2.0;
		currCenterX = baseCenterX;
		currCenterY = baseCenterY;

		mainPane.add(picturePanel, BorderLayout.NORTH);
		mainPane.add(buttonPanel, BorderLayout.SOUTH);

		window.getContentPane().add(mainPane);
		window.setVisible(true);	

	}

	public static void UIgetRectangle(Mat m){

		// screen size stuff
		GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
		GraphicsDevice gs = ge.getDefaultScreenDevice();
		DisplayMode[] dmodes = gs.getDisplayModes();

		for (int i = 0; i < dmodes.length; i++) {
			screenWidth = dmodes[i].getWidth();
			screenHeight = dmodes[i].getHeight();
		}
		Mat temp = new Mat();
		Imgproc.resize(m, temp, new Size(3000, m.rows()*3000.0/m.cols()));
		original = temp.clone();
		m = temp;
		
		
		
		Mat temp2 = new Mat();
		Imgproc.resize(m, temp2, new Size(300, m.rows()*300.0/m.cols()));
		//Imgproc.resize(temp2,temp2,m.size());
		lowRes = temp2.clone();
		
		
		out.println(m.cols());
		out.println(m.rows());

		baseImage = m;
		double sf;
		double sf1 = (screenWidth*0.9)/m.cols();
		double sf2 = (screenHeight*0.8)/m.rows();
		if(sf1<sf2){
			sf = sf1;
		}else{
			sf = sf2;
		}

		baseSF = sf;
		currSF = sf;

		Size imgSize = new Size(m.cols()*sf, m.rows()*sf);
		Mat rImg = new Mat();
		Imgproc.resize(m, rImg, imgSize);

		BufferedImage bi = mat2BuffImg(rImg);
		setupGUIforRect(bi);
	}


	public static void main( String[] args )
	
	{
		System.loadLibrary(Core.NATIVE_LIBRARY_NAME);
		
		sourceDir = "./images/eval/";
		resultDir ="./processed_images/eval/";
		
		name = "w1_e";
		
		String readString = sourceDir + name + ".JPG";
		out.println(readString);
		Mat img = Highgui.imread(readString , Highgui.CV_LOAD_IMAGE_COLOR);
		out.println(img.type());
		out.println(CvType.CV_8UC3);
//		final JFileChooser fc = new JFileChooser();
//		JFrame f = new JFrame();
//		f.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
//		f.setVisible(true);
//		int returnVal = fc.showOpenDialog(f);
		out.println(Imgproc.GC_BGD + " "+ Imgproc.GC_FGD+ " " + Imgproc.GC_PR_BGD+ " "+ Imgproc.GC_PR_FGD);
		//displayOpenCVImgMat(img);
		UIgetRectangle(img);
		//		Mat edges = img.clone();
		//		Imgproc.Canny(img,edges,100,200);
		//		show(edges);
		//		Highgui.imwrite("./processed_images/uglyEdges.jpg", edges);

		//Mat denoisedImg = Highgui.imread("C:/Users/Dmitriy/workspace/Testing open CV/images/beagle.jpg" , Highgui.CV_LOAD_IMAGE_GRAYSCALE);
		out.println("hello");
		// out.println();

		//		BufferedImage imgB = Mat2BufferedImage(img);
		//		displayImage(imgB);
		//	   displayImage(imgB);
		//	   out.println("still kicking");
	}
}


//{
//	public static BufferedImage Mat2BufferedImage(Mat m){
//		// source: http://answers.opencv.org/question/10344/opencv-java-load-image-to-gui/
//		// Fastest code
//		// The output can be assigned either to a BufferedImage or to an Image
//
//		int type = BufferedImage.TYPE_BYTE_GRAY;
//		if ( m.channels() > 1 ) {
//			type = BufferedImage.TYPE_3BYTE_BGR;
//		}
//		int bufferSize = m.channels()*m.cols()*m.rows();
//		byte [] b = new byte[bufferSize];
//		m.get(0,0,b); // get all the pixels
//		BufferedImage image = new BufferedImage(m.cols(),m.rows(), type);
//		final byte[] targetPixels = ((DataBufferByte) image.getRaster().getDataBuffer()).getData();
//		System.arraycopy(b, 0, targetPixels, 0, b.length);  
//		return image;
//
//	}
//
//	public static void displayImage(Image img2)
//	{   
//		//BufferedImage img=ImageIO.read(new File("/HelloOpenCV/lena.png"));
//		ImageIcon icon=new ImageIcon(img2);
//		JFrame frame=new JFrame();
//		frame.setLayout(new FlowLayout());        
//		frame.setSize(img2.getWidth(null)+50, img2.getHeight(null)+50);     
//		JLabel lbl=new JLabel();
//		lbl.setIcon(icon);
//		frame.add(lbl);
//		frame.setVisible(true);
//		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
//		frame.setResizable(true);
//
//	}