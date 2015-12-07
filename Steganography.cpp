#include <opencv2/core/core.hpp>
#include "opencv2/opencv.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdint.h>
using namespace cv;
using namespace std;
char ExtractChar(Mat coverimage, int i, int j)
{
	int bit;
	int red = coverimage.at<char>(i, j);
	int green = coverimage.at<char>(i, j + 1);
	int blue = coverimage.at<char>(i, j + 2);
	j += 3;
	if (j / 3 == coverimage.cols){ j = 0; i++; }

	int red2 = coverimage.at<char>(i, j);
	int green2 = coverimage.at<char>(i, j + 1);
	int blue2 = coverimage.at<char>(i, j + 2);

	// convert the two pixels to a character

	unsigned int T = 0;
	T += (red & 1);
	T += (2 * (green & 1));
	T += (4 * (blue & 1));
	bit = blue & 2;
	if (bit > 0)
		bit = 1;
	else
		bit = 0;
	T += (8 * bit);
	T += (16 * (red2 & 1));
	T += (32 * (green2 & 1));
	T += (64 * (blue2 & 1));
	bit = blue2 & 2;
	if (bit > 0)
		bit = 1;
	else
		bit = 0;
	T += (128 * bit);
	char c = (char)T;
	return c;
}

void encode(char* file, char* image, char* outputfile){
	Mat coverimage = imread(image, CV_LOAD_IMAGE_UNCHANGED);
	//cvtColor(coverimage, coverimage, CV_RGBA2BGRA);
	if (!coverimage.data){
		cout << "Could not read one of the immages " << endl; return;
	}
	int MaxNumberOfPixels = coverimage.rows * coverimage.cols;
	int MaxCharsToEncode = (int)(0.5 * MaxNumberOfPixels);
	int NumberOfCharsToEncode;
	int FileNameSize = strlen(file) + 1;
	char * FileName = new char[FileNameSize];
	strcpy(FileName, file);
	int filesize = 0;
	char* StegoIdentifierString = "Identifier";
	FILE* fp;
	fp = fopen(file, "rb");
	if (!fp)
	{
		cout << "Error: unable to read file " << file << " for text input!" << endl; return;
	}

	NumberOfCharsToEncode = FileNameSize
		+ strlen(StegoIdentifierString)
		+ 2 // indicate length of filename
		+ 4; // indicate length of data

	unsigned char *CharsToEncode = new unsigned char[NumberOfCharsToEncode];

	if (NumberOfCharsToEncode >  MaxCharsToEncode)
	{
		cout << "Error: File is too small to even encode file information!\n"
			<< "       Terminating encoding.\n";
		return;
	}

	while (!feof(fp))
	{
		char c;
		fread(&c, 1, 1, fp);
		filesize++;
	}
	filesize--;

	MaxCharsToEncode -= NumberOfCharsToEncode;
	if (filesize > MaxCharsToEncode)
	{
		filesize = MaxCharsToEncode;
		cout << "Warning: Input file exceeds encoding capacity of the image\n"
			<< "         File will be truncated.\n";
		return;
	}
	fclose(fp);

	// create this "file header" string
	int k = 0;
	// this part gives the length of the filename
	while (k <  strlen(StegoIdentifierString))
	{
		CharsToEncode[k] = StegoIdentifierString[k]; k++;
	}
	int TempInt = FileNameSize % 256;
	CharsToEncode[k] = (unsigned char)TempInt; k++;
	TempInt = FileNameSize - TempInt;
	if (TempInt < 0){ TempInt = 0; }
	TempInt = TempInt / 256;
	CharsToEncode[k] = (unsigned char)TempInt; k++;

	// this part hides the filename
	int j = 0;
	while (j < FileNameSize)
	{
		CharsToEncode[k] = FileName[j]; k++; j++;
	}

	// this part gives the length of the hidden data
	int TempIntOriginal = filesize;
	TempInt = filesize % 256;
	CharsToEncode[k] = (unsigned char)TempInt; k++;
	TempIntOriginal -= TempInt;

	if (TempIntOriginal > 0)
	{
		TempInt = TempIntOriginal % 65536;
		CharsToEncode[k] = (unsigned char)(TempInt / 256); k++;
		TempIntOriginal -= TempInt;// *256;
	}
	else
	{
		CharsToEncode[k] = 0; k++;
	}

	if (TempIntOriginal > 0)
	{
		TempInt = TempIntOriginal % 16777216;
		CharsToEncode[k] = (unsigned char)(TempInt / 65536); k++;
		TempIntOriginal -= TempInt;// *65536;
	}
	else
	{
		CharsToEncode[k] = 0; k++;
	}

	if (TempIntOriginal > 0)
	{
		TempInt = TempIntOriginal % 4294967296;
		CharsToEncode[k] = (unsigned char)(TempInt / 1677216); k++;
		TempIntOriginal -= TempInt;// *16777216;
	}
	else
	{
		CharsToEncode[k] = 0; k++;
	}

	k = 0;
	int i = 0;
	j = 0;
	fp = fopen(file, "rb");
	while (!feof(fp) && k < NumberOfCharsToEncode)
	{
		// decompose the character 

		unsigned int T = (unsigned int)CharsToEncode[k];

		int R1 = T % 2;
		T = (T - R1) / 2;
		int G1 = T % 2;
		T = (T - G1) / 2;
		int B1 = T % 2;
		T = (T - B1) / 2;
		int A1 = T % 2;
		T = (T - A1) / 2;

		int R2 = T % 2;
		T = (T - R2) / 2;
		int G2 = T % 2;
		T = (T - G2) / 2;
		int B2 = T % 2;
		T = (T - B2) / 2;
		int A2 = T % 2;
		T = (T - A2) / 2;
		int red = coverimage.at<char>(i, j);
		int green = coverimage.at<char>(i, j+1);
		int blue = coverimage.at<char>(i, j+2);

		red = red -(red &1) + R1;
		green = green - (green & 1) + G1;
		blue = blue - (blue & 3) + B1 + (A1*2);
		coverimage.at<char>(i, j) = red;
		coverimage.at<char>(i, j + 1) = green;
		coverimage.at<char>(i, j + 2) = blue ;

		j+=3;
		if (j/3 == coverimage.cols){ j = 0; i++; }

		red = coverimage.at<char>(i, j);
		green = coverimage.at<char>(i, j + 1);
		blue = coverimage.at<char>(i, j + 2);

		red = red - (red & 1) + R2;
		green = green - (green & 1) + G2;
		blue = blue - (blue & 3) + B2 + (A2 * 2);
		coverimage.at<char>(i, j) = red;
		coverimage.at<char>(i, j + 1) = green;
		coverimage.at<char>(i, j + 2) = blue;

		j+=3; k++;
		if (j/3 == coverimage.cols){ j = 0; i++; }
	}

	// encode the actual data 
	k = 0;
	while (!feof(fp) && k < 6 * filesize)
	{
		char c;
		fread(&c, 1, 1, fp);

		// decompose the character 

		unsigned int T = (unsigned int)c;

		int R1 = T % 2;
		T = (T - R1) / 2;
		int G1 = T % 2;
		T = (T - G1) / 2;
		int B1 = T % 2;
		T = (T - B1) / 2;
		int A1 = T % 2;
		T = (T - A1) / 2;

		int R2 = T % 2;
		T = (T - R2) / 2;
		int G2 = T % 2;
		T = (T - G2) / 2;
		int B2 = T % 2;
		T = (T - B2) / 2;
		int A2 = T % 2;
		T = (T - A2) / 2;

		int red = coverimage.at<char>(i, j);
		int green = coverimage.at<char>(i, j + 1);
		int blue = coverimage.at<char>(i, j + 2);

		red = red - (red & 1) + R1;
		green = green - (green & 1) + G1;
		blue = blue - (blue & 3) + B1 + (A1 * 2);
		coverimage.at<char>(i, j) = red;
		coverimage.at<char>(i, j + 1) = green;
		coverimage.at<char>(i, j + 2) = blue;
		j += 3;
		if (j / 3 == coverimage.cols){ j = 0; i++; }

		red = coverimage.at<char>(i, j);
		green = coverimage.at<char>(i, j + 1);
		blue = coverimage.at<char>(i, j + 2);

		red = red - (red & 1) + R2;
		green = green - (green & 1) + G2;
		blue = blue - (blue & 3) + B2 + (A2 * 2);
		coverimage.at<char>(i, j) = red;
		coverimage.at<char>(i, j + 1) = green;
		coverimage.at<char>(i, j + 2) = blue;

		j += 3; k++;
		if (j / 3 == coverimage.cols){ j = 0; i++; }
	}
	fclose(fp);

	imwrite(outputfile, coverimage);

	return;
}

