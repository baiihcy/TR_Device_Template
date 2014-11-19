#include "types.h"
#pragma once

struct _PubDev;
#define GetPPubDevFromPDeviceUnit(_pDeviceUnit) (struct _PubDev*)_pDeviceUnit->pExDeviceUnit
extern IoGlobal * pGlobal;
extern SCADAUnit*   pScadaUnit;
extern BOOL InitCommunication(struct _PubDev *);



typedef struct DEVICE_VARIABLES_TAG
{
}DEVICE_VARIABLES;
