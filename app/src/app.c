/*============================================================================
 * Autor: Alvaro Gabriel Pizá
 * Licencia:
 * Fecha: 
 *===========================================================================*/

/*==================[inclusions]=============================================*/
#include "sapi.h"     // <= sAPI header
#include <string.h>
#include "app.h"
#include "MEFbutton_refresh.h"
#include "grabar_sd.h"
#include "ff.h"       // <= Biblioteca FAT FS
#include "fssdc.h"    // API de bajo nivel para unidad "SDC:" en FAT FS
#include "rtc_ds1307.h"


/*==================[macros and definitions]=================================*/
#define UART_SELECTED   UART_USB

MEFbutton_t MEFbutton1, MEFbutton2, MEFbutton3, MEFbutton4; // Defino las maquinas de estados para los botones
MEFpantalla_t MEFpantalla; // Defino la maquina de estados para el menu

/*==================[internal data declaration]==============================*/
CONSOLE_PRINT_ENABLE

/*==================[external data definition]===============================*/
static char uartBuff[10]; // Buffer



/*==================[external functions definition]==========================*/
char* itoa(int value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	// Apply negative sign
	if (tmp_value < 0) *ptr++ = '-';
	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}

/*==================[Functions prototipes]==========================*/
void error_state( void );
void init_state( void );
void main_menu(void);
void cronometro(void);
void configura(void);
void showDateAndTime( DS1307_rtc_t * rtc );

/*==================[Global variables]==========================*/
/* Estructura RTC */
DS1307_rtc_t rtc;
bool_t val = 0;
uint8_t i = 0;

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main( void )
{
    // ---------- CONFIGURACIONES ------------------------------

    // Inicializar y configurar la plataforma

    /* Inicializar la placa */
	boardConfig();
    /* Inicializar UART_USB como salida de consola */
    consolePrintConfigUart( UART_USB, 115200 );

    /* Estructura RTC */
    bool_t val = 0;
    uint8_t i = 0;

    /* Inicializar LCD de 20x4 (caracteres x lineas) con cada caracter de 5x8 pixeles */
    lcdInit( 20, 4, 5, 8 );

    /* Inicializar RTC */
    val = DS1307rtcInit( &rtc );
    
    // Configuro el estado inicial de las maquinas de estado
    init_state();

    // ---------- REPETIR POR SIEMPRE --------------------------
    while( TRUE ) 
    {
        main_menu();
    }

    // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
    // directamenteno sobre un microcontroladore y no es llamado por ningun
    // Sistema Operativo, como en el caso de un programa para PC.
    return 0;
}


// Menú principal

void main_menu(void)
{
    lcdClear(); // Borrar la pantalla
    lcdGoToXY( 1, 1 ); // Poner cursor en 1, 1 (columna,fila)
    lcdSendStringRaw( "1- Cronometro" );  
    lcdGoToXY( 1, 2 ); // Poner cursor en 1, 2 (columna,fila)
    lcdSendStringRaw( "2- Configuracion" );
    
    while(MEFpantalla == m_principal)
    {
        MEFbutton_refresh(TEC1, &MEFbutton1);
        MEFbutton_refresh(TEC2, &MEFbutton2);
    
        // Si presiono el botón 1 paso al estado cronómetro
        if(  (MEFbutton1 == B_desc  && !gpioRead(TEC1)) || MEFpantalla == m_cronom)
        {
            MEFpantalla = m_cronom;
            delay(750);
            cronometro();
        }
        
        // Si presiono el botón 2 paso al estado configuración
        if(  (MEFbutton2 == B_desc && !gpioRead(TEC2)) || MEFpantalla == m_config )
        {
            MEFpantalla = m_config;
            delay(750);
            configura();
        }
    
    }
    return;
}