void decode(char* image){
	Mat coverimage = imread(image, CV_LOAD_IMAGE_UNCHANGED);
	//cvtColor(coverimage, coverimage, CV_RGBA2BGRA);
	if (!coverimage.data){
		cout << "Could not read the image " << endl; return;
	}
	char* StegoIdentifierString = "Identifier";
	char* ComparisonString = new char[strlen(StegoIdentifierString) + 1];

	int i = 0;
	int j = 0;
	int k = 0;
	while (k < strlen(StegoIdentifierString))
	{
		ComparisonString[k] = ExtractChar(coverimage, i, j);
		j += 6;
		while (j/3 >= coverimage.cols)
		{
			j =0; i++;
		}
		k++;
	}
	ComparisonString[k] = '\0';

	if (strcmp(StegoIdentifierString, ComparisonString))
	{
		cout << "Error: No (compatible) hidden data found in image!\n";
		return;
	}

	// get the next two characters to determine file size 
	unsigned char C1 = (unsigned char)ExtractChar(coverimage, i, j);
	j += 6;
	while (j / 3 >= coverimage.cols)
	{
		j = 0; i++;
	}
	unsigned char C2 = (unsigned char)ExtractChar(coverimage, i, j);
	j += 6;
	while (j / 3 >= coverimage.cols)
	{
		j = 0; i++;
	}

	int FileNameSize = C1 + 256 * C2;
	char* FileName = new char[FileNameSize];

	// read the filename
	k = 0;
	while (k < FileNameSize)
	{
		FileName[k] = ExtractChar(coverimage, i, j);
		j += 6;
		while (j / 3 >= coverimage.cols)
		{
			j = 0; i++;
		}
		k++;
	}
	FileName[k] = '\0';

	// find the actual data size

	C1 = (unsigned char)ExtractChar(coverimage, i, j);
	j += 6;
	while (j / 3 >= coverimage.cols)
	{
		j = 0; i++;
	}
	C2 = (unsigned char)ExtractChar(coverimage, i, j);
	j += 6;
	while (j / 3 >= coverimage.cols)
	{
		j = 0; i++;
	}
	unsigned char C3 = (unsigned char)ExtractChar(coverimage, i, j);
	j += 6;
	while (j / 3 >= coverimage.cols)
	{
		j = 0; i++;
	}
	unsigned char C4 = (unsigned char)ExtractChar(coverimage, i, j);
	j += 6;
	while (j / 3 >= coverimage.cols)
	{
		j = 0; i++;
	}
	int FileSize = C1 + (256 * C2) + (65536 * C3) + (16777216 * C4);
	int NumberOfCharsToEncode = FileNameSize + 2 + 4
		+ strlen(StegoIdentifierString);

	if (FileSize == 0 || FileNameSize == 0 || NumberOfCharsToEncode == 0)
	{
		cout << "No hiddent data detected. Exiting ... " << endl;
		return;
	}

	cout << "Hidden data detected! Outputting to file " << FileName << " ... " << endl;

	FILE* fp;
	fp = fopen(FileName, "wb");
	if (!fp)
	{
		cout << "Error: Unable to open file " << FileName << " for output!\n";
		return;
	}

	int MaxNumberOfPixels = coverimage.cols * coverimage.rows;

	k = 0;
	i = 0;
	j = 0;

	// set the starting pixel to skip the internal header
	j = 6 * NumberOfCharsToEncode;
	while (j / 3 >= coverimage.cols)
	{
		j = 0; i++;
	}

	while (k < 6 * FileSize)
	{
		// read the two pixels
		int bit;
		int red = coverimage.at<char>(i, j);
		int green = coverimage.at<char>(i, j + 1);
		int blue = coverimage.at<char>(i, j + 2);
		j += 3;
		k++;
		if (j / 3 == coverimage.cols){ j = 0; i++; }

		int red2 = coverimage.at<char>(i, j);
		int green2 = coverimage.at<char>(i, j + 1);
		int blue2 = coverimage.at<char>(i, j + 2);
		j += 3;
		k++;
		if (j / 3 == coverimage.cols){ j = 0; i++; }

		// convert the two pixels to a character

		unsigned int T = 0;
		T += (red & 1);
		T += (2 * (green & 1));
		T += (4 * (blue & 1));
		bit = blue & 2;
		if (bit > 0)
			bit = 1;
		else
			bit = 0;
		T += (8 * bit);
		T += (16 * (red2 & 1));
		T += (32 * (green2 & 1));
		T += (64 * (blue2 & 1));
		bit = blue2 & 2;
		if (bit > 0)
			bit = 1;
		else
			bit = 0;
		T += (128 * bit);
		char c = (char)T;

		fwrite(&c, 1, 1, fp);
	}
	fclose(fp);

	return;
}


int main(int argc, char* argv[]){

	if (*argv[1]=='e'&&argc==5)
		encode(argv[2], argv[3], argv[4]);

	if (*argv[1] == 'd'&&argc == 3)
		decode(argv[2]);

	//waitKey(0);

	return 0;
}