// Edit by Disl 2015.12
#include "GlobalDefine.h"

const char *fileName = "./data/kitchen.yuv";
const char *keyMapName   = "./data/kitchenlabel.mat";
const char *colorTableName = "./data/colortable.mat";

//kitchen 13 conference 6 bookstore 3
const int labelNum = 13;
const int groundLabel=20;
//Sequence Parameter
const int frameNum = 30;
const int gopNum = 15;

int main()
{
	cout<<"Hello,World!"<<endl;

	//Variable
	MATFile *pColorTableFile = NULL;
	mxArray *pMxColorTable = NULL;
	double *dColorTable = NULL; //Assume it is double
	MATFile *pKeyLabelFile = NULL;
	mxArray *pMxKeyLabel = NULL;
	double *dKeyLabel = NULL;
	int fileH,fileW;
	//time temp
	clock_t time_start,time_end;
	//read YUV file
	fstream yuvFile(fileName,ios::binary|ios::in); //must set ios::binary
	if(!yuvFile)
	{
		cout<<"Wrong YUV File"<<endl;
		return 0;
	}

	//read colorTable
	pColorTableFile = matOpen(colorTableName,"r");
	if(!pColorTableFile)
	{
		cout<<"ColorTable File Error!"<<endl;
		return 0;
	}
	pMxColorTable = matGetVariable(pColorTableFile,"Color_sample");
	if(!pMxColorTable)
	{
		cout<<"ColorTable Variable Error!"<<endl;
		return 0;
	}
	dColorTable = (double*)mxGetData(pMxColorTable);
	int colorTableM = mxGetM(pMxColorTable); //Get row
	int colorTableN = mxGetN(pMxColorTable); //Get column
	//cout<<colorTableM<<"*"<<colorTableN<<endl;
    //convert
	int *colorTable = NULL;
	colorTable = new int[colorTableM*colorTableN];
	for(int i=0;i<colorTableM;i++)
		for(int j=0;j<colorTableN;j++)
		{
			colorTable[i*colorTableN+j]=(int)dColorTable[j*colorTableM+i];
			//cout<<colorTable[i*colorTableN+j]<<" ";
		}

    //read key-frame label
	pKeyLabelFile = matOpen(keyMapName,"r");
	if(!pKeyLabelFile)
	{
		cout<<"KeyLabel File Error!"<<endl;
		return 0;
	}
	pMxKeyLabel = matGetVariable(pKeyLabelFile,"labelmap");
	if(!pMxKeyLabel)
	{
		cout<<"KeyLabel Variable Error!"<<endl;
		return 0;
	}
	dKeyLabel = (double*) mxGetData(pMxKeyLabel);
	fileH = mxGetM(pMxKeyLabel);
	fileW = mxGetN(pMxKeyLabel);
    //cout<<fileH<<"*"<<fileW<<endl;
	//convert
	framePram framePramArray[frameNum];
	for(int i=0;i<frameNum;i++)
		framePramArray[i].initData(fileH,fileW);
	for(int i=0;i<fileH;i++)
		for(int j=0;j<fileW;j++)
		{
			framePramArray[0].labelMap[i*fileW+j] = (unsigned char)dKeyLabel[j*fileH+i];
			//cout<<keyLabel[i*fileW+j]<<" ";
			//For pre-Process : 0-Label to 20
			if(!(framePramArray[0].labelMap[i*fileW+j]))
				framePramArray[0].labelMap[i*fileW+j]=groundLabel;
		}
	
    //import YUV File
	unsigned char *Uarray=new unsigned char[(fileH*fileW)>>2];
	unsigned char *Varray=new unsigned char[(fileH*fileW)>>2];
	
	time_start = clock();
	for(int i=0;i<frameNum;i++)
	{
		yuvFile.read((char*)(framePramArray[i].yImg),fileH*fileW);
		
		//if(i==0)
			//cout<<yuvFile.gcount()<<endl;
		
		yuvFile.read((char*)Uarray,(fileH*fileW)>>2);
		yuvFile.read((char*)Varray,(fileH*fileW)>>2);
		
		for(int j=0;j<fileH;j++)
			for(int k=0;k<fileW;k++)
				framePramArray[i].cvGray.ptr<unsigned char>(j)[k] = framePramArray[i].yImg[j*fileW+k];
			
	}
	time_end = clock();
	/*
	cout<<(int)Yarray[0][0]<<" "<<(int)Yarray[19][640]<<" "<<(int)Yarray[29][29]<<endl;
	cout<<(int)cvGrayArray[0].ptr<unsigned char>(0)[0]<<" "<<(int)cvGrayArray[19].ptr<unsigned char>(1)[0]<<" "<<(int)cvGrayArray[29].ptr<unsigned char>(0)[29]<<endl;
	*/
	cout<<"YUV File Convert Time: "<< (double)(time_end-time_start)/CLOCKS_PER_SEC*1000<<" ms"<<endl;
	
	//fill blank of label
	time_start = clock();
	fillBlank(framePramArray[0].labelMap,framePramArray[0].yImg,fileH,fileW);
	time_end = clock();
	cout<<"Key LabelMap FillBlank Time: "<< (double)(time_end-time_start)/CLOCKS_PER_SEC*1000<<" ms"<<endl;
	
	//Generate Label Map

	cout<<"Start Label Propagation"<<endl;
	//For P-thread
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr,PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
	pthread_attr_setschedpolicy(&attr,SCHED_FIFO);
	threadPram paramArray[gopNum];
	pthread_t pid[gopNum];
	//Index pointer
	int pointerI=0;
	int k=1;
	int rightBound; //[k,rightBound)
	//Time statistical
	time_start = clock();
	while(k<frameNum)
	{
		//k is index for Frame k+1
		//range : [ k , min(frameNum,k+gopNum) )
		rightBound = frameNum<k+gopNum ? frameNum : k+gopNum ;
		for(int j=0;j<rightBound-k;j++)
		{
			paramArray[j].index=k+j+1;
			paramArray[j].H=fileH;
			paramArray[j].W=fileW;
			paramArray[j].frameI = & (framePramArray[pointerI]);
			paramArray[j].frameP = & (framePramArray[k+j]);

			//Create Thread
			pthread_create(&(pid[j]),&attr,Function_t,&(paramArray[j]));
		}
		//wait all over
		for(int j=0;j<rightBound-k;j++)
			pthread_join(pid[j],NULL);
		k+=gopNum;
		pointerI=k-1;
	}
	time_end = clock();
	cout<<"Label Propagation Time: "<< (double)(time_end - time_start)/CLOCKS_PER_SEC*1000<<" ms"<<endl;

	//save result

	//temp Variable for Filename
	char resultLabelMapName[100];
	MATFile *pOutMatFile=NULL;
	mxArray *pRes=NULL;

	for(int i=1;i<frameNum;i++)
	{
		sprintf(resultLabelMapName,"./data/kitchen_%d.mat",i+1);	
		pOutMatFile = matOpen(resultLabelMapName,"w");
		if(!pOutMatFile)
		{
			cout<<"Out File Error"<<endl;
			return 0;
		}

		for(int j=0;j<fileH;j++)
			for(int k=0;k<fileW;k++)
				dKeyLabel[k*fileH+j]=(double)(framePramArray[i].labelMap[j*fileW+k]);
		pRes = mxCreateDoubleMatrix(fileH,fileW,mxREAL);
		if(!pRes)
		{
			cout<<"Out Variable Error"<<endl;
			return 0;
		}
		mxSetData(pRes,dKeyLabel);
		matPutVariable(pOutMatFile,"nextFrameLabel",pRes);
		matClose(pOutMatFile);
	}

	//delete
	delete []Uarray;
	delete []Varray;

	pthread_attr_destroy(&attr);
	yuvFile.close();
	matClose(pColorTableFile);
	matClose(pKeyLabelFile);
	mxFree(dColorTable);
	mxFree(dKeyLabel);
	delete []colorTable;
}

// class function
void framePram::initData(int H,int W)
{
	this->yImg = new unsigned char[H*W*2];
	this->labelMap = this->yImg + H*W;
	for(int i=0;i<H;i++)
		for(int j=0;j<W;j++)
			this->labelMap[i*W+j] = 0;
	this->cvGray.create(H,W,CV_8UC(1));
}