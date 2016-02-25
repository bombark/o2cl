#include <stdio.h>
#include <iostream>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <tiobj.hpp>
#include <tisys.hpp>
#include <signal.h> 



using namespace std;
using namespace cv;


VideoCapture G_capture;

void sig_hnd(int sig){
	G_capture.release();
	exit(0);
}


void saveInfo(Mat& img){
	TiObj data;
	data.set("cols",img.cols);
	data.set("rows",img.rows);
	data.saveFile("info");
}


int main(){
	TiObj params;
	char* _params = getenv("params");
	if ( _params )
		params.loadText(_params);

	Mat img;

	// If the device is fake, then create a example of the data
	if ( params.has("fake") ){
		img.create(480,640,CV_8UC3);
		img.setTo(0);
		saveInfo(img);
		imwrite("image.jpg",img);

	// Read the Cam 
	} else {
		int id = atoi( params.atStr("id","0").c_str() );
		G_capture.open(id);
		signal( SIGINT, sig_hnd );

		G_capture >> img;
		saveInfo(img);


		while(1){
			G_capture >> img;
			//imshow("asa",img);
			imwrite("image.jpg",img);
			waitKey(100);
			cout << "#end\n";
			fflush(stdout);
		}
	}


	return 0;
}
