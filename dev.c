// ModbusDev.cpp: implementation of the CPubDev class.
//
//////////////////////////////////////////////////////////////////////

#include "dev.h"

IoGlobal * pGlobal=NULL;
SCADAUnit*   pScadaUnit=NULL;
////////////////////////////device des////////公共部分//////////////////////////////////////

int nCookie=0;

int* GetCookie()
{
    return &nCookie;
}
void AddCookie()
{
    nCookie++;
}
void ReleaseCookie()
{
    nCookie--;
}

static void FreeExDevice(void* pbyDeviceUnit)
{
	DeviceUnit * pDeviceUnit=(DeviceUnit*)pbyDeviceUnit;
	if(pDeviceUnit&&pDeviceUnit->pExDeviceUnit)
	{
		CPubDev	* pPubDev=(CPubDev*)pDeviceUnit->pExDeviceUnit;
		if (pPubDev->OnDestroy)
			pPubDev->OnDestroy(pPubDev);
		//////////////////////////////////////////////////////////////////////////
		while (pPubDev->m_pPollSendFrame_Head)
		{
			if (pPubDev->DestroyPollSendFrame(pPubDev,pPubDev->m_pPollSendFrame_Head)==FALSE)
				break;
		}
		while (pPubDev->m_pIntervalSend_Head)
		{
			pPubDev->m_pIntervalSend_Head->m_IntervalSendInfo.m_bWillDestroy=TRUE;
			if (pPubDev->DestroyIntervalSend(pPubDev,pPubDev->m_pIntervalSend_Head)==FALSE)
				break;
		}
		while (pPubDev->m_pYxDelayExplain_Head)
		{
			if (pPubDev->DeleteYxDelayExplainNode(pPubDev,pPubDev->m_pYxDelayExplain_Head)==FALSE)
				break;
		}
		//////////////////////////////////////////////////////////////////////////
		{
			SENDFRAME_LIST_NODE *pNow=pPubDev->m_pInsertSendFrame_Head,*pPrev;
			pPubDev->m_pInsertSendFrame_Head=NULL;
			pPubDev->m_pInsertSendFrame_Tail=NULL;
			while (pNow)
			{
				pPrev=pNow;
				pNow=pPrev->m_pNext;
				free(pPrev->m_PollSendFrameInfo.m_pData);
				free(pPrev);
			}
		}
		free(pPubDev->m_NowPollingInfo.m_pData);
		//////////////////////////////////////////////////////////////////////////
		free(pPubDev);
		pDeviceUnit->pExDeviceUnit=NULL;
	}
}


static void FinishInitExDevice(void* pbyDeviceUnit)
{
	DeviceUnit * pDeviceUnit=(DeviceUnit*)pbyDeviceUnit;
	if(pDeviceUnit)
	{
		CPubDev * pPubDev=(CPubDev*)pDeviceUnit->pExDeviceUnit;
	
	}
}


// 
// #define  EXPLAIN_YC_LINE(_startyc,_num,_ExpressValue,_epoint) \
// {\
// 	pData=&byBuf[_startyc*2];\
// 	for(i=0;i<_num;i++)\
// {\
// 	SetIDNo(ID_IDLDEVICE,\
// 	pChannelUnit->m_ChannelNo,\
// 	pDeviceUnit->m_DeviceNo,\
// 	_epoint+i,&nIDNo);\
// 	*(long*)pExData=nIDNo;\
// 	wValue=GetRegisterData(pData,i,TRUE);\
// 	*(float*)&pExData[4]=(float)(_ExpressValue);\
// 	ExplainYC(pGlobal,pExData,8);\
// }\
// }
// 
// #define  EXPLAIN_YC_LINE32(_startyc,_num,_ExpressValue,_epoint) \
// {\
// 	pData=&byBuf[_startyc*2];\
// 	for(i=0;i<_num;i++)\
// {\
// 	SetIDNo(ID_IDLDEVICE,\
// 	pChannelUnit->m_ChannelNo,\
// 	pDeviceUnit->m_DeviceNo,\
// 	_epoint+i,&nIDNo);\
// 	*(long*)pExData=nIDNo;\
// 	WORD wValueH=GetRegisterData(pData,i*2,TRUE);\
// 	WORD wValueL=GetRegisterData(pData,i*2+1,TRUE);\
// 	dValue=(wValueH<<16)+wValueL;\
// 	*(float*)&pExData[4]=(float)(_ExpressValue);\
// 	ExplainYC(pGlobal,pExData,8);\
// }\
// }
// 

///////////////////////////设备特性部分///////////////////////////////////////////////



static inline void OpenRawMode(CPubDev *pPubDev)
{
	if (pPubDev->m_bInitComplete){
		DEBUG_PRINT(pPubDev,"Only In InitCommunication to Open Raw Mode!");
		return;
	}

	if (pPubDev->m_bRawMode)
		return;
	DeviceUnit *pDeviceUnit=pPubDev->pDeviceUnit;
	
	pPubDev->m_bRawMode=TRUE;
	pDeviceUnit->m_FrameInterface.m_szFrameModbulName[0]='\0';
	DEBUG_PRINT(pPubDev,"Open Raw Mode!");
}

static void SetFrameModule(struct _PubDev *pPubDev,const char szFrameModbulName[256])
{
	if (pPubDev->m_bInitComplete){
		DEBUG_PRINT(pPubDev,"Only In InitCommunication to Set FrameModbul!");
		return ;
	}
	if (szFrameModbulName==NULL){
		pPubDev->OpenRawMode(pPubDev);
		return ;
	}

	if (pPubDev->m_bRawMode)
		pPubDev->m_bRawMode=FALSE;
	DeviceUnit *pDeviceUnit=pPubDev->pDeviceUnit;
	
	strcpy(pDeviceUnit->m_FrameInterface.m_szFrameModbulName,szFrameModbulName);
	if (strchr(szFrameModbulName,'.')==NULL)
		strcat(pDeviceUnit->m_FrameInterface.m_szFrameModbulName,".so");
	DEBUG_PRINT(pPubDev,"SetFrameModule:%s",pDeviceUnit->m_FrameInterface.m_szFrameModbulName);
	return ;
}

static inline void RegisterPrePolling(struct _PubDev *pPubDev,PREPOLLING_CALLBACK pfnPrePolling)
{
	pPubDev->PrePolling=pfnPrePolling;
}

static inline void RegisterDefaultRecvCallback(struct _PubDev *pPubDev,RECEIVE_CALLBACK pfnRecvCallBack)
{
	pPubDev->DefaultRecvCallback=pfnRecvCallBack;
}

static inline void RegisterOnDestroy(struct _PubDev *pPubDev,DESTROY_CALLBACK pfnOnDestroy)
{
	pPubDev->OnDestroy=pfnOnDestroy;
}

