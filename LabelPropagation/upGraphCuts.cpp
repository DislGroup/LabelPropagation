// Edit by Disl 2015.12
#include "GCoptimization.h"
#include <math.h>

//SSE4
#include <emmintrin.h>
#include <tmmintrin.h> // SSSE3
#if HAVE_SSE4_1
#include <smmintrin.h>
#endif

const double beta_at = 1;
const double gamm_at = 0.85;
const long double threshold = 1e-8;
const int templates[9] = {1,2,1,2,4,2,1,2,1};

inline int abs_dsl(int x)
{
	int y = x>>31;
	return (x+y)^y;
}

void gaussianFilter(unsigned char* yImg,int H,int W)
{
	unsigned char* temp = new unsigned char[H*W];
	for(int i=0;i<H;i++)
		for(int j=0;j<W;j++)
			temp[i*W+j]=yImg[i*W+j];
	int sum,index;
	for(int i=1;i<H-1;i++)
		for(int j=1;j<W-1;j++)
		{
			sum=0;
			index=0;
			for(int m=i-1;m<i+2;m++)
				for(int n=j-1;n<j+2;n++)
					sum += temp[m*W+n] * templates[index++];
			sum/=16;
			if(sum>255)
				sum=255;
			yImg[i*W+j]=sum;
		}
}

void gaussianFilterSSE(unsigned char* yImg,int H,int W)
{
	int newH = H+2;
	int pendW = (W+0x7)&0xfffffff8;
	int newW = pendW+2;
	unsigned char* temp = new unsigned char[newH*newW];
	for(int i=0;i<H;i++)
		for(int j=0;j<W;j++)
			temp[(i+1)*newW+(j+1)]=yImg[i*W+j];
	//pending
	for(int i=0;i<W;i++)
	{	
		temp[i+1]=yImg[i];
		temp[(H+1)*newW+i+1]=yImg[(H-1)*W+i];
	}
	for(int i=0;i<H;i++)
	{
		temp[(i+1)*newW]=yImg[i*W];
		temp[(i+1)*newW+W+1]=yImg[i*W+W-1];
	}
	temp[0]=temp[1];
	temp[W+1]=temp[W];
	temp[(H+1)*newW]=temp[H*newW];
	temp[(H+1)*newW+W+1]=temp[(H+1)*newW+W];
	for(int j=W+2;j<newW;j++)
		for(int i=0;i<newH;i++)
			temp[i*newW+j]=0;

	unsigned short result[8]={0};
	__m128i r_base;
	__m128i r_TL,r_T,r_TR,r_L,r_R,r_DL,r_D,r_DR;

	for(int i=0;i<H;i++)
		for(int j=0;j<pendW;j+=8)
		{
			//load data
			r_base=_mm_set_epi16(temp[(i+1)*newW+j+8],temp[(i+1)*newW+j+7],temp[(i+1)*newW+j+6],temp[(i+1)*newW+j+5],temp[(i+1)*newW+j+4],temp[(i+1)*newW+j+3],temp[(i+1)*newW+j+2],temp[(i+1)*newW+j+1]);
			r_TL=_mm_set_epi16(temp[i*newW+j+7],temp[i*newW+j+6],temp[i*newW+j+5],temp[i*newW+j+4],temp[i*newW+j+3],temp[i*newW+j+2],temp[i*newW+j+1],temp[i*newW+j]);
			r_T=_mm_set_epi16(temp[i*newW+j+8],temp[i*newW+j+7],temp[i*newW+j+6],temp[i*newW+j+5],temp[i*newW+j+4],temp[i*newW+j+3],temp[i*newW+j+2],temp[i*newW+j+1]);
			r_TR=_mm_set_epi16(temp[i*newW+j+9],temp[i*newW+j+8],temp[i*newW+j+7],temp[i*newW+j+6],temp[i*newW+j+5],temp[i*newW+j+4],temp[i*newW+j+3],temp[i*newW+j+2]);
			r_L=_mm_set_epi16(temp[(i+1)*newW+j+7],temp[(i+1)*newW+j+6],temp[(i+1)*newW+j+5],temp[(i+1)*newW+j+4],temp[(i+1)*newW+j+3],temp[(i+1)*newW+j+2],temp[(i+1)*newW+j+1],temp[(i+1)*newW+j]);
			r_R=_mm_set_epi16(temp[(i+1)*newW+j+9],temp[(i+1)*newW+j+8],temp[(i+1)*newW+j+7],temp[(i+1)*newW+j+6],temp[(i+1)*newW+j+5],temp[(i+1)*newW+j+4],temp[(i+1)*newW+j+3],temp[(i+1)*newW+j+2]);
			r_DL=_mm_set_epi16(temp[(i+2)*newW+j+7],temp[(i+2)*newW+j+6],temp[(i+2)*newW+j+5],temp[(i+2)*newW+j+4],temp[(i+2)*newW+j+3],temp[(i+2)*newW+j+2],temp[(i+2)*newW+j+1],temp[(i+2)*newW+j]);
			r_D=_mm_set_epi16(temp[(i+2)*newW+j+8],temp[(i+2)*newW+j+7],temp[(i+2)*newW+j+6],temp[(i+2)*newW+j+5],temp[(i+2)*newW+j+4],temp[(i+2)*newW+j+3],temp[(i+2)*newW+j+2],temp[(i+2)*newW+j+1]);
			r_DR=_mm_set_epi16(temp[(i+2)*newW+j+9],temp[(i+2)*newW+j+8],temp[(i+2)*newW+j+7],temp[(i+2)*newW+j+6],temp[(i+2)*newW+j+5],temp[(i+2)*newW+j+4],temp[(i+2)*newW+j+3],temp[(i+2)*newW+j+2]);

			//Calculate
			r_base = _mm_slli_epi16(r_base,2);
			r_T = _mm_slli_epi16(r_T,1);
			r_L = _mm_slli_epi16(r_L,1);
			r_R = _mm_slli_epi16(r_R,1);
			r_D = _mm_slli_epi16(r_D,1);

			r_base = _mm_add_epi16(r_base,r_TL);
			r_base = _mm_add_epi16(r_base,r_T);
			r_base = _mm_add_epi16(r_base,r_TR);
			r_base = _mm_add_epi16(r_base,r_L);
			r_base = _mm_add_epi16(r_base,r_R);
			r_base = _mm_add_epi16(r_base,r_DL);
			r_base = _mm_add_epi16(r_base,r_D);
			r_base = _mm_add_epi16(r_base,r_DR);

			r_base = _mm_srli_epi16(r_base,4);

			_mm_storeu_si128((__m128i *)result,r_base);

			for(int k=0;k<8 && j+k<W;k++)
				yImg[i*W+j+k] = result[k]>255 ? 255 : result[k];
		}
		delete []temp;
}

