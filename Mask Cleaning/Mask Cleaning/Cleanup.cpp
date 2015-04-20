#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <stdio.h>
#include <stdlib.h>
#include "wtypes.h"
#include <iostream>

#include <fstream>
#include <sstream>

//#include "stdafx.h"
#include <string>

#include <msclr\marshal_cppstd.h>

using namespace cv;
using namespace std;
using namespace System;
using namespace System::IO;
 
// Get the horizontal and vertical screen sizes in pixel
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
 

Mat removeSmallBlobs(Mat m){
	Mat labels;
	Mat stats;
	Mat centroids;
	int n = connectedComponentsWithStats(m,labels,stats,centroids,4);
	int biggest = 1;
	for(int i = 1; i<n;i++){
		if(stats.at<int>(i,CC_STAT_AREA)>stats.at<int>(biggest,CC_STAT_AREA)){
			biggest = i;
		}
	}

	cout<<stats.at<int>(biggest,CC_STAT_AREA);
	Mat cleanMask;
	compare(labels,biggest,cleanMask,CMP_EQ);

	dilate(cleanMask, cleanMask, Mat(), Point(-1,-1), 2);
	
	Mat invMask = 255 - cleanMask;

	m.setTo(0,invMask);
	m = m-100;
	Mat goodMask = Mat::zeros(m.size(),CV_8U);
	goodMask.setTo(255,m);
	return goodMask;
}

Mat basicCrop(Mat m,Mat img, Mat* croppedImg, int extraRows){
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
	
	Rect myROI(lx,ty,rx-lx,by-ty+extraRows);
	Mat cropped = m(myROI);

	*croppedImg = img(myROI);
	//namedWindow( "w1", WINDOW_AUTOSIZE ); // create a window for display.
	//imshow( "w1", *croppedImg); // show our image inside it.f
	//waitKey(0); // Wait for a keystroke in the window
	cout<<"basic cropped"<< '\n';
	return cropped;
}


//img is the color image that the mask applies to
// it will be cropped and masked
Mat dropCrop(Mat m, Mat img, Mat* imgCrop){
	Mat croppedImg;
	Mat* croppedImgPtr = &croppedImg;
	Mat cropped = basicCrop(m,img, croppedImgPtr,30);
	Mat cT;
	transpose(cropped,cT);

	int nRows = cT.rows;
	int nCols = cT.cols;
	int j = 0;

	uchar* p;
	int found = 0;
	int yPoint = 0;
	for(int i = (int)(0.2*nRows) ; i<(int)(0.8*nRows);i++){
	p = cT.ptr<uchar>(i);
	found = 0;
	j = 0;
		while(!found && j<nCols){
			if(p[j] > 0){
				if(j>yPoint){
					yPoint = j;
				}
				found  = 1;
			}
			j++;
		}
	}
	cout<<"drop crop counted"<< '\n';
	Rect myROI2(0, 0, cropped.cols, yPoint + 10);
	Mat dropCropped = cropped(myROI2);
	cout<<"mask cropped \n";

	Mat dropCroppedImg = croppedImg(myROI2);
	
	cout<<"drop cropped\n";

	Mat reCroppedImg;
	Mat* reCroppedImgPtr = &reCroppedImg;
	Mat reCropped = basicCrop(dropCropped,dropCroppedImg,reCroppedImgPtr,0);

	//namedWindow( "w2", WINDOW_AUTOSIZE ); // create a window for display.
	//imshow( "w2", reCropped); // show our image inside it.f
	//waitKey(0); // Wait for a keystroke in the window
	reCroppedImg = reCroppedImg+Scalar(1,1,1);
	reCroppedImg.setTo((0,0,0),255-reCropped);
	*imgCrop = reCroppedImg.clone();

	return reCropped;
}

Mat dropTop(Mat m){
	Mat mT;
	transpose(m,mT);
	Mat top(1,m.cols,CV_32F);

	int nRows = mT.rows;
	int nCols = mT.cols;
	int j = 0;
	uchar* p;
	int found = 0;

	float* x;
	x = top.ptr<float>(0);

	for(int i = 0 ; i<nRows;i++){
	p = mT.ptr<uchar>(i);
	found = 0;
	j = 0;
	x[i] = 0;
		while(!found && j<nCols){
			if(p[j] > 0){
				x[i] = j;
				found  = 1;
			}
			j++;
		}
	}
	//namedWindow( "w1", WINDOW_AUTOSIZE ); // create a window for display.
 //   imshow( "w1", mT); // show our image inside it.f
	//save
	FileStorage file("../images/w4_1t.yml",FileStorage::WRITE);
	file<<"Top"<<top;
	return top;
}


void main(void){
	System::String^ srcDir = "..//images//eval//";
	System::String^ resultDir = "..//images//eval_clean//";
	cv::String resultDirStd = "..//images//eval_clean//";
	//String preName ="w";
	//String postName1 = "best_GCmask.jpg";
	//String postName2 = "best_scaled.jpg"
	DirectoryInfo^ di = gcnew DirectoryInfo( resultDir );

   // Create the directory only if it does not already exist. 
   if ( di->Exists == false )
      di->Create();
	
	array<System::String^>^ pics =  System::IO::Directory::GetFiles(srcDir);

	
	Mat forDisplay;

	int horizontal = 0;
	int vertical = 0;
	 GetDesktopResolution(horizontal, vertical);

	cout << horizontal << '\n' << vertical << '\n';
	msclr::interop::marshal_context context;
	std::string stdStr; 

	for(int i = 0; i<(pics->Length)/2; i++){
		Mat mask;
//		String readStr1 = srcDir+preName+i+postName1;
		stdStr = context.marshal_as<std::string>(pics[i*2]);
		mask = imread(stdStr, IMREAD_GRAYSCALE); // read the mask

		Mat image;
		stdStr = context.marshal_as<std::string>(pics[i*2+1]);
		image = imread(stdStr,IMREAD_COLOR);


		Mat deBlobbed = removeSmallBlobs(mask);
	
		Mat imgCropped;
		Mat* imgCroppedPtr = &imgCropped;
		Mat cropped = dropCrop(deBlobbed, image,imgCroppedPtr);
		//Mat top = dropTop(cropped);
		//FileStorage file("../images/cropped.yml",FileStorage::WRITE);
	//	file<<"cropped"<<cropped;

		//imwrite("../images/w3_2cm.jpg",cropped);
		DirectoryInfo^ dis = di->CreateSubdirectory( "w" +  System::Convert::ToString(i) );
		//CreateSubdirectory(resultDir + "w" +  System::Convert::ToString(i));
		imwrite(resultDirStd+"w"+std::to_string((long long)i)+"/eval_cleancut.png",imgCropped);
		//waitKey(0); // Wait for a keystroke in the window
	}
}