static SENDFRAME_LIST_NODE* RegisterPollSendFrame(struct _PubDev *pPubDev,BYTE byFrameType,BYTE byCmd, BYTE *pData,WORD wSize,RECEIVE_CALLBACK pfnRecvCallBack)
{
	SENDFRAME_LIST_NODE *pPollSendFrameNode;
	if ((pPollSendFrameNode=malloc(sizeof(SENDFRAME_LIST)))==NULL ||
		(pPollSendFrameNode->m_PollSendFrameInfo.m_pData=malloc(wSize))==NULL)
	{
		DEBUG_PRINT(pPubDev,"RegisterPoll Error: malloc Failed, PollID:%d",pPubDev->m_nRegisteredPollCount+1);
		return NULL;
	}

//	DEBUG_PRINT(pPubDev,"PollID:%d",pPubDev->m_nRegisteredPollCount+1);
	memcpy(pPollSendFrameNode->m_PollSendFrameInfo.m_pData,pData,wSize);
	pPollSendFrameNode->m_PollSendFrameInfo.m_byFrameType=byFrameType;
	pPollSendFrameNode->m_PollSendFrameInfo.m_cbDataSize=wSize;
	pPollSendFrameNode->m_PollSendFrameInfo.m_byCmd=byCmd;
	pPollSendFrameNode->m_PollSendFrameInfo.m_pfnReceiveCallback=pfnRecvCallBack;

	if (pPubDev->m_pPollSendFrame_Head==NULL || pPubDev->m_pPollSendFrame_Tail==NULL)
	{
		if (pPubDev->m_pInsertSendFrame_Head)
			pPubDev->DestroyPollSendFrame(pPubDev,pPubDev->m_pInsertSendFrame_Head);
		if (pPubDev->m_pInsertSendFrame_Tail)
			pPubDev->DestroyPollSendFrame(pPubDev,pPubDev->m_pInsertSendFrame_Tail);
		pPubDev->m_pPollSendFrame_Head=pPollSendFrameNode;
		pPubDev->m_pPollSendFrame_Tail=pPollSendFrameNode;
	}
	else
	{
		pPubDev->m_pPollSendFrame_Tail->m_pNext=pPollSendFrameNode;
		pPubDev->m_pPollSendFrame_Tail=pPollSendFrameNode;
	}
	
	pPollSendFrameNode->m_pNext=pPubDev->m_pPollSendFrame_Head;
	pPollSendFrameNode->m_nID=++pPubDev->m_nRegisteredPollCount;
	
	return pPollSendFrameNode;
}

static inline SENDFRAME_LIST_NODE* RegisterPollSendFrame_ModbusAsk(struct _PubDev *pPubDev,BYTE byCmd, WORD wRegAddr,WORD wRegNum,RECEIVE_CALLBACK pfnRecvCallBack)
{
	if (pPubDev->m_bRawMode)
	{
		DEBUG_PRINT(pPubDev,"CAN NOT RegisterPollSendFrame_ModbusAsk ON RAWMODE! ");
		return FALSE;
	}
	BYTE pData[8];
	WORD wSize=4;
	*(WORD*)&pData[0]=htons(wRegAddr);
	*(WORD*)&pData[2]=htons(wRegNum);
	
	return RegisterPollSendFrame(pPubDev,0,byCmd,pData,wSize,pfnRecvCallBack);
}

static BOOL DestroyPollSendFrame(struct _PubDev *pPubDev,SENDFRAME_LIST_NODE *pPollSendFrameNode)
{
	if (pPollSendFrameNode==NULL)
		pPollSendFrameNode=pPubDev->m_pPollSendFrame_Now;//传入NULL即为当前POLL
	SENDFRAME_LIST_NODE *pTmp=pPubDev->m_pPollSendFrame_Head;
	SENDFRAME_LIST_NODE *pPrev=pPubDev->m_pPollSendFrame_Tail;
	//DEBUG_PRINT(pPubDev,"pDestroyPoll=%o   pTmp=%o",pDestroyPoll,pTmp);
	while (pTmp && pTmp!=pPollSendFrameNode && pTmp->m_pNext!=pPubDev->m_pPollSendFrame_Head)
		pPrev=pTmp,pTmp=pTmp->m_pNext;
	if (pTmp==pPollSendFrameNode)
	{
		if (pTmp==pPubDev->m_pPollSendFrame_Head)
		{
			if (pPubDev->m_pPollSendFrame_Head->m_pNext==pPubDev->m_pPollSendFrame_Head)
			{
				pPubDev->m_pPollSendFrame_Head=pPubDev->m_pPollSendFrame_Tail=NULL;
				//DEBUG_PRINT(pPubDev,"Error: POLL链表已空");
				//return FALSE;
			}
			else
			{
				pPrev->m_pNext=pTmp->m_pNext;
				pPubDev->m_pPollSendFrame_Head=pTmp->m_pNext;
			}
		}
		else if (pTmp==pPubDev->m_pPollSendFrame_Tail)
		{
			pPrev->m_pNext=pTmp->m_pNext;
			pPubDev->m_pPollSendFrame_Tail=pPrev;
		}
		else
		{
			pPrev->m_pNext=pTmp->m_pNext;
		}
		if (pTmp==pPubDev->m_pPollSendFrame_Now)
			pPubDev->m_pPollSendFrame_Now=pPrev;
		free(pTmp->m_PollSendFrameInfo.m_pData);
		free(pTmp);
		return TRUE;
	}
	else if (pTmp==NULL)
	{
		DEBUG_PRINT(pPubDev,"Error: POLL链表断开");
		return FALSE;
	}
	else
	{
		DEBUG_PRINT(pPubDev,"Error: 未找到要销毁的链表项");
		return FALSE;
	}
	return TRUE;
}
static inline void IntervalSend_SetTime(struct INTERVALSEND_NODE_TAG *pIntervalSendListNode)
{pIntervalSendListNode->m_IntervalSendInfo.m_tSendTime=time(NULL);}
static inline void IntervalSend_ResetTime(struct INTERVALSEND_NODE_TAG *pIntervalSendListNode)
{pIntervalSendListNode->m_IntervalSendInfo.m_tSendTime=0;}
static inline void IntervalSend_SetAutoSetTime(struct INTERVALSEND_NODE_TAG *pIntervalSendListNode,BOOL bAutoSetTime)
{pIntervalSendListNode->m_IntervalSendInfo.m_bAutoSetTime=bAutoSetTime;}
static inline void IntervalSend_SetInterval(struct INTERVALSEND_NODE_TAG *pIntervalSendListNode,WORD wIntervalSec)
{pIntervalSendListNode->m_IntervalSendInfo.m_wIntervalSec=wIntervalSec;}
static inline void IntervalSend_Destroy(struct INTERVALSEND_NODE_TAG *pIntervalSendListNode)
{pIntervalSendListNode->m_IntervalSendInfo.m_bWillDestroy=TRUE;}

