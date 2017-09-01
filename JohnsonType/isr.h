/******************************************************************************
*
* Freescale Semiconductor Inc.
* (c) Copyright 2013 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
*
***************************************************************************
*
* THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
* IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
* THE POSSIBILITY OF SUCH DAMAGE.
*
***************************************************************************//*!
*
* @file isr.h
*
* @author Freescale
*
* @version 0.0.1
*
* @date Jun. 25, 2013
*
* @brief define interrupt service routines referenced by the vector table.
*
* Note: Only "vectors.c" should include this header file.
*
*******************************************************************************
******************************************************************************/

#ifndef __ISR_H
#define __ISR_H

#include "project.h"

/* Example */
/*
#undef  VECTOR_036
#define VECTOR_036 RTC_Isr

// ISR(s) are defined in your project directory.
extern void RTC_Isr(void);
*/

/*!
 * @brief define interrupt service routine for different vectors.
 *
 */


#undef  VECTOR_037
#define VECTOR_037      ACMP1_Isr
     
#undef  VECTOR_036
#define VECTOR_036      RTC_Isr             /*!< Vector 36 points to RTC interrupt service routine */

#undef  VECTOR_035
#define VECTOR_035      FTM2_Isr            /*!< Vector 35 points to FTM2 interrupt service routine */

#undef  VECTOR_034
#define VECTOR_034      FTM1_Isr

#undef  VECTOR_033
#define VECTOR_033      FTM0_Isr            /*!< Vector 33 points to FTM0 interrupt service routine */

#undef  VECTOR_031
#define VECTOR_031      ADC_Isr

#undef  VECTOR_030
#define VECTOR_030      UART2_Isr

#undef  VECTOR_029
#define VECTOR_029      UART1_Isr

#undef  VECTOR_028
#define VECTOR_028      UART0_Isr
  
#undef  VECTOR_031
#define VECTOR_031      ADC_Isr

#ifdef __LCB_SYS__
#undef  VECTOR_002
#define VECTOR_002      NMI_Isr      

#undef  VECTOR_032
#define VECTOR_032      ACMP0_Isr

#undef  VECTOR_040
#define VECTOR_040      KBI0_Isr

#undef  VECTOR_041
#define VECTOR_041      KBI1_Isr
#endif


#undef  VECTOR_037
#define VECTOR_037      ACMP1_Isr


 void RTC_Isr(void);
 void FTM2_Isr(void);
 void FTM1_Isr(void);
 void FTM0_Isr(void);
 void UART0_Isr(void);
 void UART1_Isr(void);
 void UART2_Isr(void);
 void ADC_Isr(void);
 void ACMP1_Isr(void);
 #ifdef __LCB_SYS__
 void NMI_Isr(void);
 void KBI1_Isr(void);
 void ACMP0_Isr(void);
 void KBI0_Isr(void);
 #endif

#endif  //__ISR_H

/* End of "isr.h" */
