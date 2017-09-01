#include "Midware_Digital_01.h" 

#define DATA_TIME_OUT         600     //ms
#define DELAY_SENT_TIME       5      //ms  
#define CONNECT_FAIL_TIME     5000   //ms  

#define LCB_ERP_LEAVE         0x00
#define LCB_ERP_ENTER         0xFF
#define LCB_ERP_ACK           0x01

#define NORMAL_TX_START_DATA       0x01
#define NORMAL_RX_START_DATA       0x00

#define UPDATE_START_DATA          0xD1
#define UPDATE_END_DATA            0xD2

const UCHAR code_CRC8[16] =
{
  0X00,	0X31,	0X62,	0X53,	0XC4,	0XF5,	0XA6,	0X97,
  0XB9,	0X88,	0XDB,	0XEA,	0X7D,	0X4C,	0X1F,	0X2E,
};	

struct  
{
  unsigned TxStart     : 1;
  unsigned RxStart     : 1;
  unsigned RxOK        : 1;
  unsigned mode        : 1; //0:normal   1:update
  unsigned UpdateFlag  : 1; //0:not received update cmd 1:received update cmd
}COMM_STS;

__no_init static UINT16 delayCnt, RxLen, txLen;
__no_init static UCHAR buffer[DATA_LEN_MAX];
__no_init static UINT16 rxPoint,txPoint;
__no_init static UCHAR recivedStep;
__no_init static UCHAR delayTxTime;
__no_init static UINT16 connectFailCnt;
__no_init static DIGITAL_ERROR DIGITAL_ERR;

void Midware_Digital_Rx_Int(UCHAR by_Data);
void Midware_Digital_Tx_Int(void);

static void Midware_Digital_Send_String(UCHAR *pString, UINT16 byLength)
{
  while(COMM_STS.TxStart);
//  while(delayTxTime);                                                          
  
  COMM_STS.TxStart = 1;
  memcpy(buffer, pString, byLength);
  txPoint = 0;
  txLen = byLength;
  Hal_Digital_Occur_TxInt();
}

UCHAR Midware_Digital_Get_Crc8(UCHAR *pby_Data, UINT16 by_Length)
{
  UCHAR CRCcheckDat = 0;
  UCHAR CRChalfDat = 0;
  UCHAR CRCtempDat = 0;
  
  for(UINT16 by_Loop = 0; by_Loop < by_Length; by_Loop++)
  {
    CRCtempDat = pby_Data[by_Loop];
    CRChalfDat = (CRCcheckDat >> 4);
    CRCcheckDat <<= 4;
    CRCcheckDat ^= code_CRC8[CRChalfDat ^ (CRCtempDat >> 4)];
    CRChalfDat = (CRCcheckDat >> 4);
    CRCcheckDat <<= 4;
    CRCcheckDat ^= code_CRC8[CRChalfDat ^ (CRCtempDat & 0x0F)];
  }      
  return (CRCcheckDat);
}

void Midware_Digital_Initial_HW(UCHAR mode)
{
  Hal_Digital_Initial(mode); 
  Hal_Digital_Set_RxTxFuct(Midware_Digital_Rx_Int, Midware_Digital_Tx_Int);
  COMM_STS.mode = mode;
}

void Midware_Digital_Initial_Data(void)
{
  COMM_STS.RxOK = 0;
  COMM_STS.RxStart = 0;
  COMM_STS.TxStart = 0; 
  COMM_STS.mode = NORMAL_MODE; 
  COMM_STS.UpdateFlag = 0;
  memset(&DIGITAL_ERR, 0, sizeof(DIGITAL_ERR));
  recivedStep = NORMAL_START;  
  connectFailCnt = 0;
  delayTxTime = 0;  
  RxLen = 0;
  
}