static inline void IntervalSend_Init(CPubDev *pPubDev,INTERVALSEND_LIST_NODE *pIntervalSendNode)
{
	if (!pIntervalSendNode)
		return ;
	memset(pIntervalSendNode,0,sizeof(INTERVALSEND_LIST_NODE));
	pIntervalSendNode->SetTime=IntervalSend_SetTime;
	pIntervalSendNode->ResetTime=IntervalSend_ResetTime;
	pIntervalSendNode->SetAutoSetTime=IntervalSend_SetAutoSetTime;
	pIntervalSendNode->SetInterval=IntervalSend_SetInterval;
	pIntervalSendNode->Destroy=IntervalSend_Destroy;
}
static BOOL IntervalSend_Polling(struct _PubDev *pPubDev)
{
	if (!pPubDev)
		return FALSE;
	//reset m_pIntervalSend_Now
	pPubDev->m_pIntervalSend_Now=NULL;

	time_t tNow=time(NULL);
	INTERVALSEND_LIST_NODE *pNode=pPubDev->m_pIntervalSend_Head,*pTmp;
	INTERVALSEND_INFO *pInfo;
	while(pNode)
	{
		pInfo=&pNode->m_IntervalSendInfo;
		if (pInfo->m_bWillDestroy)
		{
			pTmp=pNode;
			pNode=pNode->m_pNext;
			pPubDev->DestroyIntervalSend(pPubDev,pTmp);
			continue;
		}
		if (tNow - pInfo->m_tSendTime >= pInfo->m_wIntervalSec && 
			pNode->m_IntervalSendInfo.m_pfnIntervalSendCallback &&
			pNode->m_IntervalSendInfo.m_pfnIntervalSendCallback(pPubDev,pNode))
		{
			DEBUG_PRINT(pPubDev,"IntervalSend(%d)%s %s",
				pNode->m_nID,
				!pInfo->m_bAutoSetTime?",Not AutoSetTime":"",
				pInfo->m_bOnceOnly?",OnceOnly":"");

			if (pInfo->m_bAutoSetTime)
				pNode->SetTime(pNode);

			pPubDev->m_pIntervalSend_Now=pNode;
			return TRUE;
		}
		pNode=pNode->m_pNext;
	}
	return FALSE;
}
static INTERVALSEND_LIST_NODE* RegisterIntervalSend(struct _PubDev *pPubDev,BOOL bOnceOnly,WORD wIntervalSec,INTERVALSEND_CALLBACK pfnIntervalSendCallBack)
{
	INTERVALSEND_LIST_NODE *pIntervalSendNode;
	if ((pIntervalSendNode=malloc(sizeof(INTERVALSEND_LIST)))==NULL )
	{
		DEBUG_PRINT(pPubDev,"RegisterIntervalSend Error: malloc Failed, RegisterID:%d",pPubDev->m_nRegisteredIntervalSendCount+1);
		return NULL;
	}
	//Init CPutDev IntervalSend_Polling
	pPubDev->IntervalSend_Polling=IntervalSend_Polling;
	
	IntervalSend_Init(pPubDev,pIntervalSendNode);
	pIntervalSendNode->m_IntervalSendInfo.m_bAutoSetTime=TRUE;
	pIntervalSendNode->m_IntervalSendInfo.m_bOnceOnly=bOnceOnly;
	pIntervalSendNode->m_IntervalSendInfo.m_wIntervalSec=wIntervalSec;
	pIntervalSendNode->m_IntervalSendInfo.m_pfnIntervalSendCallback=pfnIntervalSendCallBack;
	if (bOnceOnly) pIntervalSendNode->m_IntervalSendInfo.m_tSendTime=time(NULL);

	DEBUG_PRINT(pPubDev,"Register IntervalSend (%d): IntervalSec=%d %s",
		pPubDev->m_nRegisteredIntervalSendCount+1,wIntervalSec,bOnceOnly?"SendOnceOnly":"");

	if (pPubDev->m_pIntervalSend_Head==NULL || pPubDev->m_pIntervalSend_Tail==NULL)
	{
		if (pPubDev->m_pIntervalSend_Head)
			pPubDev->DestroyIntervalSend(pPubDev,pPubDev->m_pIntervalSend_Head);
		if (pPubDev->m_pIntervalSend_Tail)
			pPubDev->DestroyIntervalSend(pPubDev,pPubDev->m_pIntervalSend_Tail);
		pPubDev->m_pIntervalSend_Head=pIntervalSendNode;
		pPubDev->m_pIntervalSend_Tail=pIntervalSendNode;
	}
	else
	{
		if (bOnceOnly)
		{
			INTERVALSEND_LIST_NODE *pNode=pPubDev->m_pIntervalSend_Head;
			while (pNode->m_pNext && pNode->m_pNext->m_IntervalSendInfo.m_bOnceOnly)
				pNode=pNode->m_pNext;
			if (pNode->m_pNext==NULL)
			{
				pPubDev->m_pIntervalSend_Tail->m_pNext=pIntervalSendNode;
				pPubDev->m_pIntervalSend_Tail=pIntervalSendNode;
			}
			else
			{
				pIntervalSendNode->m_pNext=pNode->m_pNext;
				pNode->m_pNext=pIntervalSendNode;
			}
		}
		else
		{
			pPubDev->m_pIntervalSend_Tail->m_pNext=pIntervalSendNode;
			pPubDev->m_pIntervalSend_Tail=pIntervalSendNode;
		}
	}

	pIntervalSendNode->m_nID=++pPubDev->m_nRegisteredIntervalSendCount;
	
	return pIntervalSendNode;
}
static BOOL DestroyIntervalSend(struct _PubDev *pPubDev,INTERVALSEND_LIST_NODE *pIntervalSendListNode)
{
	if (!pIntervalSendListNode)
		return FALSE;
	if (!pIntervalSendListNode->m_IntervalSendInfo.m_bWillDestroy)
	{
		pIntervalSendListNode->Destroy(pIntervalSendListNode);
		return TRUE;
	}	
	INTERVALSEND_LIST_NODE *pPrev,*pNode=pPubDev->m_pIntervalSend_Head;
	while (pNode && pNode!=pIntervalSendListNode)
		pPrev=pNode,pNode=pNode->m_pNext;
	if (pNode==pIntervalSendListNode)
	{
		DEBUG_PRINT(pPubDev,"Destroy IntervalSend(%d)",pNode->m_nID);

		if (pNode==pPubDev->m_pIntervalSend_Head)
		{
			pPubDev->m_pIntervalSend_Head=pNode->m_pNext;
			if (pPubDev->m_pIntervalSend_Head==NULL)
				pPubDev->m_pIntervalSend_Tail=NULL;
		}
		else if (pNode==pPubDev->m_pIntervalSend_Tail)
		{
			pPrev->m_pNext=pNode->m_pNext;
			pPubDev->m_pIntervalSend_Tail=pPrev;
		}
		else
			pPrev->m_pNext=pNode->m_pNext;
		free(pNode);
		return TRUE;
	}
	return TRUE;
}

static inline BOOL PollSendFrame(struct _PubDev *pPubDev,BYTE byFrameType,BYTE byCmd, BYTE *pSend,WORD wSize,RECEIVE_CALLBACK pfnRecvCallBack)
{
	DeviceUnit *pDeviceUnit=pPubDev->pDeviceUnit;
	
	pPubDev->m_NowPollingInfo.m_byFrameType=byFrameType;
	pPubDev->m_NowPollingInfo.m_byCmd=byCmd;
	pPubDev->m_NowPollingInfo.m_cbDataSize=wSize;
	memcpy(pPubDev->m_NowPollingInfo.m_pData,pSend,wSize);
	pPubDev->m_NowPollingInfo.m_pfnReceiveCallback=pfnRecvCallBack;
	
	if (pPubDev->m_bRawMode)
		pDeviceUnit->PollingRawFrame(pDeviceUnit,pSend,wSize,eCid_PollSendFrame);
	else
		pDeviceUnit->ComanndData(pDeviceUnit,byCmd,pSend,wSize,byFrameType,0,FALSE,eCid_PollSendFrame);
	
	return TRUE;
}

