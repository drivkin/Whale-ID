
#include <opencv2/imgproc/imgproc.hpp>

#include <stdlib.h>
#include "wtypes.h"
#include <stdio.h>
#include <iostream>

#include "opencv2/core/core.hpp"
#include "math.h"
//#include "opencv2/features2d/features2d.hpp"
//#include "opencv2/imgcodecs/imgcodecs.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"

//#include "stdafx.h"





using namespace std;
using namespace cv;

#define clr
#define AFFINE_WARP
//#define NO_ZERO
//#define INCLUDE_EDGES

#ifdef clr
#include <fstream>
#include <sstream>
#include <string>
#include <msclr\marshal_cppstd.h>
using namespace System;
using namespace System::IO;
#endif

Mat getLBPHist(Mat img,int thresh);

void GetDesktopResolution(int& horizontal, int& vertical)
{
   RECT desktop;
   // Get a handle to the desktop window
   const HWND hDesktop = GetDesktopWindow();
   // Get the size of screen to the variable desktop
   GetWindowRect(hDesktop, &desktop);
   // The top left corner will have coordinates (0,0)
   // and the bottom right corner will have coordinates
   // (horizontal, vertical)
   horizontal = desktop.right;
   vertical = desktop.bottom;
}

Mat templateMatch(Mat ref, Mat can, int tSize){
	Rect myROI(917,185,tSize,tSize); // later will do a bunch, now just for test
	Mat tmp = can(myROI);
	imshow("template window",tmp);
	imwrite("../images/template.jpg",tmp);

	int result_cols =  ref.cols - tmp.cols + 1;
	int result_rows = ref.rows - tmp.rows + 1;

	Mat result;
	result.create( result_cols, result_rows, CV_32FC1 );
	matchTemplate(ref,tmp,result,TM_CCOEFF_NORMED);

	double minVal; double maxVal; Point minLoc; Point maxLoc;
	Point matchLoc;
	minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
	cout<<minVal<<"  "<< maxVal<< "  " << maxLoc.x << " " << maxLoc.y;
	Rect myROI2(maxLoc.x,maxLoc.y,tmp.cols,tmp.rows);
	Mat refMatch = ref(myROI2);
	imshow("the match", refMatch);


	Mat A =  result.clone();
	A = A - minVal;
	Mat B;
	A.convertTo(B,CV_8U,255.0/(maxVal-minVal));

	imshow("result wind",B);
	imwrite("../images/result.jpg",B);
	return result;
}

Point getV(Mat img){
	Mat cT;
	transpose(img,cT);

	int nRows = cT.rows;
	int nCols = cT.cols;
	int j = 0;

	uchar* p;
	int found = 0;
	int yPoint = 0;
	int xpoint = 0;
	for(int i = (int)(0.2*nRows) ; i<(int)(0.8*nRows);i++){
	p = cT.ptr<uchar>(i);
	found = 0;
	j = 0;
		while(!found && j<nCols){
			if(p[j] > 0){
				if(j>yPoint){
					yPoint = j;
					xpoint = i;
				}
				found  = 1;
			}
			j++;
		}
	}
	Point v(xpoint,yPoint);
	return v;
}

Point getTopY(Mat img){
	Mat cT;
	cT = img.clone();

	int nRows = cT.rows;
	int nCols = cT.cols;
	int j = 0;

	uchar* p;
	int found = 0;
	int yPoint = 0;
	int xpoint = 0;
	found = 0;
	for(int i = 0 ; i<nRows;i++){
	p = cT.ptr<uchar>(i);
	j = 0;
		while(!found && j<nCols){
			if(p[j] > 0){
				found  = 1;
				yPoint = i;
				xpoint = j;
			}
			j++;
		}
	}

	Point pt(xpoint,yPoint);
	return pt;
}

