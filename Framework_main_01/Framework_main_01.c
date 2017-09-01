
#include "Framework_main_01.h"

#define  ROTATE_ANGLE    45//360°/8 sensor

__no_init static struct
{
  UCHAR  by_ReveiveSleepCmd : 1;              //收到上控的睡眼命令
  UCHAR  byReSleep          : 1;
  UCHAR  byRPMwakeupFlag    : 1;
  UCHAR  byUCBwakeupFlag    : 1;
}MAIN;

typedef enum 
{
  TIMER_SLEEP_DELAY,
  TIMER_WAKEUP_DELAY,
  TIMER_FLITER,
  TIMER_TEST,
}TIMER_Type;

__no_init static UCHAR byGetOtherCMD;
__no_init static UINT wPowerOn;

/***********************************
Global LCB for ECB+Incline: 0xB3   
Global LCB for Induction: 0xB4
Global LCB for Induction+Incline: 0xB5
GlobaLCBforECB+Induction+Incline:0xB6
************************************/

void main_Initial_HW(void)
{
  DisableInterrupts;
  Midware_System_Initial_HW();
  Midware_RPM_Initial_HW(); 
  Midware_Led_Init_HW();
  Midware_Digital_Initial_HW(NORMAL_MODE);
  Midware_ECB_Initial_HW();
  Midware_Induction_Init_HW();
  Midware_Incline_Initial_HW();
  Midware_MachineType_Init_HW();
  EnableInterrupts;
}

void main_Initial_Data(void)
{
  DataUnion32 unData;
  byGetOtherCMD = 0;
  Midware_ECB_Initial_Data();
  Midware_ErrorCode_Initial_Data();
  Midware_Nvflash_Initial_Data();
  {
    unData.data16[0] = Midware_Nvflash_Get_OneCalibratedData(K_FACTOR_INDEX);
    unData.data16[1] = Midware_Nvflash_Get_OneCalibratedData(B_OFFSET_INDEX);
    Midware_Induction_Init_Data(unData.data16[0],unData.data16[1]);
  }
  Midware_Incline_Initial_Data();
  Midware_RPM_Initial_Data();
  Midware_Led_Init_Data();
  Midware_MachineType_Init_Data();
  Midware_Update_Init_Data();
  Midware_Digital_Initial_Data();
  Midware_Timer_Initial_Data();
  memset(&MAIN, 0, sizeof(MAIN));
  EnableInterrupts;
  Midware_Timer_Clear(TIMER_TEST);
}