static BOOL InsertSendFrame(struct _PubDev *pPubDev,BYTE byFrameType,BYTE byCmd, BYTE *pData,WORD wSize,RECEIVE_CALLBACK pfnRecvCallBack)
{
	DeviceUnit *pDeviceUnit=pPubDev->pDeviceUnit;
	SENDFRAME_LIST_NODE *pInsertSendFrame_Node;
	if ((pInsertSendFrame_Node=malloc(sizeof(SENDFRAME_LIST_NODE)))==NULL ||
		(pInsertSendFrame_Node->m_PollSendFrameInfo.m_pData=malloc(wSize))==NULL)
	{
		DEBUG_PRINT(pPubDev,"InsertSendFrame Error: malloc Failed , PollID:%d",pPubDev->m_nRegisteredPollCount+1);
		return FALSE;
	}
	

	memcpy(pInsertSendFrame_Node->m_PollSendFrameInfo.m_pData,pData,wSize);
	pInsertSendFrame_Node->m_PollSendFrameInfo.m_byFrameType=byFrameType;
	pInsertSendFrame_Node->m_PollSendFrameInfo.m_cbDataSize=wSize;
	pInsertSendFrame_Node->m_PollSendFrameInfo.m_byCmd=byCmd;
	pInsertSendFrame_Node->m_PollSendFrameInfo.m_pfnReceiveCallback=pfnRecvCallBack;
	pInsertSendFrame_Node->m_pNext=NULL;
	if (pPubDev->m_pInsertSendFrame_Head==NULL || 
		pPubDev->m_pInsertSendFrame_Tail==NULL )
	{
		if (pPubDev->m_pInsertSendFrame_Head){
			if (pPubDev->m_pInsertSendFrame_Head->m_PollSendFrameInfo.m_pData)
				free(pPubDev->m_pInsertSendFrame_Head->m_PollSendFrameInfo.m_pData);
			free(pPubDev->m_pInsertSendFrame_Head);
			printf("\n free head");
		}
		if (pPubDev->m_pInsertSendFrame_Tail){
			if (pPubDev->m_pInsertSendFrame_Tail->m_PollSendFrameInfo.m_pData)
				free(pPubDev->m_pInsertSendFrame_Tail->m_PollSendFrameInfo.m_pData);
			free(pPubDev->m_pInsertSendFrame_Tail);
			printf("\n free tail");
		}
		pPubDev->m_pInsertSendFrame_Head=pInsertSendFrame_Node;
		pPubDev->m_pInsertSendFrame_Tail=pInsertSendFrame_Node;
	}
	else
	{
		pPubDev->m_pInsertSendFrame_Tail->m_pNext=pInsertSendFrame_Node;
		pPubDev->m_pInsertSendFrame_Tail=pInsertSendFrame_Node;
	}
	pInsertSendFrame_Node->m_nID=++pPubDev->m_nRegisteredPollCount;

	if (pPubDev->m_bRawMode)
		pDeviceUnit->InsertRawFrame(pDeviceUnit,pData,wSize,eCid_InsertSendFrame);
	else
		pDeviceUnit->ComanndData(pDeviceUnit,byCmd,pData,wSize,byFrameType,0,TRUE,eCid_InsertSendFrame);//插帧
	DEBUG_PRINT(pPubDev,"Insert Frame!!!");
	return TRUE;

}

static inline BOOL SendFrame(struct _PubDev *pPubDev,BYTE byFrameType,BYTE byCmd, BYTE *pSend,WORD wSize,RECEIVE_CALLBACK pfnRecvCallBack)
{
	if (pPubDev->m_bInPollingSend)
		pPubDev->PollSendFrame(pPubDev,byFrameType,byCmd,pSend,wSize,pfnRecvCallBack);
	else
		pPubDev->InsertSendFrame(pPubDev,byFrameType,byCmd,pSend,wSize,pfnRecvCallBack);
	
	return TRUE;
}
static inline BOOL SendFrame_ModbusAsk(struct _PubDev *pPubDev,BYTE byCmd, WORD wRegAddr,WORD wRegNum,RECEIVE_CALLBACK pfnRecvCallBack)
{
	if (pPubDev->m_bRawMode)
	{
		DEBUG_PRINT(pPubDev,"CAN NOT SendFrame_ModbusAsk ON RAWMODE! ");
		return FALSE;
	}
	BYTE pData[8];
	WORD wSize=4;
	*(WORD*)&pData[0]=htons(wRegAddr);
	*(WORD*)&pData[2]=htons(wRegNum);
	
	return pPubDev->SendFrame(pPubDev,0,byCmd,pData,wSize,pfnRecvCallBack);
}

