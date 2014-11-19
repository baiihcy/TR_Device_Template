#include "dev.h"
#include "main.h"

// BOOL PREPOLLING_CALLBACK(struct _PubDev *pPubDev)//Polling回调函数
// BOOL RECEIVE_CALLBACK(struct _PubDev *pPubDev,BYTE *pRecv,int nSize)//报文接收回调函数
// BOOL INTERVALSEND_CALLBACK(struct _PubDev *pPubDev,INTERVALSEND_LIST_NODE *pSendNode)//间隔发送回调报文
// BOOL YKOPERATION_CALLBACK(struct _PubDev *pPubDev,WORD wOutPort,BOOL bOnOff)//遥控操作回调函数 bOnOff TRUE为分闸，FALSE为合闸
// BOOL SETDEVICETIME_CALLBACK(struct _PubDev * pPubDev)//装置对时回调函数
// BOOL RESETDEVICE_CALLBACK(struct _PubDev * pPubDev)//装置复归回调函数
// g_DP 数据处理对象，直接使用即可

BOOL InitCommunication(struct _PubDev *pPubDev)
{
	pPubDev->SetFrameModule(pPubDev,FRAMEMODULE_MODBUS);//使用MODBUS规约模块
	return TRUE;
}
