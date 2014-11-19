
#if !defined(AFX_MODBUSDEV_H__8EB79FAE_265A_4D4F_8AD8_8D533FABA874__INCLUDED_)
#define AFX_MODBUSDEV_H__8EB79FAE_265A_4D4F_8AD8_8D533FABA874__INCLUDED_

#include "types.h"
#include "main.h"
#include "DataProcess.h"



// enum 
// {
// 	e_ct1=0,
// 	e_ct2=1,
// 	cid_read_cdt1_data=0x82,
// 	cid_read_cdt2_data=0x83,
// 	cid_write_cdt1_data=0x84,
// 	cid_write_cdt2_data=0x85
// };
// 
// enum
// {
// 	inner_readct1_data_cmd=200,
// 	inner_readct2_data_cmd=201,
// 	inner_writect1_data_cmd=202,
// 	inner_writect2_data_cmd=202,
// };

enum
{
	eCid_PollSendFrame=0xf0,
	eCid_InsertSendFrame=0xff
};

//////////////////////////////////////////////////////////////////////////
struct _PubDev ;
typedef struct SENDFRAME_INFO_TAG
{
	BYTE m_byFrameType;
	BYTE m_byCmd;
	BYTE *m_pData;
	WORD m_cbDataSize;
	BOOL (*m_pfnReceiveCallback)(struct _PubDev *pPubDev,BYTE *pSend,int nSize) ;
}SENDFRAME_INFO;

typedef struct SENDFRAME_LIST_TAG
{
	int m_nID;
	struct SENDFRAME_LIST_TAG *m_pNext;
	struct SENDFRAME_INFO_TAG m_PollSendFrameInfo;
}SENDFRAME_LIST,SENDFRAME_LIST_NODE;

struct INTERVALSEND_NODE_TAG;
typedef struct INTERVALSEND_INFO_TAG
{
	time_t m_tSendTime;
	WORD m_wIntervalSec;
	BOOL m_bAutoSetTime;
	BOOL m_bOnceOnly;
	BOOL m_bWillDestroy;
	BOOL (*m_pfnIntervalSendCallback)(struct _PubDev *pPubDev,struct INTERVALSEND_NODE_TAG *pIntervalSendListNode);
}INTERVALSEND_INFO;