static inline void ExplainYc(struct _PubDev *pPubDev,WORD wYcindex,float fValue)
{
	BYTE pxxxxExData[8];
	*(long*)pxxxxExData=TRYXYCIDNO(0,pPubDev->pChannelUnit->m_ChannelNo,pPubDev->pDeviceUnit->m_DeviceNo,wYcindex);
	*(float*)&pxxxxExData[4]=(float)(fValue);
	ExplainYC(pGlobal,pxxxxExData,8);
}
static inline void ExplainYx(struct _PubDev *pPubDev,WORD wYxindex,BOOL bOnoff)
{
	BYTE pxxxxExData[8];
	*(long*)pxxxxExData=TRYXYCIDNO(0,pPubDev->pChannelUnit->m_ChannelNo,pPubDev->pDeviceUnit->m_DeviceNo,wYxindex);
	*(long*)&pxxxxExData[4]=bOnoff?1:0;
	ExplainYX(pGlobal,pxxxxExData,8);
	//////////////////////////////////////////////////////////////////////////
	//删除延时解析遥信
	YX_DELAY_EXPLAIN_NODE* pNode;
	if (pNode=pPubDev->FindYxDelayExplainNode(pPubDev,wYxindex))
	{
		pPubDev->DeleteYxDelayExplainNode(pPubDev,pNode);
	}
}
static void InsertYxDelayExplainNode(struct _PubDev *pPubDev,YX_DELAY_EXPLAIN_NODE InsertionNode)
{
	YX_DELAY_EXPLAIN_NODE *pNode=(YX_DELAY_EXPLAIN_NODE*)malloc(sizeof(YX_DELAY_EXPLAIN_NODE));
	if (pNode)
	{
		memcpy(pNode,&InsertionNode,sizeof(YX_DELAY_EXPLAIN_NODE));
		if (pPubDev->m_pYxDelayExplain_Tail && 
			pPubDev->m_pYxDelayExplain_Tail->tExplainTime < pNode->tExplainTime)
		{
			pPubDev->m_pYxDelayExplain_Tail->pNext=pNode;
			pPubDev->m_pYxDelayExplain_Tail=pNode;
			pNode->pNext=NULL;
		}
		else
		{
			YX_DELAY_EXPLAIN_NODE *pNow=NULL,*pPrev=NULL;
			pNow=pPubDev->m_pYxDelayExplain_Head;
			while (pNow)
			{
				if (pNow->tExplainTime>pNode->tExplainTime)
					break;
				
				pPrev=pNow;
				pNow=pNow->pNext;
			}
			
			if (pPrev==NULL || pPubDev->m_pYxDelayExplain_Head==NULL)//Head
			{
				pNode->pNext=pPubDev->m_pYxDelayExplain_Head;
				pPubDev->m_pYxDelayExplain_Head=pNode;
				if (pPubDev->m_pYxDelayExplain_Tail==NULL)
					pPubDev->m_pYxDelayExplain_Tail=pPubDev->m_pYxDelayExplain_Head;
			}
			else
			{
				pPrev->pNext=pNode;
				pNode->pNext=pNow;
				
				if (pNow==NULL)
					pPubDev->m_pYxDelayExplain_Tail=pNode;
			}
		}
	}//if (pNode)

}
static BOOL DeleteYxDelayExplainNode(struct _PubDev *pPubDev,YX_DELAY_EXPLAIN_NODE* pInsertion)
{
	if (pInsertion==NULL)
		return FALSE;
	YX_DELAY_EXPLAIN_NODE *pNow=NULL,*pPrev=NULL;
	pNow=pPubDev->m_pYxDelayExplain_Head;
	while(pNow)
	{
		if (pNow==pInsertion)
			break;

		pPrev=pNow;
		pNow=pNow->pNext;
	}

	if (pNow==pPubDev->m_pYxDelayExplain_Head)
	{
		pPubDev->m_pYxDelayExplain_Head=pNow->pNext;
		if (pPubDev->m_pYxDelayExplain_Head==NULL)
			pPubDev->m_pYxDelayExplain_Tail=NULL;
		free(pNow);
		return TRUE;
	}
	else if (pNow)
	{
		pPrev->pNext=pNow->pNext;
		if (pPubDev->m_pYxDelayExplain_Tail==pNow)
			pPubDev->m_pYxDelayExplain_Tail=pPrev;
		free(pNow);
		return TRUE;
	}
	return FALSE;
}
static inline YX_DELAY_EXPLAIN_NODE* FindYxDelayExplainNode(struct _PubDev *pPubDev,WORD wYxindex)
{
	YX_DELAY_EXPLAIN_NODE *pNode=pPubDev->m_pYxDelayExplain_Head;
	while (pNode)
	{
		if (pNode->wYxindex==wYxindex)
			return pNode;
		pNode=pNode->pNext;
	}
	return NULL;
}
static inline void DelayExplainYx(struct _PubDev *pPubDev,WORD wYxindex,BOOL bOnoff,WORD nDelay)
{
	YX_DELAY_EXPLAIN_NODE YxDelayExplainNode={0};
	YxDelayExplainNode.wYxindex=wYxindex;
	YxDelayExplainNode.bExplainState=bOnoff;
	YxDelayExplainNode.tExplainTime=time(NULL)+nDelay;
	
	TRACE("Delay Explain YX:Index=%d,State=%d,Delay=%dS",wYxindex,bOnoff,nDelay);
	pPubDev->InsertYxDelayExplainNode(pPubDev,YxDelayExplainNode);
}
static inline BOOL GetYcValue(struct _PubDev *pPubDev,WORD wYcindex,float *pfReturn)
{
	LONG nIDNo=TRYXYCIDNO(0,pPubDev->pChannelUnit->m_ChannelNo,pPubDev->pDeviceUnit->m_DeviceNo,wYcindex);
	IoGlobal * pIoGlobal=(IoGlobal *)pGlobal;
	SCADAUnit * pScadaUnit=NULL;
	Analog* pAnalog=NULL;
	int b=0;
	if(pIoGlobal)
	{
		pScadaUnit=pIoGlobal->pScadaUnit;
		YCUnit *m_node=pScadaUnit->GetYCNode(pScadaUnit,nIDNo);
		if (m_node==NULL || m_node->m_pAnalog==NULL)
			return FALSE;
		pAnalog=m_node->m_pAnalog;
		*pfReturn=pAnalog->m_RealValue;
		return TRUE;
	}
	return FALSE;
}
static inline BOOL GetYxValue(struct _PubDev *pPubDev,WORD wYxindex,BOOL *pfReturn)
{
	LONG nIDNo=TRYXYCIDNO(0,pPubDev->pChannelUnit->m_ChannelNo,pPubDev->pDeviceUnit->m_DeviceNo,wYxindex);
	IoGlobal * pIoGlobal=(IoGlobal *)pGlobal;
	SCADAUnit * pScadaUnit=NULL;
	Digital* pDigital=NULL;
	int b=0;
	if(pIoGlobal)
	{
		pScadaUnit=pIoGlobal->pScadaUnit;
		YXUnit *m_node=pScadaUnit->GetYXNode(pScadaUnit,nIDNo);
		if (m_node==NULL || m_node->m_pDigital==NULL)
			return FALSE;
		pDigital=m_node->m_pDigital;
		*pfReturn=pDigital->m_RealValue;
		return TRUE;
	}
	return FALSE;
}
static inline void ExplainYxByte(struct _PubDev *pPubDev,WORD wYxindex,BYTE byte)
{
	int i;
	for (i=0;i<8;i++)
	{
		pPubDev->ExplainYx(pPubDev,wYxindex+i,(byte>>i)&1);
	}
}
static inline void RespondResult_YK(struct _PubDev *pPubDev,enum yk_Kind YkKind,BOOL bSuccess)
{
	DeviceUnit *pDeviceUnit=pPubDev->pDeviceUnit;
	ChannelUnit *pChannelUnit=pPubDev->pChannelUnit;
	BYTE byResult=bSuccess?r_Succeed:r_Failed;

	pDeviceUnit->m_rResult=byResult;
	pDeviceUnit->SetInnerEcho(pScadaUnit,pDeviceUnit,k_Yk,YkKind);
	if (YkKind==k_Execute && bSuccess)
		pChannelUnit->SetKeepOne(pChannelUnit,TRUE,3);
}
static inline void RespondResult_FixValue(struct _PubDev * pPubDev,WORD wStartIdx,WORD wNum,WORD awFixValue[])
{
	if (pPubDev->m_byCache_FixValue_QH==0)
	{
		//内部规约
		WORD wSize=4+wNum*2;
		BYTE *pData=malloc(wSize);
		if (pData==NULL)
		{
			TRACE("RespondResult_FixValue: malloc Failed");
			return;
		}
		pData[0]=pPubDev->m_bCache_FixValue_Writing?k_Execute:k_Select;//func
		pData[1]=pPubDev->m_byCache_FixValue_QH;//qh
		g_DP.SetWordValue(pData,1,wStartIdx,FALSE);
		g_DP.SetWordValue(pData,2,wNum,FALSE);
		pPubDev->InnerChannelSendData(pPubDev,pData,wSize,k_FixValue);
		free(pData);
	}
}
static inline BOOL InitSoeUnit(struct _PubDev *pPubDev,SOE_UNIT *pSoeUnit)
{
	if (pSoeUnit==NULL)
		return FALSE;

	pSoeUnit->m_Time=pPubDev->GetDateTime(pPubDev);
	
	pSoeUnit->m_bYxState=0;
	pSoeUnit->m_nYxIndex=0;
	pSoeUnit->m_fValue=0.0;
	return TRUE;
}

