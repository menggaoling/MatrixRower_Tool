#include "Hal_System_02.h"
#include "gpio.h"
#include "rtc.h"
#include "wdog.h"
#include "adc.h"


#define POWER_ERP_PORT                GPIOA
#define POWER_ERP_PIN                 GPIO_PTB1



static void Hal_System_Initial_Clock(void) //clock
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
    RTC_ConfigType  sRTCConfig = {0};
    RTC_ConfigType  *pRTC_Config = &sRTCConfig;
    /* configure RTC to 1KHz(1ms) interrupt frequency */
    pRTC_Config->u16ModuloValue = 250 - 1;
    pRTC_Config->bInterruptEn   = RTC_INTERRUPT_ENABLE;     /*!< enable interrupt */
    pRTC_Config->bClockSource   = RTC_CLKSRC_EXTERNAL;      /*!< clock source is 8MHz */
    pRTC_Config->bClockPresaler = RTC_CLK_PRESCALER_100;    /*!< prescaler is 32 */
    RTC_Init(pRTC_Config);
}

static void Hal_System_Initial_Lock_Mcu(void)
{
    if(!(FTMRH->FCLKDIV & FTMRH_FCLKDIV_FDIVLCK_MASK))
    {
        // FCLKDIV register is not locked,FCLKDIV[FDIV] = 0X13
        FTMRH->FCLKDIV = (FTMRH->FCLKDIV & ~(FTMRH_FCLKDIV_FDIV_MASK)) | FTMRH_FCLKDIV_FDIV(BUS_CLK / 1000000L - 1);
        FTMRH->FCLKDIV |= FTMRH_FCLKDIV_FDIVLCK_MASK; //lock
    }
}
static void Hal_System_Initial_WDT(void) 
{
    WDOG_ConfigType sWDOGConfig = {0};   
    sWDOGConfig.sBits.bWaitEnable   = TRUE;
    sWDOGConfig.sBits.bStopEnable   = TRUE;
    sWDOGConfig.sBits.bDbgEnable    = TRUE;
    sWDOGConfig.sBits.bUpdateEnable = FALSE;
    sWDOGConfig.sBits.bDisable      = FALSE; /* enable WDOG */
    sWDOGConfig.sBits.bClkSrc       = WDOG_CLK_INTERNAL_1KHZ;
    sWDOGConfig.u16TimeOut          = 1000;  /* 1s */
    sWDOGConfig.u16WinTime          = 0;  
    WDOG_Init(&sWDOGConfig);
	
}
static void Hal_System_Initial_ADC(void)
{
    ADC_ConfigType  sADC = {0};
    
    sADC.u8ClockSource = CLOCK_SOURCE_BUS_CLOCK;
    sADC.u8ClockDiv = ADC_ADIV_DIVIDE_4;
    sADC.u8Mode = ADC_MODE_12BIT;
    sADC.sSetting.bIntEn = 0;
    ADC_Init(ADC, &sADC);
    ADC_SetHighSpeed(ADC);
}
static void Hal_System_Power_Initial(void) 
{
    GPIO_Init(POWER_ERP_PORT, 1 << (POWER_ERP_PIN % 32), GPIO_PinOutput); 
    GPIO_PinSet(POWER_ERP_PIN);  
}

void Hal_System_Initial(void)
{
    Hal_System_Initial_Clock();
    Hal_System_Initial_RTC();  
    Hal_System_Initial_WDT();
    Hal_System_Initial_ADC();
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
    ADC_DeInit(ADC);
    GPIO_PinClear(POWER_ERP_PIN); 
}
void Hal_System_WDOG_Feed(void)
{
    WDOG_Feed();
}
void Hal_System_Empty_IO(void)
{
#ifdef DIGITAL_MCB
   //PTA
    GPIO_Init(GPIOA, 1 << (GPIO_PTA1 % 32), GPIO_PinOutput);
    GPIO_PinSet(GPIO_PTA1);
    GPIO_Init(GPIOA, 1 << (GPIO_PTA3 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTA3);
    
    //PTB
    GPIO_Init(GPIOA, 1 << (GPIO_PTB0 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTB0);
    
    //PTC
    GPIO_Init(GPIOA, 1 << (GPIO_PTC1 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTC1);
    
    //PTD
    GPIO_Init(GPIOA, 1 << (GPIO_PTD4 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTD4);
    
    //PTE
    GPIO_Init(GPIOB, 1 << (GPIO_PTE1 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTE1);
    GPIO_Init(GPIOB, 1 << (GPIO_PTE2 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTE2);    
    
    //PTI
    GPIO_Init(GPIOC, 1 << (GPIO_PTI4 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTI4);    

#else

   //PTA
    GPIO_Init(GPIOA, 1 << (GPIO_PTA1 % 32), GPIO_PinOutput);
    GPIO_PinSet(GPIO_PTA1);
    GPIO_Init(GPIOA, 1 << (GPIO_PTA2 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTA2);
    GPIO_Init(GPIOA, 1 << (GPIO_PTA3 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTA3);
    
    //PTB
    GPIO_Init(GPIOA, 1 << (GPIO_PTB0 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTB0);
    
    //PTC
    GPIO_Init(GPIOA, 1 << (GPIO_PTC1 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTC1);
    GPIO_Init(GPIOA, 1 << (GPIO_PTC2 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTC2);
    
    //PTD
    GPIO_Init(GPIOA, 1 << (GPIO_PTD4 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTD4);
    
    //PTE
    GPIO_Init(GPIOB, 1 << (GPIO_PTE1 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTE1);
    GPIO_Init(GPIOB, 1 << (GPIO_PTE2 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTE2);   
    //PTH
    GPIO_Init(GPIOB, 1 << (GPIO_PTH2 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTH2);   
    
    //PTI
    GPIO_Init(GPIOC, 1 << (GPIO_PTI4 % 32), GPIO_PinOutput);
    GPIO_PinClear(GPIO_PTI4);   
#endif
}