UCHAR Midware_Digital_Process(UCHAR *commandRx)
{
  if(COMM_STS.RxOK == 0) 
    return 0;  
  COMM_STS.RxOK = 0;
  
  if(COMM_STS.mode == 0)
  {
    memcpy(commandRx, &buffer, sizeof(buffer));
    return commandRx[NORMAL_COMMD];
  }
  else
  {
    UINT16 lenChange = UPDATE_DATA;
    commandRx[UPDATE_COMMD] = buffer[1];
    for(UINT16 loop = UPDATE_DATA; loop < RxLen; loop++)
    {
      if(buffer[loop] == 0xD0 && buffer[loop + 1] <= 3)
      {
        commandRx[lenChange++] = buffer[loop] + buffer[loop + 1];
        loop += 1;
      }
      else
        commandRx[lenChange++] = buffer[loop];
    }
    
    if(lenChange >= 3)
      RxLen = lenChange - 3;
    else
      RxLen = 0;
  }
  return commandRx[UPDATE_COMMD];
}

void Midware_Digital_Send_Command(UCHAR *commandTx, UINT16 Datalen)
{
  UCHAR txTemp[DATA_LEN_MAX];
  UINT16 tempLen = 0;
  if(COMM_STS.mode == 0)
  {
//    txTemp[NORMAL_START] = NORMAL_TX_START_DATA;
//    txTemp[NORMAL_STATUS] = commandTx[NORMAL_STATUS];
//    txTemp[NORMAL_COMMD] = commandTx[NORMAL_COMMD];
//    txTemp[NORMAL_LEN] = commandTx[NORMAL_LEN];
    for(UCHAR i = 0; i < commandTx[NORMAL_LEN]; i++)
    {		
      txTemp[i] = commandTx[NORMAL_DATA + i];
    }
//    txTemp[commandTx[NORMAL_LEN] + NORMAL_DATA] = Midware_Digital_Get_Crc8(txTemp, commandTx[NORMAL_LEN] + NORMAL_DATA);
    tempLen = commandTx[NORMAL_LEN] ;
  }
  else
  {
    UINT16 index = UPDATE_DATA;
    txTemp[UPDATE_START] = UPDATE_START_DATA;
    txTemp[UPDATE_COMMD] = commandTx[UPDATE_COMMD];
    for(UINT16 loop = UPDATE_DATA; loop < (Datalen + UPDATE_DATA); loop++)
    {
      if(commandTx[loop] >= 0xD0 && commandTx[loop] <= 0XD3)
      {
        txTemp[index++] = 0xD0;
        txTemp[index++] = commandTx[loop] - 0xD0;
      }
      else
        txTemp[index++] = commandTx[loop];
    }
    
    txTemp[index++] = UPDATE_END_DATA;
    tempLen = index;
  }
  Midware_Digital_Send_String(txTemp, tempLen);
}

void Midware_Digital_1ms_Int(void)
{
  static UINT16 countMs = 0;
  
  if(COMM_STS.TxStart || COMM_STS.RxStart)
  {	
    if(++countMs > DATA_TIME_OUT)
    {
      countMs = 0; 
      if(COMM_STS.TxStart)
      {    
        DIGITAL_ERR.Digital_Tx_Timeout = 1;
      }
      if(COMM_STS.RxStart)
      {
        DIGITAL_ERR.Digital_Rx_Timeout = 1;
      }           
      recivedStep = NORMAL_START;
      COMM_STS.RxStart = 0;
      Hal_Digital_Enable_RxInt();
      COMM_STS.TxStart = 0;
    }
  }
  else countMs = 0;
  
  if(delayTxTime) --delayTxTime;
  
  if(connectFailCnt < CONNECT_FAIL_TIME) 
  {
    ++connectFailCnt;
  }
  else
  {
    DIGITAL_ERR.Digital_TimeOut = 1;
  }
  
  if(delayCnt > 0) --delayCnt;
}

void Midware_Digital_Get_Error(DIGITAL_ERROR *stError)
{
  memcpy(stError, &DIGITAL_ERR, sizeof(DIGITAL_ERR));
}