// Función de cronómetro
void cronometro(void)
{
    /* Defino las variables locales del contador */
    uint8_t ds = 0;
    uint8_t cs = 0;
    uint8_t ss = 0;
    uint8_t mm = 0;
    uint8_t hh = 0;
    int8_t lector; // variable para el control de error del lector
    char resultado[] = "gigolo!";

    lcdClear(); // Borrar la pantalla
    lcdGoToXY( 1, 1 ); // Poner cursor en 1, 1 (columna,fila)
    lcdSendStringRaw( "Cronometro" );  
    lcdGoToXY( 1, 2 ); // Poner cursor en 1, 2 (columna,fila)
    lcdSendStringRaw( "1- Iniciar" );
    lcdGoToXY( 1, 3 ); // Poner cursor en 1, 3 (columna,fila)
    lcdSendStringRaw( "4- Volver" );
    
    while(MEFpantalla == m_cronom)
    {
        MEFbutton_refresh(TEC1, &MEFbutton1);
        MEFbutton_refresh(TEC4, &MEFbutton4);

        // Si presiono el botón 1 inicio el contador del cronómetro
        if(MEFbutton1 == B_desc  && !gpioRead(TEC1))
        {
            MEFpantalla = p_contador;
            lcdClear(); // Borrar la pantalla
            lcdGoToXY( 1, 1 ); // Poner cursor en 1, 1 (columna,fila)
            lcdSendStringRaw( "Cronometro:" );
            lcdGoToXY( 1, 4 ); // Poner cursor en 1, 4 (columna,fila)
            lcdSendStringRaw( "3- Detener" );
            
            while(MEFpantalla == p_contador)
            {
                if(ds == 10) {cs++, ds = 0;}
                if(cs == 10) {ss++, cs = 0;}
                if(ss == 60) {mm++, ss = 0;}
                if(mm == 60) {hh++, mm = 0;}
            
                                itoa( (int) (hh), (char*)uartBuff, 10 ); /* 10 significa decimal */
                                lcdGoToXY( 1, 2 ); // Poner cursor en 1, 2 (columna,fila)
                            	if( (hh)<10 )
                                {
                                    lcdSendStringRaw( "0" );  
                                }
                                lcdSendStringRaw( uartBuff );
                                
                                itoa( (int) (mm), (char*)uartBuff, 10 ); /* 10 significa decimal */
                                lcdGoToXY( 3, 2 ); // Poner cursor en 3, 2 (columna,fila)
                                lcdSendStringRaw( ":" );
                            	if( (mm)<10 )
                                {
                                    lcdSendStringRaw( "0" );  
                                }
                                lcdSendStringRaw( uartBuff );
                                
                                itoa( (int) (ss), (char*)uartBuff, 10 ); /* 10 significa decimal */
                                lcdGoToXY( 6, 2 ); // Poner cursor en 6, 2 (columna,fila)
                                lcdSendStringRaw( ":" );
                            	if( (ss)<10 )
                                {
                                    lcdSendStringRaw( "0" );  
                                }
                                lcdSendStringRaw( uartBuff );
                                lcdGoToXY( 9, 2 ); // Poner cursor en 9, 2 (columna,fila)
                                lcdSendStringRaw( "." );

                                itoa( (int) (cs), (char*)uartBuff, 10 ); /* 10 significa decimal */
                                lcdSendStringRaw( uartBuff );
                                itoa( (int) (ds), (char*)uartBuff, 10 ); /* 10 significa decimal */
                                lcdSendStringRaw( uartBuff );

                
                MEFbutton_refresh(TEC3, &MEFbutton3);
                if(MEFbutton3 == B_desc  && !gpioRead(TEC3))
                {
                    lcdGoToXY( 1, 4 ); // Poner cursor en 1, 1 (columna,fila)
                    lcdSendStringRaw( "4- Guardar " );
                    
                    /* Leer fecha y hora */
                    lector = DS1307rtcRead( &rtc );
                    /* Acá debo hacer el control de errores*/

                    // Creo un char con el tiempo cronometrado, la fecha y la hora del registro
                    sprintf(resultado, "%d:%d:%d.%d%d    %d/%d/%d    %d:%d:%d" , hh, mm, ss, cs, ds, mday_decimal(rtc.mday), month_decimal(rtc.month), year_decimal(rtc.year), time_decimal(rtc.hour), time_decimal(rtc.min), time_decimal(rtc.sec));
                    uint8_t largo = strlen(resultado); // mido la longitud final del char
                    while(TRUE)
                    {
                        MEFbutton_refresh(TEC4, &MEFbutton4);
                        if(MEFbutton4 == B_desc  && !gpioRead(TEC4))
                        {
                            
                            grabar_sd(resultado, largo); // envio resultado y su largo
                            MEFpantalla = m_principal;
                            main();
                        }
                    }
                }
                ds++;
                delay(10); 
            }
        }
        
        // Si presiono el botón 4 vuelvo al estado principal
        if(MEFbutton4 == B_desc  && !gpioRead(TEC4))
        {
            MEFpantalla = m_principal;
            delay(250);
            main_menu();
        }
    }
    return;
}

