#include "dev.h"
#include "main.h"

// BOOL PREPOLLING_CALLBACK(struct _PubDev *pPubDev)//Polling�ص�����
// BOOL RECEIVE_CALLBACK(struct _PubDev *pPubDev,BYTE *pRecv,int nSize)//���Ľ��ջص�����
// BOOL INTERVALSEND_CALLBACK(struct _PubDev *pPubDev,INTERVALSEND_LIST_NODE *pSendNode)//������ͻص�����
// BOOL YKOPERATION_CALLBACK(struct _PubDev *pPubDev,WORD wOutPort,BOOL bOnOff)//ң�ز����ص����� bOnOff TRUEΪ��բ��FALSEΪ��բ
// BOOL SETDEVICETIME_CALLBACK(struct _PubDev * pPubDev)//װ�ö�ʱ�ص�����
// BOOL RESETDEVICE_CALLBACK(struct _PubDev * pPubDev)//װ�ø���ص�����
// g_DP ���ݴ������ֱ��ʹ�ü���

BOOL InitCommunication(struct _PubDev *pPubDev)
{
	pPubDev->SetFrameModule(pPubDev,FRAMEMODULE_MODBUS);//ʹ��MODBUS��Լģ��
	return TRUE;
}