static inline BOOL ExplainSoe(struct _PubDev *pPubDev,SOE_UNIT *pSoeUnit)
{
	DeviceUnit *pDeviceUnit=pPubDev->pDeviceUnit;
	ChannelUnit *pChannelUnit=pPubDev->pChannelUnit;
	DataTable * pDataTable=pDeviceUnit->pDataTable;

	if (!pSoeUnit||!pDataTable||!pDataTable->pYXTable)
		return FALSE;

	YXTable *pYXTable=pDataTable->pYXTable;


	struct timeval tv;
	tv.tv_sec=time(NULL);
	struct tm* p=localtime(&tv.tv_sec);

	YXUnit * pYXUnit;
	YXSoeUnit * pYXSoeUnit=(YXSoeUnit*)malloc(sizeof(YXSoeUnit));
	if(!pScadaUnit||!pScadaUnit->SendSOE)
	{
		free(pYXSoeUnit);
		return FALSE;
	}

	TIME_UNIT *pTimeUnit=&pSoeUnit->m_Time;
	p->tm_year=pTimeUnit->Year-1900;
	p->tm_mon=pTimeUnit->Mon-1;
	p->tm_mday=pTimeUnit->Mday;
	p->tm_hour=pTimeUnit->Hour;
	p->tm_min=pTimeUnit->Min;
	p->tm_sec=pTimeUnit->Sec;
	tv.tv_sec=timelocal(p);
	tv.tv_usec=(pTimeUnit->MS)*1000;//微秒



	LONG nIDNo;
	SetIDNo(ID_IDLDEVICE,pChannelUnit->m_ChannelNo,pDeviceUnit->m_DeviceNo,pSoeUnit->m_nYxIndex,&nIDNo);
	pYXUnit=pYXTable->GetYXNode(pYXTable,nIDNo);
	if (!pYXUnit) 
	{
		free(pYXSoeUnit);
		return FALSE;
	}
	memcpy(&pYXSoeUnit->m_SoeTime,&tv,sizeof(struct timeval));
	pYXSoeUnit->ToValue=pSoeUnit->m_bYxState?1:0;
	pYXSoeUnit->pYXUnit=pYXUnit;
	pYXSoeUnit->pNext=NULL;
	pScadaUnit->SendSOE(pScadaUnit,pYXSoeUnit);
	
	DEBUG_PRINT(pPubDev,"ExplainSoe: %d-%d-%d %d:%d:%d.%d;Index:%d,State:%d,Value:%.2f",
		pTimeUnit->Year,pTimeUnit->Mon,pTimeUnit->Mday,
		pTimeUnit->Hour,pTimeUnit->Min,pTimeUnit->Sec,pTimeUnit->MS,
		pSoeUnit->m_nYxIndex,pSoeUnit->m_bYxState,pSoeUnit->m_fValue);
	return TRUE;
}
static BOOL _YkSelect(struct _DeviceUnit * pDeviceUnit,WORD wOutPort,BOOL bOnOff)
{
	CPubDev* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	if (pPubDev->YkSelect)
		return pPubDev->YkSelect(pPubDev,wOutPort,bOnOff);
	return FALSE;
}
static BOOL _YkExecute(struct _DeviceUnit * pDeviceUnit,WORD wOutPort,BOOL bOnOff)
{
	CPubDev* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	if (pPubDev->YkExecute)
		return pPubDev->YkExecute(pPubDev,wOutPort,bOnOff);
	return FALSE;
}
static BOOL _YkCancel(struct _DeviceUnit * pDeviceUnit,WORD wOutPort,BOOL bOnOff)
{
	CPubDev* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	if (pPubDev->YkCancel)
		return pPubDev->YkCancel(pPubDev,wOutPort,bOnOff);
	return FALSE;
}
static BOOL _SetDeviceTime(struct _DeviceUnit * pDeviceUnit,BOOL bFF)
{
	CPubDev* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	if (pPubDev->SetDeviceTime)
		return pPubDev->SetDeviceTime(pPubDev);
	return FALSE;
}
static BOOL _ResetDeviceStatus(struct _DeviceUnit * pDeviceUnit)
{
	CPubDev* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	if (pPubDev->ResetDeviceStatus)
		return pPubDev->ResetDeviceStatus(pPubDev);
	return FALSE;
}
BOOL _SelectWriteFixValue(struct _DeviceUnit * pDeviceUnit,BYTE * pBuf,int bylen,BYTE byQh)
{
	CPubDev* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	WORD wStartIdx=g_DP.GetWordValue(pBuf,0,FALSE);
	WORD wNum=g_DP.GetWordValue(pBuf,1,FALSE);
	pPubDev->m_bCache_FixValue_Writing=FALSE;
	pPubDev->m_byCache_FixValue_QH=byQh;
	TRACE("ReadFixValue:StartIndex=%d,Num=%d",wStartIdx,wNum);
	if (pPubDev->ReadFixValue)
		return pPubDev->ReadFixValue(pPubDev,wStartIdx,wNum);
	return TRUE;
}
BOOL _ExecuteWriteFixValue(struct _DeviceUnit * pDeviceUnit,BYTE * pBuf,int bylen,BYTE byQh)
{
	CPubDev* pPubDev=GetPPubDevFromPDeviceUnit(pDeviceUnit);
	if (bylen<4)
	{
		TRACE("WriteFixValue:Error:bylen=%d",bylen);
		return FALSE;
	}
	WORD wStartIdx=g_DP.GetWordValue(pBuf,0,FALSE);
	WORD wNum=g_DP.GetWordValue(pBuf,1,FALSE);
	WORD wCalcNum=(bylen-4)/2;
	pPubDev->m_bCache_FixValue_Writing=TRUE;
	pPubDev->m_byCache_FixValue_QH=byQh;
	if (wNum!=wCalcNum)
	{
		TRACE("WriteFixValue: Warning:Num=%d,FixValueArrayLen=%d",wNum,bylen-4);
		wNum=MIN(wNum,wCalcNum);
	}
	TRACE("WriteFixValue:StartIndex=%d,Num=%d",wStartIdx,wNum);
	if (pPubDev->WriteFixValue)
		return pPubDev->WriteFixValue(pPubDev,wStartIdx,wNum,(WORD*)&pBuf[4]);
	return TRUE;
}


static inline void RegisterYkSelect(CPubDev *pPubDev,YKSELECT_CALLBACK pfnYkSelect)
{
	pPubDev->YkSelect=pfnYkSelect;
	pPubDev->pDeviceUnit->YkSelect=_YkSelect;
}
static inline void RegisterYkExecute(CPubDev *pPubDev,YKEXECUTE_CALLBACK pfnYkExecute)
{
	pPubDev->YkExecute=pfnYkExecute;
	pPubDev->pDeviceUnit->YkExecute=_YkExecute;
}
static inline void RegisterYkCancel(CPubDev *pPubDev,YKCANCEL_CALLBACK pfnYkCancel)
{
	pPubDev->YkCancel=pfnYkCancel;
	pPubDev->pDeviceUnit->YkCancel=_YkCancel;
}
static inline void RegisterYkOperations(struct _PubDev *pPubDev,YKOPERATION_CALLBACK pfnYkSelect,YKOPERATION_CALLBACK pfnYkExcute,YKOPERATION_CALLBACK pfnYkCancel)
{
	if (pfnYkSelect)
		pPubDev->RegisterYkSelect(pPubDev,pfnYkSelect);
	if (pfnYkExcute)
		pPubDev->RegisterYkExecute(pPubDev,pfnYkExcute);
	if (pfnYkCancel)
		pPubDev->RegisterYkCancel(pPubDev,pfnYkCancel);
}
static inline void RegisterSetDeviceTime(CPubDev *pPubDev,SETDEVICETIME_CALLBACK pfnSetDeviceTime)
{
	pPubDev->SetDeviceTime=pfnSetDeviceTime;
	pPubDev->pDeviceUnit->SetDeviceTime=_SetDeviceTime;
}
static inline void RegisterResetDevice(CPubDev *pPubDev,RESETDEVICE_CALLBACK pfnResetDevice)
{
	pPubDev->ResetDeviceStatus=pfnResetDevice;
	pPubDev->pDeviceUnit->ResetDeviceStatus=_ResetDeviceStatus;
}
static inline void RegisterReadFixValue(struct _PubDev *pPubDev,READFIXEDVALUE_CALLBACK pfnReadFixValue)
{
	pPubDev->ReadFixValue=pfnReadFixValue;
	pPubDev->pDeviceUnit->SelectWriteFixValue=_SelectWriteFixValue;
}
static inline void RegisterWriteFixValue(struct _PubDev *pPubDev,WRITEFIXEDVALUE_CALLBACK pfnWriteFixValue)
{
	pPubDev->WriteFixValue=pfnWriteFixValue;
	pPubDev->pDeviceUnit->ExecuteWriteFixValue=_ExecuteWriteFixValue;
}

