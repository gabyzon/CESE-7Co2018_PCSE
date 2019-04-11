#ifndef _GRABAR_SD_H_
#define _GRABAR_SD_H_

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


void disk_timerproc( void );
void diskTickHook( void *ptr );
void grabar_sd( char *resultado , uint8_t largo );

#ifdef __cplusplus
}
#endif


#endif /* GRABAR_SD_H */