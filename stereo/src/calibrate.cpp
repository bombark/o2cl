/*
 * Stereo Camera Calibration
 * Copyright (C) 2015
 *      Victor H. Schulz
 *      Felipe Gustavo Bombardelli <fgbombardelli@gmail.com>
 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */


/*=====================================- HEADER -======================================*/

#include <iostream>
#include <fstream>
#include <tiobj.hpp>
#include <unistd.h>

#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;


/*-------------------------------------------------------------------------------------*/



/*======================================- MAIN -=======================================*/


int main(int argc, char* argv[])
{
	chdir( getenv("nom") );
	TiObj params( getenv("params") );
	double squareSize = params.atDbl("_Akk",10.0);

	TiObj desc;
	desc.loadFile(".sysobj.ti");
	if ( ! desc.at("board").isObj() ){
		cerr << "Attribute board must be a object\n";
		cerr << "Example: board = {cols=9;rows=9;size=10}\n";
		return 1;
	}
	uint board_w   = desc.atObj("board").atInt("cols"); // Number of horizontal corners on chess board
	uint board_h   = desc.atObj("board").atInt("rows"); // Number of vertical corners on chess board



	uint i, j;
	uint nframes = 0;                      // Number of good chessboards found

	Size board_sz(board_w, board_h);
	vector<vector<Point3f> > object_points;              // Real location of the corners in 3D
	vector<vector<Point2f> > imagePoints1, imagePoints2; // Detected corners in the image
	vector<Point2f> corners1, corners2;
	vector<String> filenames;                            // Vector with used filenames

	// Assign constant positions to all the corners, starting at 0,0
	// on multiples of squareSize
	vector<Point3f> obj;
	for(i=0;i<board_h;i++){
		for(j=0;j<board_w;j++){
			obj.push_back(Point3f(i*squareSize,j*squareSize,0));
		}
	}

	cout << "Reading list.ti" << endl;
	TiObj list;
	list.loadFile("list.ti");
	string line;
	Mat img1, img2;
	Mat gray1, gray2;
	bool found1, found2;

	for (int i=0; i<list.size(); i++){
		TiObj& node = list.box[i];
		string url_left  = node.atStr("left");
		string url_right = node.atStr("right");
		img1 = imread(url_left);
		img2 = imread(url_right);
		cvtColor(img1, gray1, CV_BGR2GRAY);   // Convert to gray scale, needed for findChessboardCorners()
		cvtColor(img2, gray2, CV_BGR2GRAY);
		found1 = findChessboardCorners(img1, board_sz, corners1, CV_CALIB_CB_ADAPTIVE_THRESH + CV_CALIB_CB_NORMALIZE_IMAGE);
		found2 = findChessboardCorners(img2, board_sz, corners2, CV_CALIB_CB_ADAPTIVE_THRESH + CV_CALIB_CB_NORMALIZE_IMAGE);
		if( found1 && found2 ){
			cornerSubPix(gray1, corners1, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
			drawChessboardCorners(img1, board_sz, corners1, found1);
			cornerSubPix(gray2, corners2, Size(11, 11), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 30, 0.1));
			drawChessboardCorners(img2, board_sz, corners2, found2);

			imagePoints1.push_back(corners1);
			imagePoints2.push_back(corners2);
			object_points.push_back(obj);
			cout << "Corners stored!" << endl;
			nframes++;
		} else{
			cout << "Error finding corners! Continuing..." << endl;
		}
		imshow("Left",img1);
		imshow("Right",img2);
		waitKey(0);
	}
	destroyAllWindows();


	cout << "Starting Calibration" << endl;
	Mat CM1 = Mat(3, 3, CV_64FC1);
	Mat CM2 = Mat(3, 3, CV_64FC1);
	setIdentity(CM1);
	setIdentity(CM2);
	Mat D1, D2, R, T, E, F;
	stereoCalibrate(
		object_points, imagePoints1, imagePoints2,
		CM1, D1, CM2, D2, img1.size(), R, T, E, F,
		cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 100, 1e-5),
		CV_CALIB_SAME_FOCAL_LENGTH | CV_CALIB_ZERO_TANGENT_DIST | CV_CALIB_FIX_ASPECT_RATIO
	);
	FileStorage fs1("calibration.yml", FileStorage::WRITE);
	fs1 << "CM1" << CM1;
	fs1 << "CM2" << CM2;
	fs1 << "D1" << D1;
	fs1 << "D2" << D2;
	fs1 << "R" << R;
	fs1 << "T" << T;
	fs1 << "E" << E;
	fs1 << "F" << F;
	fs1.release();
	cout << "Done Calibration" << endl;


	cout << "Starting Rectification" << endl;
	Mat R1, R2, P1, P2, Q;
	stereoRectify(CM1, D1, CM2, D2, img1.size(), R, T, R1, R2, P1, P2, Q);
	cout << "Done Rectification" << endl;


	cout << "Applying Undistort" << endl;
	Mat map1x, map1y, map2x, map2y;
	initUndistortRectifyMap(CM1, D1, R1, P1, img1.size(), CV_32FC1, map1x, map1y);
	initUndistortRectifyMap(CM2, D2, R2, P2, img1.size(), CV_32FC1, map2x, map2y);
	cout << "Undistort complete" << endl;


	/*StereoBM BMState(StereoBM::BASIC_PRESET);
	BMState.state.obj->preFilterSize=41;
	BMState.state.obj->preFilterCap=31;
	BMState.state.obj->SADWindowSize=41;
	BMState.state.obj->minDisparity=-64;
	BMState.state.obj->numberOfDisparities=128;
	BMState.state.obj->textureThreshold=10;
	BMState.state.obj->uniquenessRatio=15;

	Mat disp = Mat(img1.size(),CV_8UC1);  // Disparity and normalized disparity
	Mat vdisp = Mat(img1.size(),CV_8UC1);

	Mat img1r, img2r; // Remapped images
	Size imSize = img1.size();
	Mat pair(imSize.height,imSize.width*2,CV_8UC3);
	Mat left(pair, Rect(0, 0, imSize.width, imSize.height));
	Mat right(pair, Rect(imSize.width, 0, imSize.width, imSize.height));
	for(i=0;i<nframes;i++){
		img1 = imread(filenames.at(2*i));
		img2 = imread(filenames.at(2*i+1));
		remap(img1, img1r, map1x, map1y, INTER_LINEAR, BORDER_CONSTANT, Scalar());
		remap(img2, img2r, map2x, map2y, INTER_LINEAR, BORDER_CONSTANT, Scalar());
		cvtColor(img1r, gray1, CV_BGR2GRAY);
		cvtColor(img2r, gray2, CV_BGR2GRAY);
		BMState(gray2,gray1,disp);
		normalize(disp, vdisp, 0, 255, NORM_MINMAX, CV_8UC1);
		imshow("Disparity",vdisp);
		img1r.copyTo(left);
		img2r.copyTo(right);
		for(j=0; j<(uint)imSize.height; j+=16)
		    cv::line(pair, Point(0,j), Point(imSize.width*2,j), Scalar(0,255,0), 1, 8);
		imshow("Pair",pair);
		waitKey(0);
	}
	imglist.close();*/



	return 0;
}

/*-------------------------------------------------------------------------------------*/