static inline TIME_UNIT GetDateTime(struct _PubDev *pPubDev)
{
	TIME_UNIT DataTime;
	
	struct timeval tv={0};
	tv.tv_sec=time(NULL);
	struct tm* p=localtime(&tv.tv_sec);
	
	DataTime.Year=p->tm_year+1900;
	DataTime.Mon=p->tm_mon+1;
	DataTime.Mday=p->tm_mday;
	DataTime.Wday=p->tm_wday;
	DataTime.Yday=p->tm_yday;
	DataTime.Hour=p->tm_hour;
	DataTime.Min=p->tm_min;
	DataTime.Sec=p->tm_sec;
	DataTime.MS=tv.tv_usec/1000%1000;
	
	return DataTime;
}
static inline void InnerChannelSendData(struct _PubDev *pPubDev,BYTE *pData,int nSize,BYTE byCID)
{
	BYTE *pSend=malloc(nSize+3);
	if (pSend==NULL) 
	{
		TRACE("InnerChannelSendData: malloc Failed");
		return;
	}
	pSend[0]=pScadaUnit->m_BaseChannelNo;
	pSend[1]=pPubDev->pChannelUnit->m_ChannelNo;
	pSend[2]=pPubDev->pDeviceUnit->m_DeviceNo;
	memcpy(pSend+3,pData,nSize);
	InnerChannelMailData(pGlobal,byCID,0xff,pSend,3+nSize);
	free(pSend);
}
static inline void SetKeepSend(struct _PubDev *pPubDev,int iKeepSec)
{pPubDev->pChannelUnit->SetKeepOne(pPubDev->pChannelUnit,TRUE,iKeepSec);}
static inline void ReleaseKeepSend(struct _PubDev *pPubDev)
{pPubDev->pChannelUnit->SetKeepOne(pPubDev->pChannelUnit,FALSE,0);}
static inline void Set_LinkAddr(struct _PubDev *pPubDev,BYTE byAddr)
{pPubDev->pDeviceUnit->m_LinkAddr=byAddr;}
int Get_DeviceNo(struct _PubDev *pPubDev)
{return pPubDev->pDeviceUnit->m_DeviceNo;};
BYTE Get_ChannelNo(struct _PubDev *pPubDev)
{return pPubDev->pChannelUnit->m_ChannelNo;}
static inline BYTE Get_LinkAddr(struct _PubDev *pPubDev)
{return pPubDev->pDeviceUnit->m_LinkAddr;}
static inline const char*Get_LinkAddr2(struct _PubDev *pPubDev)
{return pPubDev->pDeviceUnit->m_LinkAddr2;}
static inline const char*Get_Param1(struct _PubDev *pPubDev)
{return pPubDev->pDeviceUnit->m_Param1;}
static inline const char*Get_Param2(struct _PubDev *pPubDev)
{return pPubDev->pDeviceUnit->m_Param2;}
static inline BYTE Get_RecvCID(struct _PubDev *pPubDev)
{return pPubDev->m_byRecvCID;}
static inline BYTE Get_RecvADDR(struct _PubDev *pPubDev)
{return pPubDev->m_byRecvADDR;}


static BOOL ExplainLinkData(struct _DeviceUnit * pDeviceUnit,BYTE *pBuf,WORD bySize)
{
//	printf("ExplainLinkData");
	//只要帧校验通过就需要返回TRUE
	if(!pDeviceUnit
		||!pDeviceUnit->pExDeviceUnit
		||!pDeviceUnit->pChannel)
		return FALSE;
	
	CPubDev * pPubDev=(CPubDev*)pDeviceUnit->pExDeviceUnit;
	ChannelUnit * pChannelUnit=pDeviceUnit->pChannel;		
	CFrameUnit *m_pRFrame=pDeviceUnit->pRFrameUnit;
	BYTE mCID;
	BYTE mAddr;
	BYTE * pData;
	int datalen;
	if(!(pPubDev->m_bRawMode)&&m_pRFrame&&m_pRFrame->BufferToFrame&&m_pRFrame->BufferToFrame(m_pRFrame,pBuf,bySize))
	{
		pPubDev->m_byRecvFrameType=m_pRFrame->m_FrameType;
		mCID=pPubDev->m_byRecvCID=m_pRFrame->m_CID;
		mAddr=pPubDev->m_byRecvADDR=m_pRFrame->m_Addr;
		pData=m_pRFrame->m_pData;
		datalen=m_pRFrame->m_wLength;
		if (mAddr!=pPubDev->Get_LinkAddr(pPubDev))
		{
			DEBUG_PRINT(pPubDev,"ExplainLink Addr Error!");
			return FALSE;
		}
	}
	else if (pPubDev->m_bRawMode)
	{
		pData=pBuf;
		datalen=bySize;
	}
	else
		return FALSE;
	
	BOOL bRet=TRUE;
	RECEIVE_CALLBACK pfnRecvCallback=pPubDev->DefaultRecvCallback;
	
	if (pDeviceUnit->m_EchoCID==eCid_InsertSendFrame)//插帧
	{
		//TRACE("插帧接收");
		SENDFRAME_LIST_NODE *pTmp=pPubDev->m_pInsertSendFrame_Head;
		if (pTmp)
		{
			if (pTmp->m_PollSendFrameInfo.m_pfnReceiveCallback!=NULL)
				pfnRecvCallback=pTmp->m_PollSendFrameInfo.m_pfnReceiveCallback;

			pPubDev->m_pInsertSendFrame_Head=pTmp->m_pNext;
			if (pPubDev->m_pInsertSendFrame_Head==NULL)
				pPubDev->m_pInsertSendFrame_Tail=NULL;
			free(pTmp->m_PollSendFrameInfo.m_pData);
			free(pTmp);
		}
	}
	else if (pDeviceUnit->m_EchoCID==eCid_PollSendFrame)//轮训
	{
		SENDFRAME_INFO *pSendFrameInfo=&pPubDev->m_NowPollingInfo;
		if (pSendFrameInfo && pSendFrameInfo->m_pfnReceiveCallback!=NULL)
			pfnRecvCallback=pSendFrameInfo->m_pfnReceiveCallback;
	}

	if (pfnRecvCallback)
		bRet = pfnRecvCallback(pPubDev,pData,datalen);
	else bRet=TRUE;

	if (pPubDev->m_pIntervalSend_Now)
	{
		if (pPubDev->m_pIntervalSend_Now->m_IntervalSendInfo.m_bOnceOnly)
			pPubDev->DestroyIntervalSend(pPubDev,pPubDev->m_pIntervalSend_Now);
		pPubDev->m_pIntervalSend_Now=NULL;
	}
	
	return bRet;
}