// Función de configuracion
void configura(void)
{
    int8_t lector; // variable para el control de error del rtc
    
    lcdClear(); // Borrar la pantalla
    lcdGoToXY( 1, 1 ); // Poner cursor en 1, 1 (columna,fila)
    lcdSendStringRaw( "1- Ver hora" ); 
    lcdGoToXY( 1, 2 ); // Poner cursor en 1, 1 (columna,fila)
    lcdSendStringRaw( "2- Configurar hora" );  
    lcdGoToXY( 1, 3 ); // Poner cursor en 1, 2 (columna,fila)
    lcdSendStringRaw( "4- Volver" );
    while(MEFpantalla == m_config)
    {
        MEFbutton_refresh(TEC1, &MEFbutton1);
        MEFbutton_refresh(TEC2, &MEFbutton2);
        MEFbutton_refresh(TEC4, &MEFbutton4);

        // Si presiono el botón 1 presentamos la hora en pantalla
        if(MEFbutton1 == B_desc && !gpioRead(TEC1))
        {
            lcdClear(); // Borrar la pantalla	
            for( i=0; i<10; i++ ){
                
                /* Titulo de pantalla */
                lcdGoToXY( 4, 1 ); // Poner cursor en 4, 1 (columna,fila)
                lcdSendStringRaw( "Fecha y Hora" ); 
                
                /* Leer fecha y hora */
                lector = DS1307rtcRead( &rtc );
                
                /* Acá debo agregar el control de error*/
                
                
                /* Mostrar fecha y hora en formato "DD/MM/YYYY, HH:MM:SS" */
                showDateAndTime( &rtc );
                delay(1000);
            }
            
            i=0;
            lcdClear(); // Borrar la pantalla
            lcdGoToXY( 1, 1 ); // Poner cursor en 1, 1 (columna,fila)
            lcdSendStringRaw( "1- Ver hora" ); 
            lcdGoToXY( 1, 2 ); // Poner cursor en 1, 1 (columna,fila)
            lcdSendStringRaw( "2- Configurar hora" );  
            lcdGoToXY( 1, 3 ); // Poner cursor en 1, 2 (columna,fila)
            lcdSendStringRaw( "4- Volver" );
        }
        
        // Si presiono el botón 2 para setear la fecha y hora preestablecida
        if(MEFbutton2 == B_desc && !gpioRead(TEC2))
        {
            /* Titulo de pantalla */
            lcdClear();
            lcdGoToXY( 3, 2 ); // Poner cursor en 4, 1 (columna,fila)
            lcdSendStringRaw( "Configurando..." );   
            
            // Cargo los valores de reloj deseados, esta vez lo hacemos a mano
            rtc.year = 0b00011000; // año 18
            rtc.month = 0b00010010; // mes 12
            rtc.mday = 0b00110001; // dia 31
            rtc.wday = 1;
            rtc.hour = 0b00010011; // 13 horas 
            rtc.min = 0b01011001; // 59 minutos
            rtc.sec= 0b01000101; // 45 segundos 
            
            /* Establecer fecha y hora */
            // Enviamos los valores cargados al RTC
            lector = DS1307rtcWrite( &rtc );
            /* Acá debo hacer el control de error*/
            
            delay(1000);
            
            lcdClear(); // Borrar la pantalla
            lcdGoToXY( 1, 1 ); // Poner cursor en 1, 1 (columna,fila)
            lcdSendStringRaw( "1- Ver hora" ); 
            lcdGoToXY( 1, 2 ); // Poner cursor en 1, 1 (columna,fila)
            lcdSendStringRaw( "2- Configurar hora" );  
            lcdGoToXY( 1, 3 ); // Poner cursor en 1, 2 (columna,fila)
            lcdSendStringRaw( "4- Volver" );            
        }        
        
        
        
        // Si presiono el botón 4 vuelvo al estado principal
        if(MEFbutton4 == B_desc && !gpioRead(TEC4))
        {
            MEFpantalla = m_principal;
            delay(250);
            main_menu();
        }
    }
    return;
}


// Configuro el estado inicial de las maquinas de estado
void init_state( void )
{
	MEFbutton1 = B_arriba;
	MEFbutton2 = B_arriba;
	MEFbutton3 = B_arriba;
	MEFbutton4 = B_arriba;
	MEFpantalla = m_principal;
	gpioWrite(LEDB, OFF);
	gpioWrite(LEDR, OFF);
	gpioWrite(LEDG, OFF);
	gpioWrite(LED1, OFF);
	gpioWrite(LED2, OFF);
	gpioWrite(LED3, OFF);
    
    return;
}

