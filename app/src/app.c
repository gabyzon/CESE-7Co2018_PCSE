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
void showDateAndTime( rtc_t * rtc );

/*==================[Global variables]==========================*/
/* Estructura RTC */
rtc_t rtc;
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
	rtc_t rtc;
    bool_t val = 0;
    uint8_t i = 0;

    /* Inicializar LCD de 20x4 (caracteres x lineas) con cada caracter de 5x8 pixeles */
    lcdInit( 20, 4, 5, 8 );

    /* Inicializar RTC */
	val = rtcConfig( &rtc );
	delay_t delay1s;
	delayConfig( &delay1s, 1000 );
	delay(2000); // El RTC tarda en setear la hora, por eso el delay
    
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
                    // ACA TENGO QUE CARGAR AL CHAR "resultado" el resultado con fecha y hora"
                    /* Leer fecha y hora */
                    val = rtcRead( &rtc );
                    sprintf(resultado, "%d:%d:%d.%d%d    %d/%d/%d    %d:%d:%d" , hh, mm, ss, cs, ds, rtc.mday, rtc.month, rtc.year, rtc.hour, rtc.min, rtc.sec);
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
                val = rtcRead( &rtc );
                
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
            rtc.year = 2018;
            rtc.month = 3;
            rtc.mday = 6;
            rtc.wday = 1;
            rtc.hour = 23;
            rtc.min = 59;
            rtc.sec= 45;
            
            /* Establecer fecha y hora */
            val = rtcWrite( &rtc );
            
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
void showDateAndTime( rtc_t * rtc ){
	
    /* Conversion de entero a ascii con base decimal */
	itoa( (int) (rtc->mday), (char*)uartBuff, 10 ); /* 10 significa decimal */
	/* Envio el dia */
    lcdGoToXY( 1, 3 ); // Poner cursor en 1, 1 (columna,fila)
	if( (rtc->mday)<10 )
    {
        lcdSendStringRaw( "0" );  
    }
    lcdSendStringRaw( uartBuff );
    lcdGoToXY( 3, 3 ); // Poner cursor en 3, 1 (columna,fila)
    lcdSendStringRaw( "/" );
    
	/* Conversion de entero a ascii con base decimal */
	itoa( (int) (rtc->month), (char*)uartBuff, 10 ); /* 10 significa decimal */
	/* Envio el mes */
    lcdGoToXY( 4, 3 ); // Poner cursor en 1, 4
	if( (rtc->month)<10 )
    {
        lcdSendStringRaw( "0" );  
    }
    lcdSendStringRaw( uartBuff );
    lcdGoToXY( 6, 3 ); // Poner cursor en 6, 1 (columna,fila)
    lcdSendStringRaw( "/" );
    
	/* Conversion de entero (rtc.year) a ascii con base decimal (uartBuff) */
	itoa( (int) (rtc->year), (char*)uartBuff, 10 ); /* 10 significa decimal */
	/* Envio el año    */
    lcdGoToXY( 7, 3 ); // Poner cursor en 1, 1
	if( (rtc->year)<10 )
    {
        lcdSendStringRaw( "0" );  
    }
    lcdSendStringRaw( uartBuff );
    lcdGoToXY( 11, 3 ); // Poner cursor en 1, 1 (columna,fila)
    lcdSendStringRaw( "," );

	/* Conversion de entero a ascii con base decimal */
	itoa( (int) (rtc->hour), (char*)uartBuff, 10 ); /* 10 significa decimal */
	/* Envio la hora */
    lcdGoToXY( 12, 3 ); // Poner cursor en 1, 1
	if( (rtc->hour)<10 )
    {
        lcdSendStringRaw( "0" );  
    }
    lcdSendStringRaw( uartBuff );
    lcdGoToXY( 14, 3 ); // Poner cursor en 1, 1 (columna,fila)
    lcdSendStringRaw( ":" );

	/* Conversion de entero a ascii con base decimal */
	itoa( (int) (rtc->min), (char*)uartBuff, 10 ); /* 10 significa decimal */
	/* Envio los minutos */
	lcdGoToXY( 15, 3 ); // Poner cursor en 1, 1
    if( (rtc->min)<10 )
    {
        lcdSendStringRaw( "0" );    
    }
    lcdSendStringRaw( uartBuff );
    lcdGoToXY( 17, 3 ); // Poner cursor en 1, 1 (columna,fila)
    lcdSendStringRaw( ":" );

	/* Conversion de entero a ascii con base decimal */
	itoa( (int) (rtc->sec), (char*)uartBuff, 10 ); /* 10 significa decimal */
	/* Envio los segundos */
	lcdGoToXY( 18, 3 ); // Poner cursor en 1, 1
    if( (rtc->sec)<10 )
    {
        lcdSendStringRaw( "0" );
    }
    lcdSendStringRaw( uartBuff );
}
