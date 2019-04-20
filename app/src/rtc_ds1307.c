/*============================================================================
 * Autor:
 * Licencia:
 * Fecha:
 *===========================================================================*/



/*==================[internal data definition]===============================*/
#include "rtc_ds1307.h"
//MPU control structure
DS1307_rtc_t DS1307_rtc;


static char uartBuff[10];


// Función de inicialización del RTC, devuelve una variable bool para control de errores.
bool_t DS1307rtcInit( DS1307_rtc_t * DS1307_rtc )
{
    bool_t init;

	// using I2C for communication
	// starting the I2C bus
    
	init = i2cInit(I2C0, DS1307_I2C_RATE);
    // Si init = 1, el RTC se inició correctamente
    // Si init = -1, hubo un error en la iniciación del RTC
    return init;
}

// Función de lectura del RTC, devuelve una variable uint8 para control de errores.
int8_t DS1307rtcRead( DS1307_rtc_t * DS1307_rtc )
{
	static bool_t read;
    uint8_t dataToReadBuffer;
    uint8_t receiveDataBuffer;
    
    /*  La lectura se efectua en tres pasos
     *  1. Cargo en buffer la dirección del registro a leer
     *  2. Leo al buffer
     *  3. Grabo lo leido en la estructura DS1307_rtc            */
    
    dataToReadBuffer = DS1307_sec; 
    read = i2cRead( I2C0,DS1307_ADDRESS, &dataToReadBuffer,1,TRUE,&receiveDataBuffer,1,TRUE);
    DS1307_rtc->sec = receiveDataBuffer; 
    if(read<1){return -1;} // Si retorna -1, hay error en la lectura de los segundos.
    
	dataToReadBuffer = DS1307_min; 
    read = i2cRead( I2C0,DS1307_ADDRESS, &dataToReadBuffer,1,TRUE,&receiveDataBuffer,1,TRUE);
    DS1307_rtc->min = receiveDataBuffer; 
    if(read<1){return -2;} // Si retorna -2, hay error en la lectura de los minutos.

	dataToReadBuffer = DS1307_hour; 
    read = i2cRead( I2C0,DS1307_ADDRESS, &dataToReadBuffer,1,TRUE,&receiveDataBuffer,1,TRUE);
    DS1307_rtc->hour = receiveDataBuffer; 
    if(read<1){return -3;} // Si retorna -3, hay error en la lectura de la hora.

	dataToReadBuffer = DS1307_wday; 
    read = i2cRead( I2C0,DS1307_ADDRESS, &dataToReadBuffer,1,TRUE,&receiveDataBuffer,1,TRUE);
    DS1307_rtc->wday = receiveDataBuffer; 
    if(read<1){return -4;} // Si retorna -4, hay error en la lectura del dia de la semana.

	dataToReadBuffer = DS1307_mday; 
    read = i2cRead( I2C0,DS1307_ADDRESS, &dataToReadBuffer,1,TRUE,&receiveDataBuffer,1,TRUE);
    DS1307_rtc->mday = receiveDataBuffer; 
    if(read<1){return -5;} // Si retorna -5, hay error en la lectura del dia del mes.

	dataToReadBuffer = DS1307_month; 
    read = i2cRead( I2C0,DS1307_ADDRESS, &dataToReadBuffer,1,TRUE,&receiveDataBuffer,1,TRUE);
    DS1307_rtc->month = receiveDataBuffer; 
    if(read<1){return -6;} // Si retorna -6, hay error en la lectura del mes.

	dataToReadBuffer = DS1307_year; 
    read = i2cRead( I2C0,DS1307_ADDRESS, &dataToReadBuffer,1,TRUE,&receiveDataBuffer,1,TRUE);
    DS1307_rtc->year = receiveDataBuffer; 
    if(read<1){return -7;} // Si retorna -7, hay error en la lectura del año.

   
    return 1; // Si retorna 1, se completó correctamente toda la lectura.
}

