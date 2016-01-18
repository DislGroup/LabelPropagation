// Edit by Disl 2015.12
#include <opencv2/opencv.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
using namespace cv;

const double pyrScale = 0.5;
const int levels = 3;
const int winsize = 15;
const int iterations =3;
const int polyN = 5;
const double polySigma =1.2;
const int flags = 0;

int upOpticalFlow(Mat &iImg,Mat &pImg,unsigned char *iLabelMap,unsigned char *pLabelMap)
{
	Mat flow;
	if(iImg.data && pImg.data)
	{
		calcOpticalFlowFarneback(iImg,pImg,flow,pyrScale,levels,winsize,iterations,polyN,polySigma,flags);
	}
	else
		return 1;
	int H=iImg.rows;
	int W=iImg.cols;
	if(flow.data && flow.rows==H && flow.cols==W)
	{
		Vec2f flow_at_point;
		int fx,fy;
		for(int i=0;i<H;i++)
			for(int j=0;j<W;j++)
			{
				flow_at_point = flow.at<Vec2f>(i,j);
				fx = (int)flow_at_point[0];
				fy = (int)flow_at_point[1];
				if( i+fy>=0 && i+fy<H && j+fx>=0 && j+fx<W)
					pLabelMap[(i+fy)*W+j+fx] = iLabelMap[i*W+j];
			}
		return 0;
	}
	else
		return 1;
}