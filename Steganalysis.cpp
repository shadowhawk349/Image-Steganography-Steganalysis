#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/opencv.hpp"
#include <stdlib.h>
#include <iostream>
#include <stdint.h>
#include <time.h>
using namespace cv;

int main(int argc, char* argv[]){
	Mat hidden = imread(argv[1]);

	int w1, h1;
	w1 = hidden.cols;
	h1 = hidden.rows;

	//******LSB ENhancment********************
	for (int i = 0; i < h1; i++)
		for (int j = 0; j < w1*3 ; j++){
			int lsb = hidden.at<char>(i, j);
			lsb = lsb & 1;
			if (lsb)
				hidden.at<char>(i, j) = 255;
			else
				hidden.at<char>(i, j) = 0;
		}

	imwrite(argv[2], hidden);

	return 0;
}