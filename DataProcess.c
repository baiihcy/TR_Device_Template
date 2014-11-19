#include "DataProcess.h"

DWORD g_dwBitMask[]={0x00000000,
	0x00000001,0x00000003,0x00000007,0x0000000f,0x0000001f,0x0000003f,0x0000007f,0x000000ff,
	0x000001ff,0x000003ff,0x000007ff,0x00000fff,0x00001fff,0x00003fff,0x00007fff,0x0000ffff,
	0x0001ffff,0x0003ffff,0x0007ffff,0x000fffff,0x001fffff,0x003fffff,0x007fffff,0x00ffffff,
	0x01ffffff,0x03ffffff,0x07ffffff,0x0fffffff,0x1fffffff,0x3fffffff,0x7fffffff,0xffffffff};

static inline BYTE GetLoByte(WORD w)
{
	return w&0x00ff;
}

static inline BYTE GetHiByte(WORD w)
{
	return (w&0xff00)>>8;
}

static inline WORD GetLoWord(DWORD dw)
{
	return dw&0x0000ffff;
}

static inline WORD GetHiWord(DWORD dw)
{
	return (dw&0xffff0000)>>16;
}

static inline WORD MakeWord(BYTE HiByte,BYTE LoByte)
{
	return LoByte+(HiByte<<8);
}

static inline DWORD MakeDWord(WORD HiWord,WORD LoWord)
{
	return LoWord+(HiWord<<16);
}

static inline WORD SwapByte(WORD w)
{
	WORD wResult=0;
	wResult=((w&0x00ff)<<8) | ((w&0xff00)>>8);
	return wResult;
}

static inline DWORD SwapWord(DWORD dw)
{
	DWORD dwResult=0;
	dwResult=((dw&0x0000ffff)<<16) | ((dw&0xffff0000)>>16);
	return dwResult;
}

static inline DWORD GetBit(DWORD dw,BYTE byBitIndex)
{
	return dw&(1<<byBitIndex);
}

static inline LONG GetINT(DWORD dw,BYTE BitOffset,BYTE BitNum)
{
	LONG lResult;
	dw=(dw>>BitOffset)&g_dwBitMask[BitNum];
	//printf("GetINI:%d>>%d & %X",dw,BitOffset,g_dwBitMask[BitNum]);
	if (BitNum>0 && GetBit(dw,BitNum-1))
	{
		lResult=-((dw^g_dwBitMask[BitNum])+1);
	}
	else
	{
		lResult=dw;
	}
	return lResult;
}

static inline DWORD GetUINT(DWORD dw,BYTE BitOffset,BYTE BitNum)
{
	return (dw>>BitOffset)&g_dwBitMask[BitNum];
}

static inline float DWordToFloat(DWORD dw)
{
	float f;
	memcpy(&f,&dw,sizeof(float));
	return f;
}

static inline DWORD FloatToDWord(float f)
{
	DWORD dw;
	memcpy(&dw,&f,sizeof(float));
	return dw;
}

static inline BYTE EncodeBCD(BYTE Decimal)
{
	return 	((Decimal/10)<<4)+(Decimal%10);
}

static inline BYTE DecodeBCD(BYTE BCD)
{
	return (BCD>>4)*10+(BCD&0x0f);
}

static BYTE Check_SUM8(BYTE *pData,WORD nSize)
{
	BYTE sum=0;
	int i=0;
	for (i=0;i<nSize;i++)
	{
		sum+=pData[i];
	}
	return sum;
}

static WORD Check_SUM16(BYTE *pData,WORD nSize)
{
	WORD sum=0;
	int i=0;
	for (i=0;i<nSize;i++)
	{
		sum+=pData[i];
	}
	return sum;
}

static WORD Check_CRC16(BYTE *pData,WORD nSize)
{
	int i,j;
	WORD wCRC=0xffff;
	WORD wTmp;
	for (i=0;i<nSize;i++)
	{
		wCRC^=pData[i];
		for (j=0;j<8;j++)
		{
			wTmp=wCRC&0x0001;
			wCRC=wCRC>>1;
			if(wTmp)wCRC=(wCRC^0xA001);
		}
	}
	return SwapByte(ntohs(wCRC));
}

static inline WORD GetWordValueByByte(BYTE *pData,WORD wByteIndex,BOOL bSwapByte)
{
	WORD Result;
	memcpy(&Result,pData+wByteIndex,sizeof(WORD));
	Result=ntohs(Result);
	if (!bSwapByte)
		return SwapByte(Result);
	else
		return Result;
}