void Main_UCB_Command_Process(void)
{  
  DataUnion32 unData;
  UCHAR CommandTx[DATA_LEN_MAX]; 
  UCHAR CommandRx[DATA_LEN_MAX]; 
  MCB_STATUS MCBsts;
  UCHAR cmd;
  
  MCBsts.status = 0;
  MCBsts.MCBerror = Midware_ErrorCode_Have_Error() ? 1 : 0;
  
  
  if(Midware_Timer_Counter(TIMER_TEST, 20))
  {
    cmd = CMD_TEST;
    Midware_Timer_Clear(TIMER_TEST);
  }
  
  if(cmd == CMD_TEST)
  {

    UINT16 txDataLen = 0;
    unData.data16[0] = 0;
    
        
    switch(cmd)
    {  
    case CmdEraseFlash:
    case CmdWriteFlash:
    case CmdReadFlash:
    case CmdReadProgramState:
    case CmdFlashUnlock:
    case CmdWriteCheckCode:
    case CmdReadUpdateMode:
      {
        Midware_Update_Process(cmd, &CommandRx[UPDATE_DATA], Midware_Digital_Get_RxLen(),&CommandTx[UPDATE_DATA], &txDataLen);
        CommandTx[UPDATE_COMMD] = cmd + 1;
        break;
      }   
    case CMD_UPDATE_PROGRAM:
      CommandTx[NORMAL_LEN] = 1;
      CommandTx[NORMAL_DATA] = 'N';
      
      if(CommandRx[NORMAL_LEN] == 2)
      {
        unData.data8[0] = CommandRx[NORMAL_DATA + 1];
        unData.data8[1] = CommandRx[NORMAL_DATA];
        if(unData.data8[1] == 'J' && unData.data8[0] == 'S')
        {
          Midware_Digital_Set_Update_Flag(1);
          CommandTx[NORMAL_DATA] = 'Y';
        }
      }
      break;
    case CMD_INITIAL: 
      {
        Midware_Digital_Initial_Data();
        break;
      }
    case CMD_GET_VERSION:   
      {
        unData.data16[0] = MCB_VERSION;
        CommandTx[NORMAL_LEN] = 2;
        CommandTx[NORMAL_DATA] = unData.data8[1];
        CommandTx[NORMAL_DATA + 1] = unData.data8[0];
        break;
      }  
    case CMD_GET_NEW_VERSION:
      {
        if(CommandRx[NORMAL_DATA] == 0xF0)
        {
          CommandTx[NORMAL_LEN] = 6;
          CommandTx[NORMAL_DATA] = 0xF0;
          CommandTx[NORMAL_DATA + 1] = 0x01;
          CommandTx[NORMAL_DATA + 2] = Midware_MachineType_Get_Type();    
          CommandTx[NORMAL_DATA + 3] = 0x00;
          CommandTx[NORMAL_DATA + 4] = M_VERSION; //Formal Version
          CommandTx[NORMAL_DATA + 5] = S_VERSION; //Deta Version
        }
        break;
      }  
    case CMD_GET_ECB_RPM:
      {
        CommandTx[NORMAL_LEN] = 2;
        unData.data16[0] = Midware_RPM_Get_RPM();
        CommandTx[NORMAL_DATA] = unData.data8[1];
        CommandTx[NORMAL_DATA + 1] = unData.data8[0];
        break;	
      }
    case CMD_SET_INCLINE_ACTION:   
      {
        Midware_Incline_Set_Action(CommandRx[NORMAL_DATA]);
        break;
      }     
    case CMD_SET_INCLINE_LOCATION:  
      {
        if(CommandRx[NORMAL_LEN] == 2)
        {
          unData.data8[0] = CommandRx[NORMAL_DATA + 1];
          unData.data8[1] = CommandRx[NORMAL_DATA];
          Midware_Incline_Set_TargetCount(unData.data16[0]);
          Midware_Incline_Allow_Act();
        } 
        break;
      }
    case CMD_GET_INCLINE_LOCATION:  
      {
        CommandTx[NORMAL_LEN] = 2;
        unData.data16[0] = Midware_Incline_Get_Count();
        CommandTx[NORMAL_DATA] = unData.data8[1];
        CommandTx[NORMAL_DATA + 1] = unData.data8[0];
        break; 
      } 	
    case CMD_SET_INDUCTION_ADC: 
      {
        if(CommandRx[NORMAL_LEN] == 2)
        {
          CommandTx[NORMAL_LEN] = 2;	
          unData.data8[0] = CommandRx[NORMAL_DATA + 1];
          unData.data8[1] = CommandRx[NORMAL_DATA];
          Midware_Induction_SET_TargetCurrentA(unData.data16[0]);
        }
        break;
      }   
    case CMD_SET_ERPMODE:
      {
        if(CommandRx[NORMAL_LEN] == 1)   //sleep
        {
          if(CommandRx[NORMAL_DATA] == 0xFF)
          {
            CommandTx[NORMAL_LEN] = 1;
            CommandTx[NORMAL_DATA] = 0x01;
            MAIN.by_ReveiveSleepCmd = 1;
            MAIN.byRPMwakeupFlag = 0;
            MAIN.byUCBwakeupFlag = 0;
            Midware_Timer_Clear(TIMER_SLEEP_DELAY);
          }
          else if(CommandRx[NORMAL_DATA] == 0x00)   //wakeup
          {
            CommandTx[NORMAL_LEN]= 1;
            CommandTx[NORMAL_DATA] = 0x01;
            MAIN.by_ReveiveSleepCmd = 0;
          }
        }
        break;
      }       
    case CMD_SKIP_CURRENT_ERROR:
      {
        Midware_ErrorCode_Skip_Last();
        break;
      }
    case CMD_GET_ERROR_CODE:
      {
        CommandTx[NORMAL_LEN] = 2;
        unData.data16[0] = Midware_ErrorCode_Get_ErrorCode();
        CommandTx[NORMAL_DATA] = unData.data8[1];
        CommandTx[NORMAL_DATA + 1] = unData.data8[0];
        break;
      }
    case CMD_SET_ECB_ZERO:          
      {
        Midware_ECB_Clr_ZeroFlag();
        break;
      }
    case CMD_SET_ECB_ACTION:        
      {
        Midware_ECB_Set_Action(CommandRx[NORMAL_DATA]);
        break;
      }
    case CMD_SET_ECB_LOCATION:      
      {
        if(CommandRx[NORMAL_LEN] == 2)
        {
          unData.data8[0] = CommandRx[NORMAL_DATA + 1];
          unData.data8[1] = CommandRx[NORMAL_DATA];
          Midware_ECB_Set_TargetCount(unData.data16[0]);
        }
        break;
      }
    case CMD_GET_ECB_LOCATION:    
    case CMD_GET_ECB_COUNT:   
      {
        
        CommandTx[NORMAL_LEN] = 2;
        unData.data16[0] = Midware_ECB_Get_Count();
        CommandTx[NORMAL_DATA] = unData.data8[1];
        CommandTx[NORMAL_DATA + 1] = unData.data8[0];
        break;
      } 
    case CMD_SET_IND_BASE_CURRENT:   
      {
        unData.data8[0] = CommandRx[NORMAL_DATA + 2];
        unData.data8[1] = CommandRx[NORMAL_DATA + 1];
        Midware_Induction_Set_BaseCurrent(CommandRx[NORMAL_DATA], unData.data16[0]);
        break;
      }   
    case CMD_SET_INDUCTION_PWM:   
      {
        unData.data8[0] = CommandRx[NORMAL_DATA + 1];
        unData.data8[1] = CommandRx[NORMAL_DATA];
        Midware_Induction_Set_PWM(unData.data16[0]);
        break;
      }   
    case CMD_FINISH_INDUCTION_ADJUST:   
      {
        static UCHAR SavePoint = 0;
        if(CommandRx[NORMAL_DATA] == 0 && SavePoint == 0)
        {
          SavePoint = 1;
          Midware_Induction_Finish_Calibrate(0);
        }
        else if(CommandRx[NORMAL_DATA] == 1 && SavePoint == 1)
        {
          SavePoint = 0;
          Midware_Induction_Finish_Calibrate(1);
          Midware_Nvflash_Prepare_OneCalibratedData(K_FACTOR_INDEX, Midware_Induction_Get_Kdata());  
          Midware_Nvflash_Prepare_OneCalibratedData(B_OFFSET_INDEX, Midware_Induction_Get_Bdata());
          Midware_Nvflash_Save_CalibratedDatas();
        }
        break;
      }  
    case CMD_TEST:   
      {
        UINT32 Torque;
        char printBuffer[16] = {0};        
        CommandTx[NORMAL_COMMD] = CMD_TEST;
        CommandTx[NORMAL_LEN] = 26;
        Midware_RPM_Process();
        unData.data32 = ((Midware_Timer_Get()<<4)/10);//per pulse is 1.6 us
        sprintf(printBuffer,"%8d,",unData.data32);
        memcpy( CommandTx+NORMAL_DATA,printBuffer,9);
        
        sprintf(printBuffer,"%5d,",Midware_RPM_Get_RPM());
//        sprintf(printBuffer,"%5d,",Midware_RPM_Get(0));
        memcpy(CommandTx+NORMAL_DATA + 9, printBuffer,6);
        
        unData.data16[0] = Midware_RPM_Get(0);
        unData.data16[1] = Midware_RPM_Get(1);
        if(unData.data16[0] > unData.data16[1])
          Torque = (unData.data16[0]*unData.data16[0] - unData.data16[1]*unData.data16[1]);
        else
          Torque = (unData.data16[1]*unData.data16[1] - unData.data16[0]*unData.data16[0]);
        Torque /= (ROTATE_ANGLE * Hal_RPM_Rotate_Pulse_Num()*100);
//        if(unData.data16[0] > unData.data16[1])
//          Torque = unData.data16[0] - unData.data16[1];
//        else
//          Torque = unData.data16[1] - unData.data16[0];
//        Torque = Midware_RPM_Get(1);
//        Torque /= 1;
        sprintf(printBuffer,"%9ld,",Torque);
        memcpy( CommandTx+NORMAL_DATA + 15,printBuffer,10);
        
        CommandTx[NORMAL_DATA + 25] = '\n';
        break;
      }  
    default:
      {
        MCBsts.commandError = 1;
        break;
      }   
    }
    if(++byGetOtherCMD > 10)
      byGetOtherCMD = 10;
    if(Midware_Digital_Get_Mode() == NORMAL_MODE)
    {
      CommandTx[NORMAL_STATUS] = MCBsts.status;
      CommandTx[NORMAL_COMMD] = cmd;
      txDataLen = CommandTx[NORMAL_LEN];
    }
    Midware_Digital_Send_Command(CommandTx, txDataLen);
  }
}

