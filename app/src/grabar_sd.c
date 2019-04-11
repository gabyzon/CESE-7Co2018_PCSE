
/*==================[inlcusiones]============================================*/

#include "grabar_sd.h"   // <= own header

//#include "ff.h"       // <= Biblioteca FAT FS
//#include "fssdc.h"    // API de bajo nivel para unidad "SDC:" en FAT FS

/*==================[definiciones y macros]==================================*/

#define FILENAME "SDC:/cronometro.txt"

/*==================[definiciones de datos internos]=========================*/

/*==================[definiciones de datos externos]=========================*/

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

// FUNCION que se ejecuta cada vezque ocurre un Tick
void diskTickHook( void *ptr );


/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA A LA FUNCION //
void grabar_sd( char *resultado, uint8_t largo ){

    static FATFS fs;           // <-- FatFs work area needed for each volume
    static FIL fp;             // <-- File object needed for each open file
    
    // SPI configuration
    spiConfig( SPI0 );

    // Inicializar el conteo de Ticks con resolucion de 10ms,
    // con tickHook diskTickHook
    tickConfig( 10 );
    tickCallbackSet( diskTickHook, NULL );

    // ------ PROGRAMA QUE ESCRIBE EN LA SD -------

    UINT nbytes;

    // Initialize SD card driver
    FSSDC_InitSPI();
    // Give a work area to the default drive
    if( f_mount( &fs, "SDC:", 0 ) != FR_OK ) {
        // If this fails, it means that the function could
        // not register a file system object.
        // Check whether the SD card is correctly connected
        gpioWrite( LEDB, ON );
        delay(1000);
        gpioWrite( LEDB, OFF );

    }

    // Create/open a file, then write a string and close it
    if( f_open( &fp, FILENAME, FA_WRITE | FA_OPEN_APPEND ) == FR_OK )
    {
        f_write( &fp, resultado, largo, &nbytes );
        f_write( &fp, "\r\n", 2, &nbytes );
        f_close(&fp);

        if( nbytes == 2 ){
            // Turn ON LEDG if the write operation was successful
            gpioWrite( LEDG, ON );
            delay(1000);
            gpioWrite( LEDG, OFF );
        }
    }
    else
    {
        // Turn ON LEDR if the write operation was fail
        gpioWrite( LEDR, ON );
        delay(1000);
        gpioWrite( LEDR, OFF );
    }

    return;
}

/*==================[definiciones de funciones internas]=====================*/

/*==================[definiciones de funciones externas]=====================*/

// FUNCION que se ejecuta cada vezque ocurre un Tick
void diskTickHook( void *ptr ){
   disk_timerproc();   // Disk timer process
}

/*==================[fin del archivo]========================================*/