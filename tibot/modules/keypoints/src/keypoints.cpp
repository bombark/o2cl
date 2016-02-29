#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <tiobj.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/nonfree/nonfree.hpp>

using namespace std;
using namespace cv;


void writeKeypoints(FILE* fd, vector<KeyPoint>& keypoints){
	ftruncate(fileno(fd), 0);
	fseek(fd,SEEK_SET,0);
	for (int i=0; i<keypoints.size(); i++){
		KeyPoint& kp = keypoints[i];
		fprintf(fd,"{x=%d;y=%d}\n",(int)kp.pt.x,(int)kp.pt.y);
	}
}

void writeDescriptors(FILE* fd, Mat& descriptors){
	ftruncate(fileno(fd), 0);
	fseek(fd,SEEK_SET,0);
	fwrite(descriptors.data, 4, descriptors.cols*descriptors.rows, fd);
}




int main(){
	char buffer[128];
	TiObj params( getenv("params") );
	
	//initModules_nonfree();

	Mat img;
	TiObj info;
	info.loadFile("image/info");
	
	int cols = info.atInt("cols");
	int rows = info.atInt("rows");
	
	img.create( rows, cols, CV_8UC3 );
	
	FILE* fd = fopen("image/image", "r");
	if ( !fd ){
		cerr << "File not found\n";
		return 1;
	}

	FILE* fd_keypoints   = fopen("keypoints","w");
	FILE* fd_descriptors = fopen("descriptors","w");


	Mat gray, descriptors, output;
	vector<KeyPoint> keypoints;

	string detector_method  = params.atStr("detector","SURF");
	Ptr<FeatureDetector> detector = FeatureDetector::create( detector_method );
	if ( detector == NULL ){
		cerr << Join("Detector %s is not valid\n").at(detector_method).ok;
		exit(1);
	}
	
	string extractor_method = params.atStr("extractor","SURF");
	Ptr<DescriptorExtractor> extractor =  DescriptorExtractor::create( extractor_method );
	if ( extractor == NULL ){
		cerr << Join("Extractor %s is not valid\n").at(extractor_method).ok;
		exit(1);
	}
	
	while( scanf("%s",buffer) ){
		// Read the input
		fseek( fd, SEEK_SET, 0);
		fread( img.data, cols*rows, 3, fd);
		cvtColor( img, gray, CV_BGR2GRAY );

		// Process
		detector->detect(gray, keypoints);
		extractor->compute( gray, keypoints, descriptors );
		
		// Write result
		writeKeypoints(fd_keypoints, keypoints);
		writeDescriptors(fd_descriptors, descriptors);
		
		// Show
		drawKeypoints (img, keypoints, output);
		imshow("aaa", output);
		waitKey(20);
		
		// End
		printf("#end\n");
		fflush(stdout);
	}
	
	fclose(fd);
	return 0;
}
