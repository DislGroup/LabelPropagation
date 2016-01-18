// Edit by Disl 2015.12
#include <math.h>

void fillBlank(unsigned char *labelMap,unsigned char* yImg,int H,int W)
{
	unsigned char *tempMap=new unsigned char[H*W];
	for(int i=0;i<H;i++)
		for(int j=0;j<W;j++)
			tempMap[i*W+j] = labelMap[i*W+j];

	//mid point
	int interH = H>>1;
	int interW = W>>1;

	//part zone
	//top_left top_right down_left down_right
	int endY[4]={0,0,H-1,H-1};
	int endX[4]={0,W-1,0,W-1};
	int incY[4]={-1,-1,1,1};
	int incX[4]={-1,1,-1,1};

	//neighbor zone
	int indexY[8]={-1,-1,-1,0,0,1,1,1};
	int indexX[8]={-1,0,1,-1,1,-1,0,1};

	//temp Variable
	unsigned char yTable[8];
	int yCount[8];
	double yError[8];

	for(int order=0;order<4;order++)
	{
		//four part
		for(int i=interH;i!=endY[order];i=i+incY[order])
			for(int j=interW;j!=endX[order];j=j+incX[order])
			{
				if(tempMap[i*W+j]==0)
				{
					for(int k=0;k<8;k++)
					{
						yTable[k]=0;
						yCount[k]=0;
						yError[k]=1;
					}
					int pos=0; //index For lookup yTable
					//neighbor 8
					for(int k=0;k<8;k++)
					{
						int m,n,p;
						m=indexY[k]+i;
						n=indexX[k]+j;
						for(p=0;p<pos;p++)
						{
							if(yTable[p]==tempMap[m*W+n])
							{
								yCount[p]++;
								double dis=sqrt((double)((yImg[m*W+n]-yImg[i*W+j])/256)*(double)((yImg[m*W+n]-yImg[i*W+j])/256));
								if(dis<yError[p])
									yError[p]=dis;
								break;
							}
						}
						if(p==pos)
						{
							yTable[pos]=tempMap[m*W+n];
							yCount[pos]=1;
							yError[pos]=sqrt((double)((yImg[m*W+n]-yImg[i*W+j])/256)*(double)((yImg[m*W+n]-yImg[i*W+j])/256));
							pos++;
						}
					}
					double tempMax=-1;
					for(int p=0;p<8;p++)
						if(tempMax < (yCount[p]/8+(1-yError[p]))) //metric
						{
							tempMax = yCount[p]/8+(1-yError[p]);
							pos=p;
						}
					labelMap[i*W+j]=yTable[pos];
				}
			}
	}
}