void Midware_Digital_Rx_Int(UCHAR by_Data)
{
  UCHAR temp = by_Data;
  
  if(!COMM_STS.RxOK)
  {
    if(COMM_STS.mode == 0)
    {
      if(recivedStep == NORMAL_START)  
      {
        rxPoint = 0;
        buffer[rxPoint] = temp;
      }
      else buffer[++rxPoint] = temp;
      
      switch(recivedStep)
      {
      default:
      case NORMAL_START:
        COMM_STS.RxOK = 0;
        if(temp != NORMAL_RX_START_DATA)
        {
          recivedStep = NORMAL_START;
        }
        else
        {
          COMM_STS.RxStart = 1;
          recivedStep = NORMAL_STATUS;
        }
        break;
      case NORMAL_STATUS:
      case NORMAL_COMMD:
        ++recivedStep;
        break;
      case NORMAL_LEN:
        recivedStep = NORMAL_DATA;
        if(buffer[rxPoint] == 0)
          recivedStep = NORMAL_CRC;
        break;
      case NORMAL_DATA:
        if((rxPoint - 3) >= buffer[NORMAL_LEN])              
        {
          recivedStep = NORMAL_CRC;
        }
        break;
      case NORMAL_CRC:
        if(Midware_Digital_Get_Crc8(buffer, rxPoint) == buffer[rxPoint])
        {
          COMM_STS.RxOK = 1;
          delayTxTime = DELAY_SENT_TIME;
          connectFailCnt = 0;
          if(buffer[2] == 0x04)
          {
            COMM_STS.RxStart = 0;
          }
        }
        else 
        {
          if(buffer[2] == 0x04)
          {
            COMM_STS.RxStart = 0;
          }
          DIGITAL_ERR.Digital_CRC_Error = 1;
        }
        COMM_STS.RxStart = 0;
        recivedStep = NORMAL_START;
        break;
      }
    }
    else
    {      
      if(temp == UPDATE_START_DATA)
      {
        rxPoint = 0;
        COMM_STS.RxStart = 1;
      }
      else if(temp == UPDATE_END_DATA)
      {
        RxLen = rxPoint + 1;
        COMM_STS.RxStart = 0;
        COMM_STS.RxOK = 1;
        connectFailCnt = 0;
        delayTxTime = DELAY_SENT_TIME;
      }      
      buffer[rxPoint] = temp;
      if(++rxPoint > (DATA_LEN_MAX - 1) || COMM_STS.RxOK)
      {
        rxPoint = 0;
      } 
    }
  }
}

void Midware_Digital_Tx_Int(void)
{
  if(txPoint >= txLen)
  {
    txLen = 0;
    COMM_STS.TxStart = 0;
    Hal_Digital_Enable_RxInt();
  }
  else
  {
    Hal_Digital_Send_Byte(buffer[txPoint++]);
  }
}

void Midware_Digital_WakeUp_UCB(void)
{
  Hal_Digital_Uart_Disable();
  Hal_Digital_Set_Direction(1);	
  Hal_Digital_Set_TxLevel(0);
  delayCnt = 500;
  while( delayCnt > 0);
  Hal_Digital_Set_TxLevel(1);
  delayCnt = 50;
  while( delayCnt > 0);
}

UCHAR Midware_Digital_Get_Rx(void) 
{
  return Hal_Digital_Get_Rx();
}

void Midware_Digital_Erp(void)
{
  Hal_Digital_Erp();
}

UCHAR Midware_Digital_IsBusy(void)
{
  return COMM_STS.TxStart;
}

void Midware_Digital_ReKeep_UartFunction(void)
{
  Hal_Digital_ReKeep_UartFunction();
}

UCHAR Midware_Digital_Get_WakeupFlag(void)
{
  return Hal_Digital_Get_WakeupFlag();
}

void Midware_Digital_Set_Mode(UCHAR i)
{
  COMM_STS.mode = i > 0 ? UPDATE_MODE : NORMAL_MODE;
}

UCHAR Midware_Digital_Get_Mode(void)
{
  return COMM_STS.mode;
}

UINT16 Midware_Digital_Get_RxLen(void)
{
  return RxLen;
}

void Midware_Digital_Set_Update_Flag(UCHAR flag)
{
  COMM_STS.UpdateFlag = flag > 0 ? 1 : 0;
}

UCHAR Midware_Digital_Get_Update_Flag(void)
{
  return COMM_STS.UpdateFlag;
}
