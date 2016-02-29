/*
 * Stereo Camera Capture
 * Copyright (C) 2015 Victor H. Schulz
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/*=====================================- HEADER -======================================*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <tiobj.hpp>
#include <unistd.h>
#include "focus.h"


#include <sys/stat.h>

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace cv;
using namespace std;

/*-------------------------------------------------------------------------------------*/


/*=====================================- UTILS -=======================================*/



/*-------------------------------------------------------------------------------------*/


/*======================================- MAIN -=======================================*/

int main(int argc, char* argv[])
{
	chdir( getenv("nom") );
	TiObj params( getenv("params") );

	TiObj desc;
	desc.loadFile(".sysobj.ti");
	cout << desc;


	// load Configuration
	int cam_width  = desc.atInt( "width", 320);
	int cam_height = desc.atInt("height", 240);
	uint numBoards = params.atInt("_Akk", 5);           // Number of photos to calibrate from

	if ( ! desc.at("board").isObj() ){
		cerr << "Attribute board must be a object\n";
		cerr << "Example: board = {cols=9;rows=9;size=10}\n";
		return 1;
	}
	uint board_w   = desc.atObj("board").atInt("cols"); // Number of horizontal corners on chess board
	uint board_h   = desc.atObj("board").atInt("rows"); // Number of vertical corners on chess board



	// Config the camera 1
	int cam1_id = desc.atInt("left",0);
	Camera cam1( Join("video%s").at(cam1_id).ok.c_str() ); // Create the objects for focus control
	VideoCapture cap1( cam1_id );                          // Create objects for controling the video streams
	cap1.set(CV_CAP_PROP_FRAME_WIDTH , cam_width);         // Set the resolution on both cameras
	cap1.set(CV_CAP_PROP_FRAME_HEIGHT, cam_height);
	cam1.setFocus(0);
	cam1.setExposureTime(250);

	// Config the camera 2
	int cam2_id = desc.atInt("right",1);
	Camera cam2( Join("video%s").at(cam2_id).ok.c_str() );
	VideoCapture cap2( cam2_id );
	cap2.set(CV_CAP_PROP_FRAME_WIDTH , cam_width);
	cap2.set(CV_CAP_PROP_FRAME_HEIGHT, cam_height);
	cam2.setFocus(0);                         // Set the focus on both cameras to a fixed value
	cam2.setExposureTime(250);

	//cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << endl;
	//cout << "Press 'space' to capture chessboard photos," << endl << "or 't' to capture the reference photos" << endl;




	// Init the variables
	Size board_sz(board_w, board_h);
	vector<Point2f> corners1, corners2;
	Mat img1, img2, gray1, gray2;
	mkdir("capture",0755);
	uint success = 0;                         // Counter for successfully saved images
	char k = 0;                               // Keyboard character
	bool found1 = false, found2 = false;      // Corners found on image 1, 2
	ofstream imglist;                         // File to store a list of captured images
	imglist.open("list.ti",ios::trunc);

	// Init the Mat to show the images
	Mat view;
	cap1 >> img1;
	view.create( img1.rows, img1.cols*2, CV_8UC3 );
	
	Mat show1 = view( Rect(        0,0,img1.cols,img1.rows) );
	Mat show2 = view( Rect(img1.cols,0,img1.cols,img1.rows) );
	
	while (success < numBoards)
	{
		cap1 >> img1;
		cap2 >> img2;
		img1.copyTo(show1);
		img2.copyTo(show2);
		cvtColor(img1, gray1, CV_BGR2GRAY);   // Convert to gray scale, needed for findChessboardCorners()
		cvtColor(img2, gray2, CV_BGR2GRAY);

		found1 = findChessboardCorners(img1, board_sz, corners1, CV_CALIB_CB_ADAPTIVE_THRESH + CV_CALIB_CB_NORMALIZE_IMAGE);
		found2 = findChessboardCorners(img2, board_sz, corners2, CV_CALIB_CB_ADAPTIVE_THRESH + CV_CALIB_CB_NORMALIZE_IMAGE);

		if (found1)
		{
			cornerSubPix(gray1, corners1, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
			drawChessboardCorners( show1 , board_sz, corners1, found1);
		}

		if (found2)
		{
			cornerSubPix(gray2, corners2, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
			drawChessboardCorners( show2, board_sz, corners2, found2);
		}




		imshow("Image", view);
		k = waitKey(10);

		/*if(k=='t'){
			cout << "Reference image pair saved." << endl;

			// Save images:
			imwrite("capture/image1_t.jpg", img1);
			imwrite("capture/image2_t.jpg", img2);
		}*/

		//if (k==' ' && found1 !=0 && found2 != 0)
		if ( found1 && found2 )
		{
			cout << "Image pair " << success << " saved." << endl;

			// Save images:
			char filename1[512];
			sprintf(filename1,"capture/image-%05d-1.jpg",success);
			
			char filename2[512];
			sprintf(filename2,"capture/image-%05d-2.jpg",success);
			
			imwrite(filename1, img1);
			imwrite(filename2, img2);
			// Record images on text list:
			imglist << Join("{left='%s';right='%s'}\n").at(filename1).at(filename2).ok;

			success++;
			sleep(1);
		}
	}

	imglist.close(); // Close file with image list

	destroyAllWindows();

	cap1.release();
	cap2.release();

	return(0);
}

/*-------------------------------------------------------------------------------------*/
