#ifndef _RTC_DS1307_H_
#define _RTC_DS1307_H_

/*==================[inclusions]=============================================*/
#include <stdint.h>
#include "sapi.h"              // <= sAPI header
#include "ff.h"       // <= Biblioteca FAT FS
#include "fssdc.h"    // API de bajo nivel para unidad "SDC:" en FAT FS

/*==================[typedef]================================================*/
#ifdef __cplusplus
extern "C" {
#endif

/*==================[cplusplus]==============================================*/
// DEPENDENCIAS

#include "sapi.h"        // <= Biblioteca sAPI
#include "sapi_i2c.h"           /* <= sAPI I2C header */
#include "sapi_delay.h"         /* <= sAPI Delay header */
#include <string.h>

/*==================[macros]=================================================*/
// I2C baudrate
#define DS1307_I2C_RATE         100000 // 100 kHz, the DS1307 operates in the standard mode (100KHz) only.

// DS1307 registers
#define DS1307_sec          0x00
#define DS1307_min          0x01
#define DS1307_hour         0x02
#define DS1307_wday         0x03
#define DS1307_mday         0x04
#define DS1307_month        0x05
#define DS1307_year         0x06
#define DS1307_control      0x07
#define DS1307_ADDRESS      0b1101000

/*==================[tipos de datos declarados por el usuario]===============*/

typedef struct {
   uint8_t  year;	 /* 1 to 4095 */
   uint8_t  month;   /* 1 to 12   */
   uint8_t  mday;	 /* 1 to 31   */
   uint8_t  wday;	 /* 1 to 7    */
   uint8_t  hour;	 /* 0 to 23   */
   uint8_t  min;	 /* 0 to 59   */
   uint8_t  sec;	 /* 0 to 59   */
} DS1307_rtc_t;

/*==================[declaraciones de funciones externas]====================*/

bool_t DS1307rtcInit( DS1307_rtc_t* DS1307_rtc );

int8_t DS1307rtcRead( DS1307_rtc_t* DS1307_rtc );

int8_t DS1307rtcWrite( DS1307_rtc_t* DS1307_rtc );

uint8_t time_decimal(uint8_t bcdByte);

uint8_t mday_decimal(uint8_t bcdByte);

uint8_t month_decimal(uint8_t bcdByte);

uint8_t year_decimal(uint8_t bcdByte);

#ifdef __cplusplus
}
#endif


#endif /* GRABAR_SD_H */