typedef struct INTERVALSEND_NODE_TAG
{
	int m_nID;
	struct INTERVALSEND_NODE_TAG *m_pNext;
	struct INTERVALSEND_INFO_TAG m_IntervalSendInfo;
	void (*SetAutoSetTime)(struct INTERVALSEND_NODE_TAG *pIntervalSendListNode,BOOL bAutoSetTime);
	void (*SetTime)(struct INTERVALSEND_NODE_TAG *pIntervalSendListNode);
	void (*ResetTime)(struct INTERVALSEND_NODE_TAG *pIntervalSendListNode);
	void (*SetInterval)(struct INTERVALSEND_NODE_TAG *pIntervalSendListNode,WORD wIntervalSec);
	void (*Destroy)(struct INTERVALSEND_NODE_TAG *pIntervalSendListNode);
}INTERVALSEND_LIST_NODE,INTERVALSEND_LIST;
typedef struct YX_DELAY_EXPLAIN_NODE_TAG
{
	WORD wYxindex;
	BYTE bExplainState;
	time_t tExplainTime;
	struct YX_DELAY_EXPLAIN_NODE_TAG *pNext;
}YX_DELAY_EXPLAIN_NODE;
//////////////////////////////////////////////////////////////////////////
typedef struct _PubDev  
{
	DeviceUnit * pDeviceUnit;
	ChannelUnit *pChannelUnit;
	
	void (*OpenRawMode)(struct _PubDev *pPubDev);
	void (*SetFrameModule)(struct _PubDev *pPubDev,const char szFrameModbulName[256]);

	void (*RegisterYkSelect)(struct _PubDev *pPubDev,YKSELECT_CALLBACK pfnYkSelect);
	void (*RegisterYkExecute)(struct _PubDev *pPubDev,YKEXECUTE_CALLBACK pfnYkExcute);
	void (*RegisterYkCancel)(struct _PubDev *pPubDev,YKCANCEL_CALLBACK pfnYkCancel);
	void (*RegisterYkOperations)(struct _PubDev *pPubDev,YKOPERATION_CALLBACK pfnYkSelect,YKOPERATION_CALLBACK pfnYkExcute,YKOPERATION_CALLBACK pfnYkCancel);
	void (*RegisterSetDeviceTime)(struct _PubDev *pPubDev,SETDEVICETIME_CALLBACK pfnSetDeviceTime);
	void (*RegisterResetDevice)(struct _PubDev *pPubDev,RESETDEVICE_CALLBACK pfnResetDevice);
	void (*RegisterReadFixValue)(struct _PubDev *pPubDev,READFIXEDVALUE_CALLBACK pfnReadFixValue);
	void (*RegisterWriteFixValue)(struct _PubDev *pPubDev,WRITEFIXEDVALUE_CALLBACK pfnWriteFixValue);
	
	void (*RegisterPrePolling)(struct _PubDev *pPubDev,PREPOLLING_CALLBACK pfnPolling);
	void (*RegisterDefaultRecvCallback)(struct _PubDev *pPubDev,RECEIVE_CALLBACK pfnRecvCallBack);
	SENDFRAME_LIST_NODE* (*RegisterPollSendFrame)(struct _PubDev *pPubDev,BYTE byFrameType,BYTE byCmd, BYTE *pSend,WORD wSize,RECEIVE_CALLBACK pfnRecvCallBack);
	SENDFRAME_LIST_NODE* (*RegisterPollSendFrame_ModbusAsk)(struct _PubDev *pPubDev,BYTE byCmd, WORD wRegAddr,WORD wRegNum,RECEIVE_CALLBACK pfnRecvCallBack);
	BOOL (*DestroyPollSendFrame)(struct _PubDev *pPubDev,SENDFRAME_LIST *pPollSendFrameNode);
	void (*RegisterOnDestroy)(struct _PubDev *pPubDev,DESTROY_CALLBACK pfnOnDestroy);

	INTERVALSEND_LIST_NODE* (*RegisterIntervalSend)(struct _PubDev *pPubDev,BOOL bOnceOnly,WORD wIntervalSec,INTERVALSEND_CALLBACK pfnIntervalSendCallBack);
	BOOL (*DestroyIntervalSend)(struct _PubDev *pPubDev,INTERVALSEND_LIST_NODE *pIntervalSendListNode);
	BOOL (*IntervalSend_Polling)(struct _PubDev *pPubDev);
	BOOL (*IntervalSend_Init)(struct _PubDev *pPubDev,INTERVALSEND_LIST_NODE *pIntervalSendListNode);
	
	BOOL (*PollSendFrame)(struct _PubDev *pPubDev,BYTE byFrameType,BYTE byCmd, BYTE *pSend,WORD wSize,RECEIVE_CALLBACK pfnRecvCallBack);
	BOOL (*InsertSendFrame)(struct _PubDev *pPubDev,BYTE byFrameType,BYTE byCmd, BYTE *pSend,WORD wSize,RECEIVE_CALLBACK pfnRecvCallBack);
	BOOL (*SendFrame)(struct _PubDev *pPubDev,BYTE byFrameType,BYTE byCmd, BYTE *pSend,WORD wSize,RECEIVE_CALLBACK pfnRecvCallBack);
	BOOL (*SendFrame_ModbusAsk)(struct _PubDev *pPubDev,BYTE byCmd, WORD wRegAddr,WORD wRegNum,RECEIVE_CALLBACK pfnRecvCallBack);

	void (*RespondResult_YK)(struct _PubDev *pPubDev,enum yk_Kind YkKind,BOOL bSuccess);
	void (*RespondResult_FixValue)(struct _PubDev * pPubDev,WORD wStartIdx,WORD wNum,WORD awFixValue[]);
	void (*ExplainYc)(struct _PubDev *pPubDev,WORD wYcindex,float fValue);
	void (*ExplainYx)(struct _PubDev *pPubDev,WORD wYxindex,BOOL bOnoff);
	void (*ExplainYxByte)(struct _PubDev *pPubDev,WORD wYxindex,BYTE byte);
	BOOL (*InitSoeUnit)(struct _PubDev *pPubDev,SOE_UNIT *pSoeUnit);
	BOOL (*ExplainSoe)(struct _PubDev *pPubDev,SOE_UNIT *pSoeUnit);
	void (*InsertYxDelayExplainNode)(struct _PubDev *pPubDev,YX_DELAY_EXPLAIN_NODE InsertionNode);
	BOOL (*DeleteYxDelayExplainNode)(struct _PubDev *pPubDev,YX_DELAY_EXPLAIN_NODE* pInsertion);
	YX_DELAY_EXPLAIN_NODE* (*FindYxDelayExplainNode)(struct _PubDev *pPubDev,WORD wYxindex);
	void (*DelayExplainYx)(struct _PubDev *pPubDev,WORD wYxindex,BOOL bOnoff,WORD nDelay);
	//////////////////////////////////////////////////////////////////////////
	BOOL (*GetYcValue)(struct _PubDev *pPubDev,WORD wYcindex,float *pfReturn);
	BOOL (*GetYxValue)(struct _PubDev *pPubDev,WORD wYxindex,BOOL *pfReturn);
	void (*InnerChannelSendData)(struct _PubDev *pPubDev,BYTE *pData,int nSize,BYTE byCID);
	void (*SetKeepSend)(struct _PubDev *pPubDev,int iKeepSec);
	void (*ReleaseKeepSend)(struct _PubDev *pPubDev);
	void (*Set_LinkAddr)(struct _PubDev *pPubDev,BYTE byAddr);
	
	TIME_UNIT (*GetDateTime)(struct _PubDev *pPubDev);
	BYTE (*Get_ChannelNo)(struct _PubDev *pPubDev);
	int (*Get_DeviceNo)(struct _PubDev *pPubDev);
	BYTE (*Get_LinkAddr)(struct _PubDev *pPubDev);
	const char* (*Get_LinkAddr2)(struct _PubDev *pPubDev);
	const char* (*Get_Param1)(struct _PubDev *pPubDev);
	const char* (*Get_Param2)(struct _PubDev *pPubDev);

	BYTE (*Get_RecvCID)(struct _PubDev *pPubDev);
	BYTE (*Get_RecvADDR)(struct _PubDev *pPubDev);
	
	BYTE m_byRecvCID;
	BYTE m_byRecvADDR;
	BOOL m_bInPollingSend;
	BOOL m_bRawMode;
	BOOL m_bInitComplete;
	BYTE m_byRecvFrameType;
	BOOL m_bCache_FixValue_Writing;
	BYTE m_byCache_FixValue_QH;
	
	//////////////////////////////////////////////////////////////////////////
	//User-Defined Fuction
	BOOL (*PrePolling)(struct _PubDev *pPubDev);
	BOOL (*DefaultRecvCallback)(struct _PubDev *pPubDev,BYTE *pRecv,int nSize);
	BOOL (*YkSelect)(struct _PubDev * pPubDev,WORD wOutPort,BOOL bOnOff);//bOnOff TRUE为分闸，FALSE为合闸
	BOOL (*YkExecute)(struct _PubDev * pPubDev,WORD wOutPort,BOOL bOnOff);//bOnOff TRUE为分闸，FALSE为合闸
	BOOL (*YkCancel)(struct _PubDev * pPubDev,WORD byOut,BOOL bOnOff);//bOnOff TRUE为分闸，FALSE为合闸
	BOOL (*ReadFixValue)(struct _PubDev * pPubDev,WORD wStartIdx,WORD wNum);
	BOOL (*WriteFixValue)(struct _PubDev * pPubDev,WORD wStartIdx,WORD wNum,WORD awFixValue[]);
	BOOL (*SetDeviceTime)(struct _PubDev* pPubDev);
	BOOL (*ResetDeviceStatus)(struct _PubDev * pPubDev);
	void (*OnDestroy)(struct _PubDev *pPubDev);
	//////////////////////////////////////////////////////////////////////////
	
	DWORD m_nRegisteredPollCount;
	SENDFRAME_INFO m_NowPollingInfo;
	SENDFRAME_LIST *m_pPollSendFrame_Now;
	SENDFRAME_LIST *m_pPollSendFrame_Head;
	SENDFRAME_LIST *m_pPollSendFrame_Tail;
	SENDFRAME_LIST *m_pInsertSendFrame_Head;
	SENDFRAME_LIST *m_pInsertSendFrame_Tail;
	YX_DELAY_EXPLAIN_NODE  *m_pYxDelayExplain_Head;
	YX_DELAY_EXPLAIN_NODE  *m_pYxDelayExplain_Tail;

	DWORD m_nRegisteredIntervalSendCount;
	INTERVALSEND_LIST_NODE *m_pIntervalSend_Now;
	INTERVALSEND_LIST *m_pIntervalSend_Head;
	INTERVALSEND_LIST *m_pIntervalSend_Tail;


	DEVICE_VARIABLES Vars;
}CPubDev;


#endif // !defined(AFX_MODBUSDEV_H__8EB79FAE_265A_4D4F_8AD8_8D533FABA874__INCLUDED_)