static BOOL RunPolling(struct _DeviceUnit * pDeviceUnit)
{
	if(!pDeviceUnit||!pDeviceUnit->pExDeviceUnit||!pDeviceUnit->pChannel)
		return FALSE;
	
	CPubDev * pPubDev=(CPubDev*)pDeviceUnit->pExDeviceUnit;	
	//////////////////////////////////////////////////////////////////////////
	//Delay Explain Yx
	if (pPubDev->m_pYxDelayExplain_Head)
	{
		while (pPubDev->m_pYxDelayExplain_Head && 
			pPubDev->m_pYxDelayExplain_Head->tExplainTime<time(NULL))
		{
			YX_DELAY_EXPLAIN_NODE *pNode=pPubDev->m_pYxDelayExplain_Head;
			
			TRACE("Explain YX(Delay):Index=%d,State=%d",pNode->wYxindex,pNode->bExplainState);
			pPubDev->ExplainYx(pPubDev,pNode->wYxindex,pNode->bExplainState);
			if (pPubDev->DeleteYxDelayExplainNode(pPubDev,pNode)==FALSE)
				free(pNode);//手动释放
		}
	}
	//////////////////////////////////////////////////////////////////////////
	pPubDev->m_bInPollingSend=TRUE;
	if (pPubDev->IntervalSend_Polling && pPubDev->IntervalSend_Polling(pPubDev))
	{
		goto __Polling_End;
	}

	if (pPubDev->PrePolling!=NULL && pPubDev->PrePolling(pPubDev))
	{
		goto __Polling_End;
	}
	
	if (pPubDev->m_pPollSendFrame_Now==NULL)
	{
		if (pPubDev->m_pPollSendFrame_Head)
			pPubDev->m_pPollSendFrame_Now=pPubDev->m_pPollSendFrame_Head;
		else goto __Error_End;
	}
	else
	{
		if (pPubDev->m_pPollSendFrame_Now->m_pNext==NULL)
		{
			TRACE("ERROR:POLL链表已断开");
			goto __Error_End;
		}
		pPubDev->m_pPollSendFrame_Now=pPubDev->m_pPollSendFrame_Now->m_pNext;
	}
	
	SENDFRAME_INFO *pPollInfo=&(pPubDev->m_pPollSendFrame_Now->m_PollSendFrameInfo);
	pPubDev->PollSendFrame(pPubDev,pPollInfo->m_byFrameType,pPollInfo->m_byCmd,pPollInfo->m_pData,pPollInfo->m_cbDataSize,pPollInfo->m_pfnReceiveCallback);
	
__Polling_End:
	pPubDev->m_bInPollingSend=FALSE;
	return TRUE;

__Error_End:
	pPubDev->m_bInPollingSend=FALSE;
	return FALSE;
}

static void InitPubDevDev(CPubDev	* pPubDev)
{
	if(pPubDev)
	{
		memset(pPubDev,0,sizeof(CPubDev));
		pPubDev->OpenRawMode=OpenRawMode;
		pPubDev->SetFrameModule=SetFrameModule;

		pPubDev->RegisterYkSelect=RegisterYkSelect;
		pPubDev->RegisterYkExecute=RegisterYkExecute;
		pPubDev->RegisterYkCancel=RegisterYkCancel;
		pPubDev->RegisterYkOperations=RegisterYkOperations;
		pPubDev->RegisterSetDeviceTime=RegisterSetDeviceTime;
		pPubDev->RegisterResetDevice=RegisterResetDevice;
		pPubDev->RegisterReadFixValue=RegisterReadFixValue;
		pPubDev->RegisterWriteFixValue=RegisterWriteFixValue;

		pPubDev->RegisterPrePolling=RegisterPrePolling;
		pPubDev->RegisterOnDestroy=RegisterOnDestroy;
		pPubDev->RegisterDefaultRecvCallback=RegisterDefaultRecvCallback;
		pPubDev->RegisterPollSendFrame=RegisterPollSendFrame;
		pPubDev->RegisterPollSendFrame_ModbusAsk=RegisterPollSendFrame_ModbusAsk;
		pPubDev->DestroyPollSendFrame=DestroyPollSendFrame;

		pPubDev->RegisterIntervalSend=RegisterIntervalSend;
		pPubDev->DestroyIntervalSend=DestroyIntervalSend;

		pPubDev->PollSendFrame=PollSendFrame;
		pPubDev->InsertSendFrame=InsertSendFrame;
		pPubDev->SendFrame=SendFrame;
		pPubDev->SendFrame_ModbusAsk=SendFrame_ModbusAsk;
		
		pPubDev->RespondResult_YK=RespondResult_YK;
		pPubDev->RespondResult_FixValue=RespondResult_FixValue;
		pPubDev->ExplainYc=ExplainYc;
		pPubDev->ExplainYx=ExplainYx;
		pPubDev->ExplainYxByte=ExplainYxByte;
		pPubDev->InitSoeUnit=InitSoeUnit;
		pPubDev->ExplainSoe=ExplainSoe;
		pPubDev->InsertYxDelayExplainNode=InsertYxDelayExplainNode;
		pPubDev->DeleteYxDelayExplainNode=DeleteYxDelayExplainNode;
		pPubDev->FindYxDelayExplainNode=FindYxDelayExplainNode;
		pPubDev->DelayExplainYx=DelayExplainYx;

		pPubDev->GetYcValue=GetYcValue;
		pPubDev->GetYxValue=GetYxValue;
		pPubDev->InnerChannelSendData=InnerChannelSendData;
		pPubDev->SetKeepSend=SetKeepSend;
		pPubDev->ReleaseKeepSend=ReleaseKeepSend;
		pPubDev->Set_LinkAddr=Set_LinkAddr;
		
		pPubDev->GetDateTime=GetDateTime;
		pPubDev->Get_DeviceNo=Get_DeviceNo;
		pPubDev->Get_ChannelNo=Get_ChannelNo;
		pPubDev->Get_LinkAddr=Get_LinkAddr;
		pPubDev->Get_LinkAddr2=Get_LinkAddr2;
		pPubDev->Get_Param1=Get_Param1;
		pPubDev->Get_Param2=Get_Param2;

		pPubDev->Get_RecvCID=Get_RecvCID;
		pPubDev->Get_RecvADDR=Get_RecvADDR;

		pPubDev->m_NowPollingInfo.m_pData=malloc(sizeof(BYTE)*256);
	}
}


void InitExDevice(void* pbyDeviceUnit,void *pbyIoGlobal)
{
	pGlobal=(IoGlobal*)pbyIoGlobal;
	if(!pGlobal||!pGlobal->pScadaUnit)
		return;
	pScadaUnit=pGlobal->pScadaUnit;
	
	DeviceUnit * pDeviceUnit=(DeviceUnit*)pbyDeviceUnit;
	if(pDeviceUnit)
	{

		CPubDev	* pPubDev=(CPubDev*)malloc(sizeof(CPubDev));
		InitPubDevDev(pPubDev);

		pPubDev->pDeviceUnit=(DeviceUnit*)pDeviceUnit;
		pPubDev->pChannelUnit=(ChannelUnit*)pDeviceUnit->pChannel;

		InitCommunication(pPubDev);
		pPubDev->m_bInitComplete=TRUE;

		pDeviceUnit->pExDeviceUnit=pPubDev;
		pDeviceUnit->RunPolling=RunPolling;
		pDeviceUnit->ExplainLinkData=ExplainLinkData;		
	}
}