void Main_Deep_Sleep(void)
{   
  static UCHAR retry = 0;
  if(MAIN.by_ReveiveSleepCmd)
  {
    if(MAIN.byRPMwakeupFlag == 1)
    {
      if(Midware_Timer_Counter(TIMER_WAKEUP_DELAY, 200))
      {
        Midware_Timer_Clear(TIMER_WAKEUP_DELAY);
        if(++retry <= 3)
        {
          Midware_Digital_WakeUp_UCB();
        }
        else
        {
          retry = 0;
          MAIN.byRPMwakeupFlag = 0;
          Midware_Timer_Clear(TIMER_SLEEP_DELAY);
          if(byGetOtherCMD >= 10)
            MAIN.by_ReveiveSleepCmd = 0;
        }
      }
    }
    else if(MAIN.byUCBwakeupFlag == 1)
    {
      if(Midware_Timer_Counter(TIMER_WAKEUP_DELAY, 300))
      {
        Midware_Timer_Clear(TIMER_WAKEUP_DELAY);
        Midware_Timer_Clear(TIMER_SLEEP_DELAY);
        MAIN.byUCBwakeupFlag = 0;
        if(byGetOtherCMD >= 5)
          MAIN.by_ReveiveSleepCmd = 0;
      }
    }
    else
    {
      if(Midware_Timer_Counter(TIMER_SLEEP_DELAY, 100))
      {
        DisableInterrupts;
        Midware_System_ERP(); 
        Midware_Incline_ERP();  
        Midware_ECB_ERP();     
        Midware_Digital_Erp();
        Midware_Led_ERP(); 
        Midware_RPM_Erp();
        Midware_Induction_ERP();  
        EnableInterrupts; 
        Midware_RPM_Get_WakeupFlag();
        byGetOtherCMD = 0;
        MAIN.byReSleep = 1;  
        while(MAIN.byReSleep)
        {
          Midware_System_ERP();
          stop(); 
          Midware_System_EnableRTC();                
          if(Midware_RPM_Get_WakeupFlag())
          {  
            MAIN.byReSleep = 0;
            Midware_Digital_WakeUp_UCB();
            MAIN.byRPMwakeupFlag = 1;
            retry = 0;
            Midware_Timer_Clear(TIMER_WAKEUP_DELAY);
          }
          else if(Midware_Digital_Get_WakeupFlag())
          {    
            MAIN.byReSleep = 0;
            Midware_Timer_Clear(TIMER_FLITER);    
            while(1)
            {
              if(Midware_Timer_Counter(TIMER_FLITER, 10))
              {
                Midware_Timer_Clear(TIMER_WAKEUP_DELAY);
                break;
              }
            }
            if(Midware_Digital_Get_Rx() > 0) 
            {
              MAIN.byReSleep = 1;
              Midware_Digital_ReKeep_UartFunction();
            }
            else 
            {
              MAIN.byUCBwakeupFlag = 1;
            }
          }
        }  
        main_Initial_HW(); 
        main_Initial_Data();
      }
    }
  }
}
void Main_Update_Process(void)
{
  if(Midware_Digital_Get_Update_Flag()&&\
    !Midware_Digital_IsBusy())
  {
    Midware_Digital_Set_Update_Flag(0);
    Midware_Digital_Set_Mode(UPDATE_MODE);
    Midware_Digital_Initial_HW(Midware_Digital_Get_Mode());
  }
}

int main(void)
{   
  main_Initial_HW(); 
  if(wPowerOn != 0xaa55)
  {
    wPowerOn = 0xaa55;
    main_Initial_Data();  
  }
  while(1)
  {	     
//    Midware_System_WDOGFeed();  //WATCHDOG_RESET      
//    Midware_MachineType_Process();
//    Midware_RPM_Process();         
//    Midware_Induction_Process();
//    Midware_ECB_Process();
//    Midware_Incline_Process(0);
//    Midware_ErrorCode_Process(); 
    Main_UCB_Command_Process(); 
//    Main_Update_Process();
//    Main_Deep_Sleep();    
  } 
}