Mat getHistograms(Mat img, int splitx, int splity, int thresh){
	if(!splitx && ! splity ){
		return getLBPHist(img, thresh);
	}else{
		Mat histograms  = Mat::zeros( splitx*splity, 256, CV_32FC1 );
		int rowCount = 0;
		for(int i = 0; i<splitx; i++){
			for(int j = 0; j<splity; j++){
				Rect myROI(i*img.cols/splitx,j*img.rows/splity,img.cols/splitx-1,img.rows/splity-1);
				Mat hist = getLBPHist(img(myROI), thresh);
				hist.row(0).copyTo(histograms.row(rowCount));
				rowCount++;
			}
		}

		//Mat l;
		//Mat r;
		//Rect LROI(0,0,img.cols/2,img.rows);
		//Rect RROI(img.cols/2,0,img.cols/2-1, img.rows);
		//l = img(LROI);
		//r = img(RROI);
		//Mat histograms  = Mat::zeros( 2, 256, CV_32FC1 );
		//Mat lhist = getLBPHist(l);
		//Mat rhist = getLBPHist(r);
		//lhist.row(0).copyTo(histograms.row(0));
		//rhist.row(0).copyTo(histograms.row(1));
		//histograms.row(0) = lhist.row(0);
		//histograms.row(1) = rhist.row(0);
		return histograms;
	}
}

Mat getHistogramsSmartSplit(Mat img){ //using the v and the tips
	Point V = getV(img);
	int xV = V.x;
	Rect r0(0,0,xV,img.rows);
	Rect r1(xV,0,img.cols-xV,img.rows);
	Mat l = img(r0);
	Mat r = img(r1);
	
	Mat histograms = Mat::zeros( 2, 256, CV_32FC1 ); 
	Mat hist = getLBPHist(l,0);
	hist.row(0).copyTo(histograms.row(0));
	hist = getLBPHist(r,0);
	hist.row(0).copyTo(histograms.row(1));
	
	//int ytl = getTopY(l);
	//int ytr = getTopY(r);
	//vector<Mat> splits;
	//Rect r2(0,ytl,l.cols,(l.rows-ytl)/2);
	//Rect r3(0,ytl+(l.rows-ytl)/2,l.cols,(l.rows-ytl)/2-1);
	//Rect r4(0,ytr,r.cols,(r.rows-ytr)/2);
	//Rect r5(0,ytr+(r.rows-ytr)/2,r.cols,(l.rows-ytr)/2-1);
	//splits.push_back(l(r2));
	//splits.push_back(l(r3));
	//splits.push_back(r(r4));
	//splits.push_back(r(r5));

	//imshow("tl",splits[0]);
	//imshow("bl",splits[1]);
	//imshow("tr",splits[2]);
	//imshow("br",splits[3]);
	//waitKey(0); // Wait for a keystroke in the window


	//Mat histograms = Mat::zeros( 4, 256, CV_32FC1 ); 
	//for(int i = 0; i<splits.size(); i++){
	//	Mat hist = getLBPHist(splits[i]);
	//	hist.row(0).copyTo(histograms.row(i));
	//}
	return histograms;
}

Mat getTopStrip(Point v, Point top, int depth, int width){
	double length = sqrt((double)((v.x-top.x)^2 + (v.y-top.y)^2));
	cout<< length;
	double costheta = (v.x-top.x)/length;
	double theta = acos(costheta);
	
	if(v.x<top.x){
		theta = -theta;
	}
	Mat r = getRotationMatrix2D(v,theta,width/length);
	return r; // change this
}

Mat getHistogramsLineSplit(Mat img){ // draw a line from the v to the tip, rotate so that it is horizontal, then take a rectange down from it
	Point V = getV(img);
	int xV = V.x;
	Rect r0(0,0,xV,img.rows);
	Rect r1(xV,0,img.cols-xV,img.rows);
	Mat l = img(r0);
	Mat r = img(r1);
	Mat lStrip = getTopStrip(V,getTopY(l),100,750);
	imshow("lstrip",lStrip);
	return lStrip;
}

