#include "Hal_Led_01.h"

#define LED_PIN_PORT                 GPIOB
#define LED_PIN_MASK                 GPIO_PTE2_MASK
#define LED_PIN                      GPIO_PTE2

void Hal_Led_Initial(void) 
{
    GPIO_Init(LED_PIN_PORT, LED_PIN_MASK, GPIO_PinOutput); 
    GPIO_PinClear(LED_PIN); 
}

void Hal_Led_Toggle(void) 
{
    GPIO_PinToggle(LED_PIN); 
}

void Hal_Led_Set_High(void)
{
    GPIO_PinSet(LED_PIN);
}

void Hal_Led_Set_Low(void) 
{
    GPIO_PinClear(LED_PIN); 
}

void Hal_Led_ERP(void) 
{
    GPIO_Init(LED_PIN_PORT, LED_PIN_MASK, GPIO_PinInput);  
}








