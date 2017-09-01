#include "Hal_System_01.h"

#define BUS_CLK                   20000000L
#define DC_ERP_PORT               GPIOA
#define DC_ERP_PIN_MASK           GPIO_PTB3_MASK
#define DC_ERP_PIN                GPIO_PTB3

static void Hal_System_Initial_Clock(void) 
{
    //External OSC = 8MHz
    OSC->CR |= OSC_CR_OSCEN_MASK;   //1->OSC module is enable
    //0->OSC module is disable
    OSC->CR |= OSC_CR_OSCSTEN_MASK; //1->OSC clock is enable in stop mode
    //0->OSC clock is disable in stop mode
    OSC->CR |= OSC_CR_OSCOS_MASK;   //1:Oscillator clock source is selected
    //0:External clock source from EXTAL PIN is select
    OSC->CR |= OSC_CR_RANGE_MASK;   //1:High frequency range of 4~24MHz
    
    OSC->CR |= OSC_CR_HGO_MASK;  //0:Low power mode ; 1:High-gain mode
    
    OSC->CR &= 0xDF;
    
    ICS->C2 = (ICS->C2 & ~(ICS_C2_BDIV_MASK)) | ICS_C2_BDIV(0); //divides the selected clock by 1
    ICS->C2 = ICS->C2 & ~(ICS_C2_LP_MASK);  //FLL is not disabled in bypass mode
    while(!(OSC->CR & OSC_CR_OSCINIT_MASK)); //wait for OSC to be initialized
    
    //System clock = FLL = (8MHZ/256)*1280=40MHz
    ICS->C1 = (ICS->C1 & ~(ICS_C1_CLKS_MASK)) | ICS_C1_CLKS(0); //00->Output of FLL is selected to controls the bus frequency
    ICS->C1 = (ICS->C1 & ~(ICS_C1_RDIV_MASK)) | ICS_C1_RDIV(3); //256(OSC_CR[Range]=1)
    ICS->C1 &= ~ICS_C1_IREFS_MASK;    //0->External reference clock is selected as the clock source for FLL
    ICS->C1 |= ICS_C1_IRCLKEN_MASK;   //1->ICSIRCLK is active(enables the internal reference clock for use as ICSIRCLK)
    ICS->C1 &= ~ICS_C1_IREFSTEN_MASK; //0->Internal reference clock is disable in stop mode
    //1->Internal reference clock stays enable in stop mode
    //BUS CLK = 40MHz/2=20MHz
    SIM->CLKDIV |= SIM_CLKDIV_OUTDIV2_MASK;  
}

static void Hal_System_Initial_RTC(void) 
{
    RTC_ConfigType  sRTCConfig ={0};
    RTC_ConfigType  *pRTC_Config=&sRTCConfig;     
    /* configure RTC to 1KHz(1ms) interrupt frequency */ 
    pRTC_Config->u16ModuloValue = 250-1;
    pRTC_Config->bInterruptEn   = RTC_INTERRUPT_ENABLE;     /*!< enable interrupt */
    pRTC_Config->bClockSource   = RTC_CLKSRC_EXTERNAL;      /*!< clock source is 8MHz */
    pRTC_Config->bClockPresaler = RTC_CLK_PRESCALER_100;    /*!< prescaler is 32 */
    RTC_Init(pRTC_Config);
}

static void Hal_System_Initial_WDT(void) 
{
    WDOG_ConfigType sWDOGConfig = {0};   
    sWDOGConfig.sBits.bWaitEnable   = TRUE;
    sWDOGConfig.sBits.bStopEnable   = TRUE;
    sWDOGConfig.sBits.bDbgEnable    = FALSE;
    sWDOGConfig.sBits.bUpdateEnable = FALSE;
    sWDOGConfig.sBits.bDisable      = FALSE; /* enable WDOG */
    sWDOGConfig.sBits.bClkSrc       = WDOG_CLK_INTERNAL_1KHZ;
    sWDOGConfig.u16TimeOut          = 1000;  /* 1s */
    sWDOGConfig.u16WinTime          = 0;  
    WDOG_Init(&sWDOGConfig);
}


static void Hal_System_Initial_Lock_Mcu(void)
{
    if(!(FTMRH->FCLKDIV & FTMRH_FCLKDIV_FDIVLCK_MASK))
    {
        // FCLKDIV register is not locked,FCLKDIV[FDIV] = 0X13
        FTMRH->FCLKDIV = (FTMRH->FCLKDIV & ~(FTMRH_FCLKDIV_FDIV_MASK)) | FTMRH_FCLKDIV_FDIV(BUS_CLK/1000000L - 1);			  
        FTMRH->FCLKDIV |= FTMRH_FCLKDIV_FDIVLCK_MASK; //lock 
    }
}

static void Hal_System_Power_Initial(void) 
{
    GPIO_Init(DC_ERP_PORT, DC_ERP_PIN_MASK, GPIO_PinOutput); 
    GPIO_PinSet(DC_ERP_PIN);  
}

void Hal_System_Initial()
{
    Hal_System_Initial_Clock();
    Hal_System_Initial_RTC();  
//    Hal_System_Initial_WDT();
    Hal_System_Initial_Lock_Mcu();
    Hal_System_Power_Initial();
}

void Hal_System_EnableRTC(void)
{
    NVIC_EnableIRQ(RTC_IRQn); 
}

void Hal_System_ERP(void)
{  
    NVIC_DisableIRQ(RTC_IRQn); 
    GPIO_PinClear(DC_ERP_PIN); 
}

void Hal_System_WDOGFeed(void)
{  
  WDOG_Feed();
}