// Configuro el estado de error en las maquinas de estado
void error_state( void )
{
	MEFbutton1 = B_arriba;
	MEFbutton2 = B_arriba;
	MEFbutton3 = B_arriba;
	MEFbutton4 = B_arriba;
    MEFpantalla = m_principal;
	gpioWrite(LEDB, OFF);
	gpioWrite(LEDR, ON);
    delay(1000);
    gpioWrite(LEDR, OFF);
	gpioWrite(LEDG, OFF);
	gpioWrite(LED1, OFF);
	gpioWrite(LED2, OFF);
	gpioWrite(LED3, OFF);
    
    return;
}

/* Enviar fecha y hora en formato "DD/MM/YYYY, HH:MM:SS" */
void showDateAndTime( DS1307_rtc_t * rtc ){
	
    int decimal;
    char sec_d[10], min_d[10], hour_d[10], mday_d[10], month_d[10], year_d[10];
    /* Conversion de entero a ascii con base decimal */
	decimal = 0;
    decimal = mday_decimal(rtc->mday); // Descompone el int8 en 2 partes y lo convierte a decimal
    sprintf(mday_d, "%d", decimal);// Convertimos el resultado en un char
      
    /* Envio el dia */
    lcdGoToXY( 1, 3 ); // Poner cursor en 1, 1 (columna,fila)
	if( (decimal)<10 )
    {
        lcdSendStringRaw( "0" );  
    }
    lcdSendStringRaw( mday_d );
    lcdGoToXY( 3, 3 ); // Poner cursor en 3, 1 (columna,fila)
    lcdSendStringRaw( "/" );
    
	/* Conversion de entero a ascii con base decimal */
    decimal = 0;
    decimal = month_decimal(rtc->month); // Descompone el int8 en 2 partes y lo convierte a decimal
    sprintf(month_d, "%d", decimal);// Convertimos el resultado en un char
    
    /* Envio el mes */
    lcdGoToXY( 4, 3 ); // Poner cursor en 1, 4
	if( (decimal)<10 )
    {
        lcdSendStringRaw( "0" );  
    }
    lcdSendStringRaw( month_d );
    lcdGoToXY( 6, 3 ); // Poner cursor en 6, 3 (columna,fila)
    lcdSendStringRaw( "/" );
    
	/* Conversion de entero (rtc.year) a ascii con base decimal (uartBuff) */
    decimal = 0;
    decimal = year_decimal(rtc->year); // Descompone el int8 en 2 partes y lo convierte a decimal
    sprintf(year_d, "%d", decimal);// Convertimos el resultado en un char
    
    /* Envio el año    */
    lcdGoToXY( 7, 3 ); // Poner cursor en 7, 3
	if( (decimal)<10 )
    {
        lcdSendStringRaw( "0" );  
    }
    lcdSendStringRaw( year_d );
    lcdGoToXY( 11, 3 ); // Poner cursor en 1, 1 (columna,fila)
    lcdSendStringRaw( "," );

	/* Conversion de entero a ascii con base decimal */
    decimal = 0;
    decimal = time_decimal(rtc->hour); // Descompone el int8 en 2 partes y lo convierte a decimal
    sprintf(hour_d, "%d", decimal);// Convertimos el resultado en un char
    
    /* Envio la hora */
    lcdGoToXY( 12, 3 ); // Poner cursor en 1, 1
	if( (decimal)<10 )
    {
        lcdSendStringRaw( "0" );  
    }
    lcdSendStringRaw( hour_d );
    lcdGoToXY( 14, 3 ); // Poner cursor en 14, 3 (columna,fila)
    lcdSendStringRaw( ":" );

	/* Conversion de entero a ascii con base decimal */
    decimal = 0;
    decimal = time_decimal(rtc->min); // Descompone el int8 en 2 partes y lo convierte a decimal
    sprintf(min_d, "%d", decimal);// Convertimos el resultado en un char
    
    /* Envio los minutos */
	lcdGoToXY( 15, 3 ); // Poner cursor en 15, 3
    if( (decimal)<10 )
    {
        lcdSendStringRaw( "0" );    
    }
    lcdSendStringRaw( min_d );
    lcdGoToXY( 17, 3 ); // Poner cursor en 17, 3 (columna,fila)
    lcdSendStringRaw( ":" );

	/* Conversion de entero a ascii con base decimal */
    decimal = 0;
    decimal = time_decimal(rtc->sec); // Descompone el int8 en 2 partes y lo convierte a decimal
    sprintf(sec_d, "%d", decimal);// Convertimos el resultado en un char
    
    /* Envio los segundos */
	lcdGoToXY( 18, 3 ); // Poner cursor en 18, 3
    if( (decimal)<10 )
    {
        lcdSendStringRaw( "0" );
    }
    lcdSendStringRaw( sec_d );
}
