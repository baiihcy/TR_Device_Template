#include "types.h"
#pragma once
typedef struct DATA_PROCESS_TAG
{
	BYTE (*GetLoByte)(WORD w);
	BYTE (*GetHiByte)(WORD w);
	WORD (*GetLoWord)(DWORD dw);
	WORD (*GetHiWord)(DWORD dw);
	WORD (*MakeWord)(BYTE HiByte,BYTE LoByte);
	DWORD (*MakeDWord)(WORD HiWord,WORD LoWord);
	WORD (*SwapByte)(WORD w);
	DWORD (*SwapWord)(DWORD dw);
	DWORD (*GetBit)(DWORD dw,BYTE byBitIndex);
	LONG (*GetINT)(DWORD dw,BYTE BitOffset,BYTE BitNum);
	DWORD (*GetUINT)(DWORD dw,BYTE BitOffset,BYTE BitNum);
	float (*DWordToFloat)(DWORD dw);
	DWORD (*FloatToDWord)(float f);
	BYTE (*EncodeBCD)(BYTE Decimal);
	BYTE (*DecodeBCD)(BYTE BCD);
	BYTE (*Check_SUM8)(BYTE *pData,WORD nSize);
	WORD (*Check_SUM16)(BYTE *pData,WORD nSize);
	WORD (*Check_CRC16)(BYTE *pData,WORD nSize);
	WORD (*GetWordValueByByte)(BYTE *pData,WORD wByteIndex,BOOL bSwapByte);
	WORD (*GetWordValue)(BYTE *pData,WORD wWordIndex,BOOL bSwapByte);
	WORD (*SetWordValueByByte)(BYTE *pData,WORD wByteIndex,WORD wValue,BOOL bSwapByte);
	WORD (*SetWordValue)(BYTE *pData,WORD wWordIndex,WORD wValue,BOOL bSwapByte);
	DWORD (*GetDWordValueByByte)(BYTE *pData,WORD wByteIndex,BOOL bSwapWord,BOOL bSwapByte);
	DWORD (*GetDWordValue)(BYTE *pData,WORD wDWordIndex,BOOL bSwapWord,BOOL bSwapByte);
	DWORD (*SetDWordValueByByte)(BYTE *pData,WORD wByteIndex,DWORD dwValue,BOOL bSwapWord,BOOL bSwapByte);
	DWORD (*SetDWordValue)(BYTE *pData,WORD wDWordInde,DWORD dwValue,BOOL bSwapWord,BOOL bSwapByte);
	void (*time_t2TimeUnit)(time_t tTime,struct TIME_UNIT_TAG *pTimeUnit);
	time_t (*TimeUnit2time_t)(struct TIME_UNIT_TAG *pTimeUnit);
}DATA_PROCESS;

#define GetRegisterValue GetWordValue
#define GetRegisterValueByByte GetWordValueByByte
#define SetRegisterValue SetWordValue
#define SetRegisterValueByByte SetWordValueByByte

extern DATA_PROCESS g_DP;//全局 数据处理对象，初始化代码在DataProcess.c