Mat autoCrop(Mat m){
	uchar* p;
	p = m.data;
	int nRows = m.rows;
	int nCols = m.cols;

	int ty = m.rows;
	int by = 0;
	int lx = m.cols;
	int rx = 0;

	for(int i = 0; i<m.rows;i++){
		p = m.ptr<uchar>(i);
		for(int j = 0; j<m.cols; j++){
			if(p[j]> 0){
				if(i<ty){
					ty=i;
				}
				if(i>by){
					by=i;
				}
				if(j<lx){
					lx=j;
				}
				if(j>rx){
					rx = j;
				}
			}
		}
	}
	
	Rect myROI(lx,ty,rx-lx,by-ty);
	Mat cropped = m(myROI);
	return cropped;
}


void getKeyTriangle(Mat img, Point2f* points){
	Point v  = getV(img);
	points[0] = v;
	int xV = v.x;
	Rect r0(0,0,xV,img.rows);
	Rect r1(xV,0,img.cols-xV,img.rows);
	Mat l = img(r0);
	//imshow("left",l);
	//imwrite("../images/left.jpg",l);
	Mat r = img(r1);
	//imshow("right",r);
	Point lt = getTopY(l);
	Point rt = getTopY(r);
	points[1] = lt;
	rt.x = rt.x + v.x;
	points[2] = rt;
}

Mat cleverTransformToStandard(Mat img, Mat standard){
	Point2f points[3];
	getKeyTriangle(img,points);
	double lv[2];
	lv[0] = points[1].x - points[0].x; //center minus left tip
	lv[1] = points[1].y - points[0].y;

	double lcv[2];
	lcv[1] = sqrt(1/(1+(pow(lv[1],2)/(pow(lv[0],2)))));
	lcv[0] = sqrt(1-pow(lcv[1],2));

	double length = sqrt(pow(lv[0],2)+pow(lv[1],2));

	Point2f tPts[3];
	tPts[0] = points[0];
	tPts[1] = points[1];
	
	double rat = length/standard.cols*standard.rows;

	Point2f third;
	third.x = tPts[1].x+rat*lcv[0];
	third.y = tPts[1].y+rat*lcv[1];

	
	cout << "vec x : " << lv[0] << " vec y: " << lv[1]<< "\n";
	cout<< "alpha : " << lcv[0] << " beta: " << lcv[1]<<"\n";
	cout<< "ratio: " << rat<<"\n";
	cout<<"tl x: "<< tPts[1].x << "tl y: " << tPts[1].y << "\n";
	cout<<"third x: " <<third.x << "third y: " << third.y<<"\n";

	tPts[2] = third;
	
	Point2f dstPts[3];

	Point vs(standard.cols,0);
	Point tl(0,0);
	Point thrd(0,standard.rows);
	
	dstPts[0] = vs;
	dstPts[1] = tl;
	dstPts[2] = thrd;

	Mat trns = getAffineTransform(tPts,dstPts);
	
	Mat warped;
	warpAffine(img,warped,trns,standard.size());

	imshow("warped",warped);
	return warped;
}

void outputKeypoints(Point2f* p){
	cout<< "Vx Vy: "<<p[0].x<<" " << p[0].y<< " ";
	cout<< "tlx tly: "<<p[1].x<<" " << p[1].y<< " ";
	cout<<"trx try: "<<p[2].x<<" " << p[2].y<< "\n";
}

Mat affineTransform(Mat can, Mat ref){
	// calculate an affine transform which should pretty much overlay the can over the ref
	Point2f srcTri[3];
	Point2f dstTri[3];
	
	getKeyTriangle(ref,dstTri);
	getKeyTriangle(can,srcTri);

	//cout<< "dest: ";
	//outputKeypoints(dstTri);

	//cout<< " source: ";
	//outputKeypoints(srcTri);
	
	Mat trns = getAffineTransform(srcTri,dstTri);
	Mat warped;
	Size s(4000,1000);
	warpAffine(can,warped,trns,s);
	Mat cropped = autoCrop(warped);
	//imshow("transformed",cropped);
	//imshow("ref",ref);
	//imshow("ref",ref);
	//imwrite("../images/transformed.jpg",cropped);
	//imwrite("../images/ref.jpg",ref);
	return cropped;
}

