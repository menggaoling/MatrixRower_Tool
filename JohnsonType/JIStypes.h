
#ifndef _JISTYPES_H_
#define _JISTYPES_H_

#include <stdarg.h>        
#include <stdio.h>         
#include <math.h>          
#include <stdbool.h>        
#include <stdint.h>

#include "uart.h"
#include "common.h"
#include "gpio.h"
#include "kbi.h"
#include "sim.h"
#include "ftm.h"
#include "adc.h"
#include "Flash.h"
#include "rtc.h"
#include "wdog.h"
#include "acmp.h"

#include "project.h"

typedef signed   int        INT;
typedef unsigned int        UINT;

typedef signed char         CHAR;

typedef unsigned char       UCHAR;
typedef unsigned char       BYTE;

typedef unsigned short      USHORT;
typedef unsigned long       DWORD;
typedef unsigned short      WORD;

typedef char                SCHAR;

typedef void                VOID;
typedef void *              PVOID;

typedef signed char         INT8;
typedef signed char *       PINT8;
typedef unsigned char       UINT8;
typedef unsigned char *     PUINT8;


typedef signed   int*       PINT;

typedef unsigned   int*     PUINT;

typedef signed char *       PCHAR;

typedef unsigned char *     PUCHAR;
typedef signed char *       PSTR;
typedef const signed char * PCSTR;

typedef short             INT16;
typedef short *           PINT16;
typedef unsigned short    UINT16;
typedef unsigned short *  PUINT16;

typedef signed int        INT32;
typedef signed int *      PINT32;
typedef unsigned int      UINT32;
typedef unsigned int *    PUINT32;

typedef float             FLOAT;
typedef float *           PFLOAT;

typedef double            DOUBLE;
typedef double *          PDOUBLE;

typedef union
{
    UCHAR  data8[4];
    UINT16 data16[2];
    UINT32 data32;
}DataUnion32;

typedef union
{
    UCHAR  data8[2];
    UINT16 data16;
} DataUnion16;


#ifndef BIT0
#define BIT0                (0x0001)
#define BIT1                (0x0002)
#define BIT2                (0x0004)
#define BIT3                (0x0008)
#define BIT4                (0x0010)
#define BIT5                (0x0020)
#define BIT6                (0x0040)
#define BIT7                (0x0080)
#define BIT8                (0x0100)
#define BIT9                (0x0200)
#define BIT10               (0x0400)
#define BIT11               (0x0800)
#define BIT12               (0x1000)
#define BIT13               (0x2000)
#define BIT14               (0x4000)
#define BIT15               (0x8000)
#endif


#define HWREG32(x)          (*((volatile unsigned long *)(x)))
#define HWREG16(x)          (*((volatile unsigned short *)(x)))
#define HWREG8(x)           (*((volatile unsigned char *)(x)))
            
#ifndef BITSET
#define BITSET(X, MASK)     ( (X) |=  (MASK) )
#endif

#ifndef BITCLR
#define BITCLR(X, MASK)     ( (X) &= ~(MASK) )
#endif

#ifndef BITGET
#define BITGET(X, MASK)     ( ( (X) & (MASK) ) ? 1 : 0 )
#endif

#define MAX(a, b)           ( (a) > (b) ? (a) : (b) )
#define MIN(a, b)           ( (a) < (b) ? (a) : (b) )
#define MID(a, b)           ( ((a) + (b)) / 2 )

#define SWAP(X)             ( ( (X)  << ((sizeof(X) * 8) >> 1) ) | ((X)  >> ((sizeof(X) * 8) >> 1) ) )

#ifdef	NULL
#undef	NULL
#endif
#define NULL	(0)

#ifdef	TRUE
#undef	TRUE
#endif
#define	TRUE	(!0)

#ifdef	FALSE
#undef	FALSE
#endif
#define FALSE	(0)

#ifdef  ON
#undef  ON
#endif
#define ON      (!0)

#ifdef  OFF
#undef  OFF
#endif
#define OFF     (0)




typedef void (*FNCT_VOID) (void);
typedef void (*FNCT_UCHAR)(UCHAR);

#endif 