int upGraphCuts(unsigned char* yImg,unsigned char * labelMap,const int H,const int W,const int totalLabel,const int usedLabel)
{
	const int len = H*W;
	// store result of optimization
	//int *result = new int[len];

	//first set up the array for data costs
	int *data = new int[len*totalLabel];
	for(int i=0;i<len;i++)
		for(int j=1;j<=totalLabel;j++)
		{
			//data[i*totalLabel+j-1]=2;
			if(j == labelMap[i])
				data[i*totalLabel+j-1]=0;
			else
			{
				if(labelMap[i]==totalLabel)
					data[i*totalLabel+j-1]=1;
				else
					data[i*totalLabel+j-1]=2;
			}
		}
    
	//next set up the array for smooth costs
	int *smooth = new int[totalLabel*totalLabel];
	for(int i=1;i<=totalLabel;i++)
		for(int j=1;j<=totalLabel;j++)
		{
			//smooth[i-1+(j-1)*totalLabel]=0;
			if(i>usedLabel && i<totalLabel)
				smooth[i-1+(j-1)*totalLabel]=100;
			else
			{
				if(i==j)
					smooth[i-1+(j-1)*totalLabel]=0;
				else
				{
					if(j>usedLabel && j<totalLabel)
						smooth[i-1+(j-1)*totalLabel]=100;
					else
						smooth[i-1+(j-1)*totalLabel]=2;
				}
			}
		}

	//construct Graph
	try
	{
		GCoptimizationGeneralGraph *gc=new GCoptimizationGeneralGraph(len,totalLabel);
		gc->setDataCost(data);
		gc->setSmoothCost(smooth);

		//now set up a grid neighborhood system
		//Boundary indicator e^(-b(Ix-Iy)^a)
		long double temp;
		int p1,p2;
		for(int i=0;i<H-1;i++)
			for(int j=0;j<W-1;j++)
			{
				p1=i*W+j;
				//Horizontal
				p2=i*W+j+1;
				temp = exp( -beta_at * pow(abs_dsl(yImg[p1]-yImg[p2]),gamm_at) );
				if(temp > threshold)
					gc->setNeighbors(p1,p2,1);
				//Vertical
				p2=(i+1)*W+j;
				temp = exp( -beta_at * pow(abs_dsl(yImg[p1]-yImg[p2]),gamm_at) );
				if(temp > threshold)
					gc->setNeighbors(p1,p2,1);
			}

		//Calculate
		gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);

		//get Result
		for(int i=0;i<len;i++)
			//result[i] = gc->whatLabel(i);
			labelMap[i] = gc->whatLabel(i)+1;

		delete gc;

	}catch (GCException e)
	{
		e.Report();
		//delete []result;
		delete []smooth;
		delete []data;
		return 1;
	}
	//delete []result;
	delete []smooth;
	delete []data;
	return 0;
}