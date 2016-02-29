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



void SigleFromCamera(Mat& output){
	
}







int main(){
	TiObj params( getenv("params") );

	Mat img;

	// If the device is fake, then create a example of the data
	if ( params.has("fake") ){
		img.create(480,640,CV_8UC3);
		img.setTo(0);
		saveInfo(img);
		imwrite("image.jpg",img);

	// Read the Cam 
	} else {
		int id = params.atInt("id",0);
		G_capture.open(id);
		signal( SIGINT, sig_hnd );

		G_capture >> img;
		saveInfo(img);

		FILE* fd = fopen("image","w");

		while(1){
			G_capture >> img;
			//imshow("asa",img);
			imwrite("image.jpg",img);
			
			fseek(fd, SEEK_SET, 0);
			fwrite(img.data, img.cols*img.rows*3, 1, fd);
			
			//waitKey(100);
			cout << "#end\n";
			fflush(stdout);
			usleep(100000);
		}
	}


	return 0;
}
