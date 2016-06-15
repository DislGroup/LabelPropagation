// Edit by Disl 2015.12
#include "GlobalDefine.h"

void* Function_t(void* Param)
{
	threadPram *it = (threadPram*)Param;
	if(!it)
	{
		cout<<"Frame "<<it->index<<" Parameter Error!"<<endl;
		return 0;
	}
	//Test
	//cout<<"Frame "<<it->index<<" Start"<<endl;
	
	//Optical Flow
	if(upOpticalFlow(it->frameI->cvGray,it->frameP->cvGray,it->frameI->labelMap,it->frameP->labelMap))
	{
		cout<<"Frame "<<it->index<<" Optical Flow Error!"<<endl;
		return 0;
	}

	//Filter
	gaussianFilterSSE(it->frameP->yImg,it->H,it->W);
	
	//Graph cuts
	if(upGraphCuts(it->frameP->yImg,it->frameP->labelMap,it->H,it->W,groundLabel,labelNum))
	{
		cout<<"Frame"<<it->index<<" Graph Cuts Error!"<<endl;
		return 0;
	}

	pthread_mutex_lock(&mtxMaster);
	finishFrame--;
	if(finishFrame == 0)
		pthread_cond_signal(&cFinish);
	pthread_mutex_unlock(&mtxMaster);

	pthread_mutex_lock(&mtxRun);
	pthread_cond_signal(&cHungry);
	pthread_mutex_unlock(&mtxRun);

	//Test
	//cout<<"End"<<endl;
	return 0;
}