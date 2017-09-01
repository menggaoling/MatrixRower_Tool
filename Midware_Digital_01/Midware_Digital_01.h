#ifndef __MIDWARE_DIGITAL_01_H__
#define __MIDWARE_DIGITAL_01_H__

#include "Hal_Digital_01.h"

#define DATA_LEN_MAX	           550

#define NORMAL_MODE	           0
#define UPDATE_MODE	           1

typedef enum
{
    CMD_NONE                        = 0x00,  
    CMD_GET_OSP                     = 0x10,
    CMD_SET_INTERVALS	            = 0x11,
    CMD_AUTO_CALIBRATE              = 0x30,     //
    CMD_START_CAL_CURRENT           = 0x44,     //
    CMD_CAL_CURRENT                 = 0x45,     //
    CMD_EXIT_CAL_CURRENT            = 0x46,     //
    CMD_SET_IND_BASE_CURRENT        = 0x47,
    CMD_SET_INDUCTION_PWM           = 0x48,
    CMD_FINISH_INDUCTION_ADJUST     = 0x49,
    CMD_GET_NEW_VERSION             = 0x50,     
    CMD_SET_ECB_ZERO                = 0x61,
    CMD_SET_ECB_LOCATION            = 0x62,
    CMD_GET_ECB_RPM                 = 0x63,
    CMD_GET_ECB_LOCATION            = 0x64,
    CMD_SET_INDUCTION_ADC           = 0x65,    
    CMD_INITIAL                     = 0x70,
    CMD_GET_STATUS                  = 0x71,
    CMD_GET_ERROR_CODE              = 0x72,
    CMD_GET_VERSION                 = 0x73,
    CMD_CALIBRATE                   = 0x74,
    CMD_UPDATE_PROGRAM              = 0x75,
    CMD_SKIP_CURRENT_ERROR          = 0x76,
    CMD_SPECIAL_EXT_COMMAND         = 0x77,
    CMD_GET_DCI_VERSION             = 0x78,
    CMD_SET_ERPMODE                 = 0x7A,     
    CMD_SET_ECB_ACTION              = 0x82,
    CMD_GET_ECB_STATUS              = 0x83,
    CMD_GET_ECB_COUNT               = 0x84,   
    CMD_SET_DRIVE_MOTOR_HP          = 0x90,
    CMD_GET_DRIVER_TYPE             = 0x91,
    CMD_GET_TM_IN_USED              = 0x95,
    CMD_GET_EC_TEMPERATURE          = 0x96,     
    CMD_SET_MOTOR_RPM_MAX           = 0xF0,
    CMD_SET_MOTOR_RPM               = 0xF1,
    CMD_SET_PWM_ADDSTEP             = 0xF2,
    CMD_SET_PWM_DECSTEP	            = 0xF3,
    CMD_SET_PWM_STOPSTEP            = 0xF4,
    CMD_SET_INCLINE_ACTION          = 0xF5,
    CMD_SET_INCLINE_LOCATION	    = 0xF6,
    CMD_SET_WORK_STATUS             = 0xF7,
    CMD_GET_ROLLER_RPM              = 0xF8,
    CMD_GET_MOTOR_RPM               = 0xF9,
    CMD_GET_INCLINE_LOCATION	    = 0xFA,
    CMD_TUNEEND_POINT_INCLINE	    = 0xFB,
    CMD_TUNEEND_POINT_INCLINE2      = 0xFC,
    COM_SET_INCLINE_STROKE          = 0xFD,
    CMD_SET_COMPENSATION_VOLTAGE    = 0xFE,
    CMD_TEST                        = 0xFF,
}COM_TYPE;


typedef struct
{
    unsigned Digital_Rx_Timeout     : 1;
    unsigned Digital_Tx_Timeout     : 1;
    unsigned Digital_CRC_Error      : 1;
    unsigned Digital_TimeOut        : 1;
}DIGITAL_ERROR;

enum {NORMAL_START = 0, NORMAL_STATUS, NORMAL_COMMD, NORMAL_LEN, NORMAL_DATA, NORMAL_CRC};
enum {UPDATE_START = 0, UPDATE_COMMD, UPDATE_DATA, UPDATE_END};

void Midware_Digital_Initial_HW(UCHAR mode);
void Midware_Digital_Initial_Data(void);
void Midware_Digital_1ms_Int(void);
void Midware_Digital_Send_Command(UCHAR *CommandTx, UINT16 Datalen);
UCHAR Midware_Digital_Process(UCHAR *commandRx);
UCHAR Midware_Digital_Get_Rx(void);
UCHAR Midware_Digital_Have_Buff(void);
void Midware_Digital_Get_Error(DIGITAL_ERROR *stError);
void Midware_Digital_WakeUp_UCB(void);
void Midware_Digital_Erp(void);
UCHAR Midware_Digital_Get_Crc8(UCHAR *pby_Data, UINT16 by_Length);
UCHAR Midware_Digital_Get_WakeupFlag(void);
UCHAR Midware_Digital_IsBusy(void);
void Midware_Digital_ReKeep_UartFunction(void);
void Midware_Digital_Set_Mode(UCHAR i);
UCHAR Midware_Digital_Get_Mode(void);
UINT16 Midware_Digital_Get_RxLen(void);
void Midware_Digital_Set_Update_Flag(UCHAR flag);
UCHAR Midware_Digital_Get_Update_Flag(void);

#endif	