// Función de escritura del RTC, devuelve una variable uint8 para control de errores.
int8_t DS1307rtcWrite( DS1307_rtc_t * DS1307_rtc )
{
    static bool_t write;
    uint8_t transmitDataBuffer[2];

    /*  La lectura se efectua en tres pasos
     *  1. Cargo en buffer[0] la dirección del registro a 
     *  2. Cargo en el buffer[1] la información a escribir
     *  3. Grabo en el registro buffer[0] la información de buffer[1]       */
    
	transmitDataBuffer[0] = DS1307_sec;
	transmitDataBuffer[1] = DS1307_rtc->sec;
    write = i2cWrite(I2C0, DS1307_ADDRESS, transmitDataBuffer, 2, TRUE);
    if(write<1){return -1;}  // Si retorna -1, hay error en la escritura de los segundos.

	transmitDataBuffer[0] = DS1307_min;
	transmitDataBuffer[1] = DS1307_rtc->min;
    write = i2cWrite(I2C0, DS1307_ADDRESS, transmitDataBuffer, 2, TRUE);
    if(write<1){return -2;}  // Si retorna -2, hay error en la escritura de los minutos.

	transmitDataBuffer[0] = DS1307_hour;
	transmitDataBuffer[1] = DS1307_rtc->hour;
    write = i2cWrite(I2C0, DS1307_ADDRESS, transmitDataBuffer, 2, TRUE);
    if(write<1){return -3;}  // Si retorna -3, hay error en la escritura de la hora.
    
    transmitDataBuffer[0] = DS1307_wday;
	transmitDataBuffer[1] = DS1307_rtc->wday;
    write = i2cWrite(I2C0, DS1307_ADDRESS, transmitDataBuffer, 2, TRUE);
    if(write<1){return -4;}  // Si retorna -4, hay error en la escritura del dia de la semana.
    
    transmitDataBuffer[0] = DS1307_mday;
	transmitDataBuffer[1] = DS1307_rtc->mday;
    write = i2cWrite(I2C0, DS1307_ADDRESS, transmitDataBuffer, 2, TRUE);
    if(write<1){return -5;}  // Si retorna -5, hay error en la escritura del dia de mes.
    
    transmitDataBuffer[0] = DS1307_month;
	transmitDataBuffer[1] = DS1307_rtc->month;
    write = i2cWrite(I2C0, DS1307_ADDRESS, transmitDataBuffer, 2, TRUE);
    if(write<1){return -6;}  // Si retorna -6, hay error en la escritura del mes.
    
    transmitDataBuffer[0] = DS1307_year;
	transmitDataBuffer[1] = DS1307_rtc->year;
    write = i2cWrite(I2C0, DS1307_ADDRESS, transmitDataBuffer, 2, TRUE);
    if(write<1){return -7;} // Si retorna -7, hay error en la escritura del año.
    
    //DS1307_ADDRESS_0 = 0xD1; // Cambio a modo lectura
    
    return 1;

}

// Conversión de BCD a decimal para los registros de hora, minutos y segundos
// Toma los 4 bits menos significativos y suma los bits 4 5 y 6 multiplicados por 10. 
// Descarta el bit mas significativo.
uint8_t time_decimal(uint8_t bcdByte)
{
    return (  ( ((bcdByte & 0x70) >> 4)) * 10) + (bcdByte & 0x0F);
}       

// Conversión de BCD a decimal para el registro del dia del mes
// Toma los 4 bits menos significativos y suma los bits 4 y 5 multiplicados por 10. 
// Descarta los dos bits mas significativo.
uint8_t mday_decimal(uint8_t bcdByte)
{
    return (  ( ((bcdByte & 0x30) >> 4)) * 10) + (bcdByte & 0x0F);
}      

// Conversión de BCD a decimal para el registro del mes
// Toma los 4 bits menos significativos y suma el bit 4 multiplicado por 10. 
// Descarta los tres bits mas significativo.
uint8_t month_decimal(uint8_t bcdByte)
{
    return (  ( ((bcdByte & 0x10) >> 4)) * 10) + (bcdByte & 0x0F);
}     

// Conversión de BCD a decimal para el registro del mes
// Toma los 4 bits menos significativos y suma los 4 bits mas significativos multiplicados por 10
// Descarta los tres bits mas significativo.
uint8_t year_decimal(uint8_t bcdByte)
{
    return (  ( ((bcdByte & 0xF0) >> 4)) * 10) + (bcdByte & 0x0F);
}