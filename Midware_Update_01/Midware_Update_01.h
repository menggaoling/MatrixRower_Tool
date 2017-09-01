#ifndef __MIDWARE_UPDATA_01_H__
#define __MIDWARE_UPDATA_01_H__

#include "JIStypes.h"

typedef enum
{
    CmdNone = 0,			
    CmdEraseFlash           = 0x10,
    CmdReturnEraseFlash,
    CmdWriteFlash,
    CmdReturnWriteFlash,
    CmdReadFlash,
    CmdReturnReadFlash,
    CmdReadProgramState     = 0x20,
    CmdReturnProgramState,
    CmdFlashUnlock          = 0x24,
    CmdReturnFlashState,
    CmdWriteCheckCode,
    CmdReturnCheckCode,
    CmdReadUpdateMode       = 0x40,
    CmdReturnUpdateMode,
}UPDATE_CMD;


void Midware_Update_Process(UCHAR RxCmd, UCHAR *Rxdata, UINT16 RxDataLen, UCHAR *TxData, UINT16 *TxDataLen);
void Midware_Update_Init_Data(void);
void Midware_Update_1ms_Int(void);

#endif