#pragma once

#include "../../../txj/libio.h"
#include "../../../txj/appmc.h"

#ifndef DEBUG_PRINT
#define DEBUG_PRINT(_pPubDev,_format,...)\
	{\
	if(((ChannelUnit*)((_pPubDev)->pDeviceUnit->pChannel))->m_ChannelNo==pGlobal->pScadaUnit->DebugChannelNo)\
		{\
		printf("\nDEBUG:Ch-%d,Dev-%d|",((ChannelUnit*)((_pPubDev)->pDeviceUnit->pChannel))->m_ChannelNo,(_pPubDev)->pDeviceUnit->m_DeviceNo);\
		printf(_format,##__VA_ARGS__);\
		}\
	}
#endif //DEBUG_PRINT
#define TRACE(_format,...) DEBUG_PRINT(pPubDev,_format,##__VA_ARGS__)

//////////////////////////////////////////////////////////////////////////
//��Լģ�飬����SetFrameModule
#define FRAMEMODULE_NULL NULL
#define FRAMEMODULE_CDT "cdtframe.so"
#define FRAMEMODULE_MODBUS "modbus.so"
#define FRAMEMODULE_103	"iec103.so"
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//֡���ͣ�����SendFrame/InsertSendFrame/PollSendFrame/RegisterPollSendFrame
#define FRAMETYPE_NULL 0
#define FRAMETYPE_NORMAL 0
#define FRAMETYPE_CDT 0
#define FRAMETYPE_MODBUS 0
#define FRAMETYPE_103_LONGFRAME 0
#define FRAMETYPE_103_SHORTFRAME 1
//////////////////////////////////////////////////////////////////////////
typedef struct TIME_UNIT_TAG
{
    int Sec;     /* seconds after the minute - [0,59] */
    int Min;     /* minutes after the hour - [0,59] */
    int Hour;    /* hours since midnight - [0,23] */
    int Mday;    /* day of the month - [1,31] */
    int Mon;     /* months since January - [1,12] */
    int Year;    /* years */
    int Wday;    /* days since Sunday - [0,6] */
    int Yday;    /* days since January 1 - [0,365] */
	int MS;		 /* millisecond */
}TIME_UNIT;

typedef struct SOE_UNIT_TAG
{
	struct TIME_UNIT_TAG m_Time;
	LONG m_nYxIndex;//SOEң�ŵ��
	BOOL m_bYxState;//SOE����״̬
	float m_fValue;//����ֵ��������
}SOE_UNIT;
//////////////////////////////////////////////////////////////////////////
struct _PubDev ;
struct INTERVALSEND_NODE_TAG;
typedef BOOL (*PREPOLLING_CALLBACK)(struct _PubDev *pPubDev);
typedef void (*DESTROY_CALLBACK)(struct _PubDev *pPubDev);
typedef BOOL (*INTERVALSEND_CALLBACK)(struct _PubDev *pPubDev,struct INTERVALSEND_NODE_TAG *pSendNode);
typedef BOOL (*RECEIVE_CALLBACK)(struct _PubDev *pPubDev,BYTE *pData,int nSize);
typedef BOOL (*YKOPERATION_CALLBACK)(struct _PubDev *pPubDev,WORD wOutPort,BOOL bOnOff);//bOnOff TRUEΪ��բ��FALSEΪ��բ
typedef BOOL (*SETDEVICETIME_CALLBACK)(struct _PubDev* pPubDev);
typedef BOOL (*RESETDEVICE_CALLBACK)(struct _PubDev * pPubDev);
typedef BOOL (*READFIXEDVALUE_CALLBACK)(struct _PubDev * pPubDev,WORD wStartIdx,WORD wNum);
typedef BOOL (*WRITEFIXEDVALUE_CALLBACK)(struct _PubDev * pPubDev,WORD wStartIdx,WORD wNum,WORD awFixValue[]);

#define YKSELECT_CALLBACK YKOPERATION_CALLBACK 
#define YKEXECUTE_CALLBACK YKOPERATION_CALLBACK 
#define YKCANCEL_CALLBACK YKOPERATION_CALLBACK 
#define WRITEFIXEDVALUE_SELECT_CALLBACK WRITEFIXEDVALUE_CALLBACK 
#define WRITEFIXEDVALUE_EXECUTE_CALLBACK WRITEFIXEDVALUE_CALLBACK 
