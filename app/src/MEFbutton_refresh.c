// Función de actualización de MEF de las teclas
#include "MEFbutton_refresh.h"
MEFbutton_t MEFbutton1, MEFbutton2, MEFbutton3, MEFbutton4; // Defino las maquinas de estados para los botones

void MEFbutton_refresh( gpioMap_t tecla , MEFbutton_t *estado)
{
   static bool_t flagFalling = FALSE;
   static bool_t flagRising = FALSE;
   
   static uint8_t contFalling = 0;
   static uint8_t contRising = 0;


   switch (*estado) {

   	   case B_arriba:{
   		   // si la tecla esta presionada, cambio el estado a descendente
   		   if (!gpioRead( tecla )) {
   			   *estado = B_desc;
   		   }
		}
   	   break;

   	   case B_abajo:{
   		   // si la tecla esta liberada, cambio el estado a ascendente
   		   if (gpioRead( tecla )) {
   			   *estado = B_asc;
   		   }
   	   }
   	   break;

   	   case B_desc:{
           /* ENTRADA - si no esta activo el flag, lo activo*/
   		   if( flagFalling == FALSE ){
              flagFalling = TRUE; // abajo de esto ejecuto la funcion del flag descendente
              /* ejecuto las tareas correspondiente al flanco descendente */

           }
           
           /* CHECK TRANSITION CONDITIONS */
           if( contFalling >= 40 ){
              if( !gpioRead(tecla) ){
                 *estado = B_abajo;
              }
              else{
                 *estado = B_arriba;
              }
              contFalling = 0;
           }
           contFalling++;
           /*SALIDA. Si el estado ya no es B_desc, coloco en False el flag */
           if( *estado != B_desc ){
              flagFalling = FALSE;
              //gpioWrite(LED1, OFF);
           }
   	   }
   	   break;

        case B_asc:
            /* ENTRY */
            if( flagRising == FALSE ){
                flagRising = TRUE; // abajo de esto ejecuto la funcion del flag ascendente
                /* ejecuto las tareas correspondiente al flanco ascendente.*/
            }
           
            /* CHECK TRANSITION CONDITIONS */
            if( contRising >= 40 ){
                if( gpioRead(tecla) ){
                    *estado = B_arriba;
                } else{
                    *estado = B_abajo;
                }
                contRising = 0;
            }
            contRising++;

            /* SALIDA */
            if( *estado != B_asc ){
                flagRising = FALSE;
                //gpioWrite(LED2, OFF);
            }
        break;

        default:
            MEFbutton1 = B_arriba;
            MEFbutton2 = B_arriba;
            MEFbutton3 = B_arriba;
            MEFbutton4 = B_arriba;
        break;
    }
    return;
}