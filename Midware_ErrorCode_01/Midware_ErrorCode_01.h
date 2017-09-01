#ifndef __MIDWARE_ERROR_01_H__
#define __MIDWARE_ERROR_01_H__

#include "Midware_Incline_01.h"
#include "Midware_Led_01.h" 
#include "Midware_Digital_01.h"
#include "Midware_Nvflash_01.h"
#ifdef __LCB_SYS__
#include "Midware_RPM_01.h"
#include "Midware_ECB_01.h"
#include "Midware_MachineType_01.h"
#include "Midware_Induction_01.h"
#else
#include "Midware_Motor_01.h"
#include "Midware_Safekey_01.h"
#include "Midware_Temp_01.h"
#endif

typedef enum
{
    //LCB
    NO_RPM = 0,
    IND_OPEN,
    INC_NO_CNT_LCB,
    TEMP_HIGH,
    TEMP_ERR,
    COMM_TIMEOUT,
    RX_TX_ERROR,
    ECB_NO_COUNT,
    ECB_ZERO_ERR,
    NVFLASH_W,
    NVFLASH_R,
    INC_ZERO_SHORT,
    IND_MOS_SHORT,
    TYPE_FAIL,
    //MCB
    MCB_TYPE_ERROR = 0,	
    SAFEKEY ,
    NO_INCODE,
    OVER_CURRENT,
    MOS_FAULT,
    SPEED_LOW,
    INC_NO_CNT_MCB,
    INC_ZEROSHORT,
    MCB_TEMP_OVER,
    MCB_TEMP_UNNORMAL,
    DIGTIAL_TIMEOUT,
    DIGTIAL_RX_ERROR,
    FLASH_W,
    FLASH_R,
    NUM_OF_ERROR,
} ERROR_BEHAVIOR;


//�豸״̬
typedef union
{
    UCHAR status;
    struct
    {
        unsigned inclineDown       : 1;     //1 = Incline Running Down
        unsigned inclineUp         : 1;	    //1 = Incline Running Up
        unsigned mainMotor         : 1;     //1 = Main Motor is Running
        unsigned ospRunning        : 1;	    //1 = OSP Detected Pulse
        unsigned bettryVotage      : 1;	    //1 = SafeKey out    0 = SafeKey in
        unsigned commandError      : 1;	    //1 = Received Error Command from UCB
        unsigned MCBerror          : 1;	    //1 = MCB Found Error(s)
        unsigned MCBinitial        : 1;	    //1 = MCB is Initializing
    };
}MCB_STATUS;

void Midware_ErrorCode_Initial_Data(void);
void Midware_ErrorCode_Process(void);
UINT16 Midware_ErrorCode_Get_ErrorCode(void);
void Midware_ErrorCode_Skip_Last(void);
UCHAR Midware_ErrorCode_Have_Error(void);
void Midware_ErrorCode_ResetSafekey(void);
UCHAR Midware_ErrorCode_GetStatus(ERROR_BEHAVIOR ErrorType);
#endif
