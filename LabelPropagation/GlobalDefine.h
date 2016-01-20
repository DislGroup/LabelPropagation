//disl 2015.11.23
//For Label Propagation in Video Sequences

#ifndef GLOBALDEFINE_H_
#define GLOBALDEFINE_H_

#include <iostream>
#include <string>
#include <fstream>
using namespace std;

#include <mat.h>
#include <time.h>
#include <pthread.h>

#include <opencv2/opencv.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
using namespace cv;

//kitchen 13 conference 6 bookstore 3
extern const int labelNum ;
extern const int groundLabel ;

//global Function
void fillBlank(unsigned char *labelMap,unsigned char* yImg,int H,int W);
int upGraphCuts(unsigned char* yImg,unsigned char * labelMap,const int H,const int W,const int totalLabel,const int usedLabel);
void gaussianFilter(unsigned char* yImg,int H,int W);
void gaussianFilterSSE(unsigned char* yImg,int H,int W);
int upOpticalFlow(Mat &iImg,Mat &pImg,unsigned char *iLabelMap,unsigned char *pLabelMap);

//P-thread Running function 
void* Function_t(void* Param);

// Parameter For Frame
class framePram
{
public:
	unsigned char *yImg;
	unsigned char *labelMap;
	Mat cvGray;
	framePram(){yImg=NULL;labelMap=NULL;}
	~framePram(){delete []yImg;}
	void initData(int H,int W);
};

// Parameter For Pthread
struct threadPram
{
	framePram *frameI;
	framePram *frameP;
	int H;
	int W;
	int index;
};

#endif