Mat getLBPHist(Mat img, int thresh){ // img should be gray & masked
	// 8 neighbors
	//cout<< "rows: " << img.rows << " cols: "<< img.cols<< " getting hist \n";
	Mat hist;
	hist = Mat::zeros( 1, 256, CV_32FC1 );
	float* hpt = hist.ptr<float>(0);
	uchar* p;
	uchar* up;
	uchar* dn;
	for(int i = 1; i<img.rows - 1; i++){
	up = img.ptr<uchar>(i-1);
	p = img.ptr<uchar>(i);
	dn = img.ptr<uchar>(i+1);

		for(int j = 1; j< img.cols - 1; j++){
			uchar center = p[j];
			uchar tl = up[j-1];
			uchar t = up[j];
			uchar tr = up[j+1];
			uchar l = p[j-1];
			uchar r = p[j+1];
			uchar bl = dn[j-1];
			uchar b = dn[j];
			uchar br = dn[j+1];
			//cout<< center;
#ifndef INCLUDE_EDGES
			if(center && tl && t && tr && l && r && bl && b && br){ // img should be set up so that only values pixels not in tail are zero
#endif
#ifdef INCLUDE_EDGES
				if(center){
#endif
				//cout<< " not border\n";
				uchar code = 0;
				//it goes clockwise from top left
				code |= (tl > center + thresh) << 7;
				code |= (t> center + thresh) << 6;
				code |= (tr > center + thresh) << 5;
				code |= (r > center + thresh) << 4;
				code |= (br> center + thresh) << 3;
				code |= (b> center+thresh) << 2;
				code |= (bl > center+thresh) << 1;
				code |= (l> center+thresh) << 0;

#ifndef NO_ZERO
				hpt[code]+=1;
#endif
#ifdef NO_ZERO
				if(code != 0){
					hpt[code]+=1;
				}
#endif
			}
		}
	}

		// now normalize the histogram
		float tot = 0;
		for(int i = 0;i<256;i++){
		//	cout<<hpt[i]<<"\n";
			tot += hpt[i];
		}

		//cout<<"\n"<<tot<<"\n";

		hist = hist/tot;
		return hist;
}

//void facial(void){
//	vector<Mat> trainingSet;
//	vector<int> trainingSetLabels;
//
//	vector<Mat> verSet;
//	vector<int> verSetTrueLabels;
//	vector<int> verSetGuessedLabels;
//
//
//
//	System::String^ baseDir = "C:\\Users\\Dmitriy\\Desktop\\whales\\";
//	array<System::String^>^ subDirs =  System::IO::Directory::GetDirectories(baseDir);
//	//load up all the pictures, the first one from every whale gets to be verification set, the rest are training set
//	for(int i=0; i < subDirs->Length ;i++){ // currently just takes the first 10, just to test
//		array<System::String^>^ pics =  System::IO::Directory::GetFiles(subDirs[i]);
//		if(pics->Length > 5){
//			for(int j = 0; j< pics->Length; j++){
//				msclr::interop::marshal_context context;
//				std::string stdStr = context.marshal_as<std::string>(pics[j]);
//				cout<< "dir: " << i << "pic: " << j << "\n";
//				if(j==0){
//					verSet.push_back(imread(stdStr,IMREAD_GRAYSCALE));
//					verSetTrueLabels.push_back(i);
//					verSetGuessedLabels.push_back(-1);
//				}
//				else{
//					trainingSet.push_back(imread(stdStr,IMREAD_GRAYSCALE));
//					trainingSetLabels.push_back(i);
//				}
//			}
//		}
//	}
//
////Mat m = imread(stdStr,IMREAD_GRAYSCALE);
////imshow("blah",m);
////
////	Mat unknownWhale = imread("../images/w4_2cutout.jpg",IMREAD_GRAYSCALE);
////
////	images.push_back(imread("../images/w2_1cutout.jpg",IMREAD_GRAYSCALE));
////	images.push_back(imread("../images/w2_2cutout.jpg",IMREAD_GRAYSCALE));
////	images.push_back(imread("../images/w3_1cutout.jpg",IMREAD_GRAYSCALE));
////	images.push_back(imread("../images/w3_2cutout.jpg",IMREAD_GRAYSCALE));
////	images.push_back(imread("../images/w4_1cutout.jpg",IMREAD_GRAYSCALE));
////	//images.push_back(imread("../images/w4_2cutout.jpg",IMREAD_GRAYSCALE));
////
////	labels.push_back(2);
////	labels.push_back(2);
////	labels.push_back(3);
////	labels.push_back(3);
////	labels.push_back(4);
////	//labels.push_back(4);
////
//	Ptr<FaceRecognizer> model = createLBPHFaceRecognizer();
//	model->train(trainingSet,trainingSetLabels);
//
//	for(int k = 0; k< verSet.size(); k++){
//		int prediction;
//		double confidence;
//		model->predict(verSet[k],prediction,confidence);
//		 verSetGuessedLabels[k] = prediction;
//		cout<<"guessed: "<< verSetGuessedLabels[k]<< " true: " << verSetTrueLabels[k]<< " confidence: " << confidence<< "\n";
//	}
//
//	vector<Mat> histograms = model->getMatVector("histograms");
//	cout << "Size of the histograms: " << histograms[0].total() << endl;
//	imshow("a histogram",histograms[23]);
//}
//
//
// 


Mat drawLBPHist(Mat histogram){
	Mat histImg;
	histImg = Mat::zeros(500,histogram.cols,CV_8U);
	float* hptr;
	hptr = histogram.ptr<float>(0);
	for(int i = 0; i<histogram.cols;i++){
		circle(histImg,Point(i,(int)(hptr[i]*2000)),4,Scalar(255));
	}

	return histImg;
}

int guessWhale(Mat uHist, vector<Mat> kHists, vector<int> labels){
	if(uHist.rows == 1){
		double maxCorr = 0;
		double corr;
		int bestGuess;
		for(int i = 0; i < kHists.size() ;i++){
			corr = compareHist(uHist,kHists[i], CV_COMP_CORREL); // correll pretty good chisq bad intersect bad  bhatta bad
			if(corr > maxCorr){
				maxCorr = corr;
				bestGuess = labels[i];
			}
			cout<< corr << "\n";
		}
		return bestGuess;
	}else{
		double maxCorr = 0;
		double corr;
		int bestGuess;
		for(int i = 0; i < kHists.size() ;i++){
			corr = 0;
			for(int j = 0; j<uHist.rows;j++){
				corr += compareHist(uHist.row(j),kHists[i].row(j), CV_COMP_CORREL); // correll pretty good chisq bad intersect bad  bhatta bad
			}
			if(corr > maxCorr){
				maxCorr = corr;
				bestGuess = labels[i];
			}
			cout<< corr << "\n";
		}
		return bestGuess;
	}
}


void main(void){
Mat ref = imread("../images/best_clean/w4/best_cleancut.jpg",IMREAD_GRAYSCALE);
Mat can = imread("../images/eval_clean/w4/eval_cleancut.jpg",IMREAD_GRAYSCALE);

//Mat standard = Mat::zeros(200,700,CV_8U);
Mat warped = affineTransform(can,ref);
imshow("for slides",warped);
imwrite("..images/warped_for_slides.jpg",warped);
//Mat warp  = cleverTransformToStandard(ref,standard);
//
////imwrite("../images/ref.jpg",warp);
//
//	//vector<Mat> images;
//	//vector<int> labels;
//
//	//vector <Mat> evalImages;
//	//vector <int> evalLabels;
//	//	//Mat unknownWhale = imread("../images/whales/w3/w3_2cutout.jpg",IMREAD_GRAYSCALE);
//
//	//	//images.push_back(imread("../images/whales/w2/w2_1cutout.jpg",IMREAD_GRAYSCALE));
//	//	//images.push_back(imread("../images/whales/w2/w2_2cutout.jpg",IMREAD_GRAYSCALE));
//	//	//images.push_back(imread("../images/whales/w3/w3_1cutout.jpg",IMREAD_GRAYSCALE));
//	//	////images.push_back(imread("../images/whales/w3/w3_2cutout.jpg",IMREAD_GRAYSCALE));
//	//	//images.push_back(imread("../images/whales/w4/w4_1cutout.jpg",IMREAD_GRAYSCALE));
//	//	//images.push_back(imread("../images/whales/w4/w4_2cutout.jpg",IMREAD_GRAYSCALE));
//
//	//	//labels.push_back(2);
//	//	//labels.push_back(2);
//	//	//labels.push_back(3);
//	//	////labels.push_back(3);
//	//	//labels.push_back(4);
//	//	//labels.push_back(4);

#ifndef AFFINE_WARP
	vector<Mat> images;
	vector<int> labels;

	vector <Mat> evalImages;
	vector <int> evalLabels;

	System::String^ baseDir = "..//images//best_clean";
	array<System::String^>^ subDirs =  System::IO::Directory::GetDirectories(baseDir);
	//load up all the pictures, the first one from every whale gets to be verification set, the rest are training set
	for(int i=0; i < subDirs->Length ;i++){ // currently just takes the first 10, just to test
		array<System::String^>^ pics =  System::IO::Directory::GetFiles(subDirs[i]);
		if(pics->Length > 0){
			for(int j = 0; j< pics->Length; j++){
				msclr::interop::marshal_context context;
				std::string stdStr = context.marshal_as<std::string>(pics[j]);
				cout<< "dir: " << i << "pic: " << j << "\n";
				Mat img  = imread(stdStr,IMREAD_GRAYSCALE);
				resize(img,img,Size(1500, (1500.0/img.cols)*img.rows));
				images.push_back(img);
				labels.push_back(i);
			}
		}
	}
	vector<Mat> knownHs;
	for(int i=0; i<images.size();i++){
		//knownHs.push_back(getHistograms(images[i],6,1));
		knownHs.push_back(getHistogramsSmartSplit(images[i]));
	}


	System::String^ evalDir = "..//images//eval_clean";
	array<System::String^>^ subDirs1 =  System::IO::Directory::GetDirectories(evalDir);

	for(int i=0; i < subDirs1->Length ;i++){ 
		array<System::String^>^ pics =  System::IO::Directory::GetFiles(subDirs1[i]);
		if(pics->Length > 0){
			for(int j = 0; j< pics->Length; j++){
				msclr::interop::marshal_context context;
				std::string stdStr = context.marshal_as<std::string>(pics[j]);
				cout<< "dir: " << i << "pic: " << j << "\n";
				Mat img  = imread(stdStr,IMREAD_GRAYSCALE);
				resize(img,img,Size(1500, (1500.0/img.cols)*img.rows));
				evalImages.push_back(img);
				evalLabels.push_back(i);
			}
		}
	}

	for(int i=0; i<images.size();i++){
		//Mat hist = getHistograms(evalImages[i],3,3);
		Mat hist = getHistogramsSmartSplit(evalImages[i]);
		int whaleNum = guessWhale(hist,knownHs,labels);
		cout<< "real whale: " << evalLabels[i]<< "guessed whale: "<< whaleNum<< "\n";
	}
#endif

#ifdef AFFINE_WARP
	vector<Mat> images;
	vector<int> labels;

	vector <Mat> evalImages;
	vector <int> evalLabels;

	System::String^ baseDir = "..//images//best_clean";
	array<System::String^>^ subDirs =  System::IO::Directory::GetDirectories(baseDir);
	//load up all the pictures, the first one from every whale gets to be verification set, the rest are training set
	for(int i=0; i < subDirs->Length ;i++){ // currently just takes the first 10, just to test
		array<System::String^>^ pics =  System::IO::Directory::GetFiles(subDirs[i]);
		if(pics->Length > 0){
			for(int j = 0; j< pics->Length; j++){
				msclr::interop::marshal_context context;
				std::string stdStr = context.marshal_as<std::string>(pics[j]);
				cout<< "dir: " << i << "pic: " << j << "\n";
				Mat img  = imread(stdStr,IMREAD_GRAYSCALE);
				resize(img,img,Size(1500, (1500.0/img.cols)*img.rows));
				images.push_back(img);
				labels.push_back(i);
			}
		}
	}
	vector<Mat> knownHs;
	for(int i=0; i<images.size();i++){
		knownHs.push_back(getHistograms(images[i],20,1,0));
		//knownHs.push_back(getHistogramsSmartSplit(images[i]));
	}

	System::String^ evalDir = "..//images//eval_clean";
	array<System::String^>^ subDirs1 =  System::IO::Directory::GetDirectories(evalDir);

	for(int i=0; i < subDirs1->Length ;i++){ 
		array<System::String^>^ pics =  System::IO::Directory::GetFiles(subDirs1[i]);
		if(pics->Length > 0){
			for(int j = 0; j< pics->Length; j++){
				msclr::interop::marshal_context context;
				std::string stdStr = context.marshal_as<std::string>(pics[j]);
				cout<< "dir: " << i << "pic: " << j << "\n";
				Mat img  = imread(stdStr,IMREAD_GRAYSCALE);
				resize(img,img,Size(1500, (1500.0/img.cols)*img.rows));
				evalImages.push_back(img);
				evalLabels.push_back(i);
			}
		}
	}

	double maxCorr = 0;
	double corr = 0;
	int bestGuess;
	for(int i = 0; i< evalImages.size();i++){
		maxCorr = 0;
		bestGuess = 0;
		for(int j= 0; j<images.size();j++){
			//cout<<j<<"\n";
			corr = 0;
			Mat warped = affineTransform(evalImages[i],images[j]);
			Mat uHist = getHistograms(warped,20,1,0);
			for(int k = 0; k<uHist.rows;k++){
				//cout<<k<<"\n";
				corr += compareHist(uHist.row(k),knownHs[j].row(k), CV_COMP_CORREL); // correll pretty good chisq bad intersect bad  bhatta bad
			}
			if(corr > maxCorr){
				maxCorr = corr;
				bestGuess = labels[j];
			}
			cout<< corr << "\n";
		}
		cout<<"real whale: " << evalLabels[i]<< " guessed whale: " << bestGuess<<"\n";
	}

#endif


//Mat w41, w42, w31, w32;
//w41 = imread("../images/whales/w4/w4_1cutout.jpg",IMREAD_GRAYSCALE);
////imshow("pic",ref);
// w42 = imread("../images/whales/w4/w4_2cutout.jpg",IMREAD_GRAYSCALE);
//w31 = imread("../images/whales/w3/w3_1cutout.jpg",IMREAD_GRAYSCALE);
//w32 = imread("../images/whales/w3/w3_2cutout.jpg",IMREAD_GRAYSCALE);
////Size newCanSize(ref.cols,int((ref.cols/((double)can.cols))*can.rows)); // resize to have the same width, but aspect ratio
////resize(can,can,newCanSize);
//	cout<< "starting\n";
//
//	cout<< "w4 1\n";
//	Mat hist0 = getLBPHist(w41);
//	Mat histImg0 = drawLBPHist(hist0);
//	imshow("w4 1\n",histImg0);
//
//	cout<<"w4_2\n";
//	Mat hist1 = getLBPHist(w42);
//	Mat histImg1 = drawLBPHist(hist1);
//	imshow("w4_2\n",histImg1);
//
//	cout<<"w3_1\n";
//	Mat hist2 = getLBPHist(w31);
//	Mat histImg2 = drawLBPHist(hist2);
//	imshow("w3_1\n",histImg2);
//
//	cout<<"w3_2\n";
//	Mat hist3 = getLBPHist(w32);
//	Mat histImg3 = drawLBPHist(hist3);
//	imshow("w3_2\n",histImg3);
//
//
//	double w4cmp = compareHist(hist0,hist1,CV_COMP_CORREL);
//	cout<<"\nwhale 41 to whale 42: "<< w4cmp;
//	double w3cmp = compareHist(hist2,hist3,CV_COMP_CORREL);
//	cout<<"\n whale 31 to whale 32: "<< w4cmp;
//
//	cout<<"\n whale 41 to whale 31: "<< compareHist(hist0,hist2,CV_COMP_CORREL);
//	cout<<"\n whale 42 to whale 31: "<< compareHist(hist1,hist2,CV_COMP_CORREL);
//
//	cout<<"\n whale 42 to whale 32: "<< compareHist(hist1,hist3,CV_COMP_CORREL);
//
//	cout<<"\n whale 41 to whale 41: "<< compareHist(hist0,hist0,CV_COMP_CORREL);
//
//facial();

//int template_size = can.cols / 6;
//Mat templateMatchTest = templateMatch(ref,can, template_size);


//Mat feat1;
//vector<KeyPoint> keypoints1;
//
//
//Ptr<ORB> detector = ORB::create();
//detector->setMaxFeatures(500);
////Ptr<DescriptorExtractor> extractor =  ORB::create();
//detector->detect(img_1,keypoints1);
//cout << keypoints1.size();
//
// Mat descriptors1;
// detector->compute( img_1, keypoints1, descriptors1 );
//
// Mat feat2;
// vector<KeyPoint> keypoints2;
//
// detector->detect(img_2,keypoints2);
//cout << keypoints2.size();
//
// Mat descriptors2;
// detector->compute( img_2, keypoints2, descriptors2);
//
//
//
// Mat img_keypoints;
// drawKeypoints(img_1,keypoints1,img_keypoints);
// //imshow("keypoints1",img_keypoints);
//
// drawKeypoints(img_2,keypoints2,img_keypoints);
// //imshow("keypoints2",img_keypoints);
//
// BFMatcher matcher;
// std::vector< DMatch > matches;
// matcher.match( descriptors1, descriptors2, matches);
//
//   double max_dist = 0; double min_dist = 100;
//
//  //-- Quick calculation of max and min distances between keypoints
//  for( int i = 0; i < descriptors1.rows; i++ )
//  { double dist = matches[i].distance;
//    if( dist < min_dist ) min_dist = dist;
//    if( dist > max_dist ) max_dist = dist;
//  }
//
//  printf("-- Max dist : %f \n", max_dist );
//  printf("-- Min dist : %f \n", min_dist );
//
//  //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist,
//  //-- or a small arbitary value ( 0.02 ) in the event that min_dist is very
//  //-- small)
//  //-- PS.- radiusMatch can also be used here.
//  std::vector< DMatch > good_matches;
//
//  for( int i = 0; i < descriptors1.rows; i++ )
//  { if( matches[i].distance <= max(2*min_dist, 0.02) )
//    { good_matches.push_back( matches[i]); }
//  }
//
//
//
// Mat img_matches;
// drawMatches(img_1,keypoints1,img_2,keypoints2,matches,img_matches);
// //imshow("Matches",img_matches);
 


 //	Mat forDisplay;

	//int horizontal = 0;
 //  int vertical = 0;
 //  GetDesktopResolution(horizontal, vertical);
 //  Size imgSize(horizontal*0.9,vertical*0.9);
 //  resize(templateMatchTest,forDisplay,imgSize);



	//namedWindow( "display window", WINDOW_AUTOSIZE ); // create a window for display.
	// imshow( "display window", forDisplay); // show our image inside it.f
		
	 waitKey(0); // Wait for a keystroke in the window
}