static inline WORD GetWordValue(BYTE *pData,WORD wRegIndex,BOOL bSwapByte)
{
	return GetWordValueByByte(pData,wRegIndex*2,bSwapByte);
}

static inline WORD SetWordValueByByte(BYTE *pData,WORD wByteIndex,WORD wValue,BOOL bSwapByte)
{
	if (!bSwapByte)
		wValue=SwapByte(wValue);
	wValue=htons(wValue);
	memcpy(pData+wByteIndex,&wValue,sizeof(WORD));
	return wValue;
}

static inline WORD SetWordValue(BYTE *pData,WORD wRegIndex,WORD wValue,BOOL bSwapByte)
{
	return SetWordValueByByte(pData,wRegIndex*2,wValue,bSwapByte);
}

static inline DWORD GetDWordValueByByte(BYTE *pData,WORD wByteIndex,BOOL bSwapWord,BOOL bSwapByte)
{
	WORD wValueL=GetWordValueByByte(pData,wByteIndex,bSwapByte);
	WORD wValueH=GetWordValueByByte(pData,wByteIndex+2,bSwapByte);
	DWORD dwResult=MakeDWord(wValueH,wValueL);
	if (bSwapWord)
		dwResult=SwapWord(dwResult);
	return dwResult;
}

static inline DWORD GetDWordValue(BYTE *pData,WORD wDWordIndex,BOOL bSwapWord,BOOL bSwapByte)
{
	return GetDWordValueByByte(pData,wDWordIndex*4,bSwapWord,bSwapByte);
}

static inline DWORD SetDWordValueByByte(BYTE *pData,WORD wByteIndex,DWORD dwValue,BOOL bSwapWord,BOOL bSwapByte)
{
	if (bSwapWord)
		dwValue=SwapWord(dwValue);
	SetWordValueByByte(pData,wByteIndex,GetLoWord(dwValue),bSwapByte);
	SetWordValueByByte(pData,wByteIndex+2,GetHiWord(dwValue),bSwapByte);
	return dwValue;
}

static inline DWORD SetDWordValue(BYTE *pData,WORD wDWordInde,DWORD dwValue,BOOL bSwapWord,BOOL bSwapByte)
{
	return SetDWordValueByByte(pData,wDWordInde*4,dwValue,bSwapByte,bSwapWord);
}

static inline void time_t2TimeUnit(time_t tTime,TIME_UNIT *pTimeUnit)
{
	if (pTimeUnit==NULL)
		return;
	
	struct tm* p=localtime(&tTime);
	
	pTimeUnit->Year=1900+p->tm_year;
	pTimeUnit->Mon=p->tm_mon+1;
	pTimeUnit->Mday=p->tm_mday;
	pTimeUnit->Wday=p->tm_wday;
	pTimeUnit->Yday=p->tm_yday;
	pTimeUnit->Hour=p->tm_hour;
	pTimeUnit->Min=p->tm_min;
	pTimeUnit->Sec=p->tm_sec;
	pTimeUnit->MS=0;
}

static inline time_t TimeUnit2time_t(TIME_UNIT *pTimeUnit)
{
	if (pTimeUnit==NULL) 
		return 0;
	time_t tTime=0;
	struct tm tmTime={0};
	tmTime.tm_year=pTimeUnit->Year-1900;
	tmTime.tm_mon=pTimeUnit->Mon-1;
	tmTime.tm_yday=pTimeUnit->Yday;
	tmTime.tm_mday=pTimeUnit->Mday;
	tmTime.tm_wday=pTimeUnit->Wday;
	tmTime.tm_hour=pTimeUnit->Hour;
	tmTime.tm_min=pTimeUnit->Min;
	tmTime.tm_sec=pTimeUnit->Sec;
	tTime=timelocal(&tmTime);
	return tTime;
}

DATA_PROCESS g_DP={GetLoByte,GetHiByte,GetLoWord,GetHiWord,MakeWord,MakeDWord,SwapByte,SwapWord,
				GetBit,GetINT,GetUINT,DWordToFloat,FloatToDWord,
				EncodeBCD,DecodeBCD,Check_SUM8,Check_SUM16,Check_CRC16,
				GetWordValueByByte,GetWordValue,SetWordValueByByte,SetWordValue,
				GetDWordValueByByte,GetDWordValue,SetDWordValueByByte,SetDWordValue,
				time_t2TimeUnit,TimeUnit2time_t};
