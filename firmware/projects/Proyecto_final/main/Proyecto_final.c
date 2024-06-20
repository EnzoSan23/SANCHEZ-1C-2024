/*! @mainpage Proyecto final
 *
 * \section genDesc General Description
 *
 * La aplicacion desarrollada permite obtener una determinada dosis de liquido 
 * solicitada por el usuario. Esto se realiza mediante el uso de una celda de carga
 * y un micro servo que controla la apertura y cierre de una valvula que determina
 * el flujo del liquido hacia el recipiento donde se acumula la dosis.
 * La aplicacion se desarrolla en la placa ESP32-C6-DevKitC-1 y utiliza una aplicacion
 * externa para el control de recepcion y envio de dato mediante bluetooth. 
 * La aplicacion "Bluetooth electronics" funciona como interfaz con el usuario.
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_NP	 	| 	GPIO_8		|
 * | 	PIN_CC	 	| 	GPIO_0		|
 * | 	PIN_MS   	| 	GPIO_9		|
 * | 	PIN_+5V	 	| 	+5V			|
 * | 	PIN_GND	 	| 	GND			|
 * 
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 15/05/2024 | Document creation		                         |
 *
 * @author Enzo Sanchez (enzosanchez237@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "analog_io_mcu.h"

//Dispositivos
#include "servo_sg90.h"
#include "neopixel_stripe.h"
#include "ble_mcu.h"
#include "led.h"
/*==================[macros and definitions]=================================*/
/** @def TASA_REFRESCO_LENTO
 *  @brief Constante que representa la tasa de refresco más lenta de los temporizadores
 * Está asociada al funcionamiento de la balanza. 
 * Dependerá de la velocidad de llenado del recipiente.
 */
#define TASA_REFRESCO_LENTO 200000
/** @def TASA_REFRESCO_RAPIDO
 *  @brief Constante que representa la tasa de refresco más rápida de los temporizadores
 * Está asociado al funcionamiento del motor.
 * Para un correcto control de las válvulas, debe ser al menos dos veces mas rápido que el 
 * temporizador relacionado a la celda de carga
 */        
#define TASA_REFRESCO_RAPIDO 100000

/*==================[internal data definition]===============================*/
TaskHandle_t balanza_task_handle = NULL;
TaskHandle_t valvula_task_handle = NULL;

/* variable donde se guarda el valor medido analogicamente con la celda de carga*/                
uint16_t pesoElectronico;           

/* variable que almacena la dosis establecida por el usuario (en ml)*/
uint32_t dosisRequerida;

/* variable donde se guarda el valor instantaneo calculado de dosis a partir del peso electronico*/
uint32_t dosisInstantanea = 0; 

/* variable logico que controla si la dosis fue seteada o no*/
bool dosis_seteada = false; 

/* variable logica que controla el estado de encendido de la aplicación*/
bool encendido = false;

/* contador usado para almacenar la dosis recibida por bluetooth*/
uint8_t i;

/* variable donde se almacenan los mensajes que se envian por bluetooth*/
char msg[30]; 
/*==================[internal functions declaration]=========================*/
/** @fn void  FuncTimerA(void* param)
 *  @brief Recibe la notificacion asociada al temporizador de la celda de carga 
 * 	@param 
 *  @return
*/
void FuncTimerA(void* param)
{
    xTaskNotifyGive(balanza_task_handle);
}

/** @fn void  FuncTimerB(void* param)
 *  @brief Recibe la notificacion asociada al temporizador del control valvular
 * 	@param 
 *  @return
*/
void FuncTimerB(void* param)
{
    xTaskNotifyGive(valvula_task_handle);
}

/** @fn void  read_data(uint8_t * data, uint8_t length)
 *  @brief Recibe y almacena la dosis enviada por el usuario. Al mismo momento 
 * cambia el estado de la variable dosis_seteada a true.
 * Además recibe información relacionado al estado de encendido/apagado de la aplicación
 * 	@param [data, length]
 *  @return
*/
void read_data(uint8_t * data, uint8_t length)
{
    if(data[0] == 'O')
    {
        encendido = true;
    }

    if(data[0] == 'o')
    {
        encendido = false;
    }
    
    if ((data[0] == 'V') & (!dosis_seteada))
    {
        dosisRequerida = 0;
        i = 1;
        while (data[i] != 'A')
        {
            dosisRequerida = dosisRequerida * 10;
			dosisRequerida = dosisRequerida + (data[i] - '0');
			i++;
        }
        dosis_seteada = true;
    }

}

/** @fn void  controlLEDs()
 *  @brief Controla el funcionamiento de la tira NeoPixel, encnediendo proceduralmente los 
 * diferentes LEDs con diferentes colores a medida que alcanza la dosis solicitada por el usuario
 * 	@param 
 *  @return
*/
void controlLEDs()
{
    if ((!dosis_seteada) | (dosisInstantanea == 0 ))
    {
        NeoPixelAllOff();
    }
    else if ((dosisInstantanea > 0) & (dosisInstantanea <= (dosisRequerida*0.125)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.125)) & (dosisInstantanea <= (dosisRequerida*0.25)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.25)) & (dosisInstantanea <= (dosisRequerida*0.375)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(2, NEOPIXEL_COLOR_ORANGE);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.375)) & (dosisInstantanea <= (dosisRequerida*0.5)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(2, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(3, NEOPIXEL_COLOR_ORANGE);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.5)) & (dosisInstantanea <= (dosisRequerida*0.625)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(2, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(3, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(4, NEOPIXEL_COLOR_YELLOW);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.625)) & (dosisInstantanea <= (dosisRequerida*0.75)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(2, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(3, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(4, NEOPIXEL_COLOR_YELLOW);
        NeoPixelSetPixel(5, NEOPIXEL_COLOR_YELLOW);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.75)) & (dosisInstantanea <= (dosisRequerida*0.875)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(2, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(3, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(4, NEOPIXEL_COLOR_YELLOW);
        NeoPixelSetPixel(5, NEOPIXEL_COLOR_YELLOW);
        NeoPixelSetPixel(6, NEOPIXEL_COLOR_GREEN);
    }
    else if ((dosisInstantanea > (dosisRequerida*0.875)) & (dosisInstantanea < (dosisRequerida)))
    {
        NeoPixelAllOff();
        NeoPixelSetPixel(0, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(1, NEOPIXEL_COLOR_RED);
        NeoPixelSetPixel(2, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(3, NEOPIXEL_COLOR_ORANGE);
        NeoPixelSetPixel(4, NEOPIXEL_COLOR_YELLOW);
        NeoPixelSetPixel(5, NEOPIXEL_COLOR_YELLOW);
        NeoPixelSetPixel(6, NEOPIXEL_COLOR_GREEN);
        NeoPixelSetPixel(7, NEOPIXEL_COLOR_GREEN);
    }
}

/** @fn static void pesar(void* param)
 *  @brief Tarea encargada de convertir el valor analogico medido por la celda de carga,
 * a un valor digital. Luego se lo convierte a su correspondiente de dosis.
 * Una vez la dosis instantanea alcanza a la dosis requerida, cambia el estado de 
 * dosis_seteada a false. 
 * 	@param 
 *  @return
*/
static void pesar(void* param)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (encendido)
        {
            AnalogInputReadSingle(CH0, &pesoElectronico);

            dosisInstantanea = -1*(pesoElectronico - 3477)/(0.74);      //Ecuacion con el recipiente tenido en cuenta       

            if (dosisInstantanea >= dosisRequerida)
            {
                dosis_seteada = false;
            }
            controlLEDs();
        }
    }
}

/** @fn static void controlValvular(void* param)
 *  @brief Tarea encargada de controlar la apertura y cierre de valvulas por el movimiento 
 * del micro servo de acuerdo al valor de dosis instantanea y su cercania a la dosis requerida.
 * Utilizando la UART, se puede chequear el estado de la válvula.
 * 	@param 
 *  @return
*/
static void controlValvular(void* param)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (encendido)
        {
            if ((dosis_seteada) & (dosisInstantanea <= dosisRequerida*0.5))
            {
                ServoMove(SERVO_0, 180);     //180 grados -> valvula abierta
                UartSendString(UART_PC, "Valvula abierta \r\n");
            }
            else if ((dosis_seteada) & (dosisInstantanea > dosisRequerida*0.5) & (dosisInstantanea <= dosisRequerida))
            {
                ServoMove(SERVO_0, 0);    //0 grados -> valvula medio cerrada
                UartSendString(UART_PC, "Valvula medio abierta \r\n");
            } 
            else if ((dosisInstantanea >= dosisRequerida) & (!dosis_seteada))
            {
                ServoMove(SERVO_0, 90);    //90 grados -> valvula cerrada
                UartSendString(UART_PC, "Valvula cerrada \r\n");
            } 
        }
           
    }
}

/*==================[external functions definition]==========================*/
void app_main(void){
    neopixel_color_t color;
    ble_config_t ble_configuration = {
        "Dosimetro",
        read_data
    };

    timer_config_t timer_balanza = {
        .timer = TIMER_A,
        .period = TASA_REFRESCO_LENTO,            //La tasa de refresco debe estar en microsegundos
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_balanza);

    timer_config_t timer_valvula = {
        .timer = TIMER_B,
        .period = TASA_REFRESCO_RAPIDO,            //La tasa de refresco debe estar en microsegundos
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_valvula);

    analog_input_config_t conversorAD = {			
	.input = CH0,			
	.mode = ADC_SINGLE,		
	.func_p = NULL,			
	.param_p = NULL,			
	.sample_frec = 0	
    };
    AnalogInputInit(&conversorAD);
    
    serial_config_t puerto_serie = {
        .port = UART_PC,
        .baud_rate = 115200,                //Velocidad de trabajo del puerto serie
        .func_p = NULL,
        .param_p = NULL
    };
    UartInit(&puerto_serie);

    ServoInit(SERVO_0, GPIO_9);
    LedsInit();
    BleInit(&ble_configuration);
    NeoPixelInit(BUILT_IN_RGB_LED_PIN, 8, &color);
    
    xTaskCreate(&pesar, "BALANZA", 512, NULL, 5, &balanza_task_handle);
    xTaskCreate(&controlValvular, "VALVULA", 512, NULL, 5, &valvula_task_handle);

    TimerStart(timer_balanza.timer);
    TimerStart(timer_valvula.timer);

    while (1)
    {
        switch(BleStatus())
        {
            case BLE_OFF:
                LedOff(LED_1);
                LedOff(LED_2);
            break;
            case BLE_DISCONNECTED:
                LedOff(LED_2);
                LedOn(LED_1);
            break;
            case BLE_CONNECTED:
                LedOff(LED_1);
                LedOn(LED_2);
            break;
        }

        if (encendido)
        {
            sprintf(msg, "*RDosis actual: %ld", dosisInstantanea);
            BleSendString(msg);
            sprintf(msg, "ml\n*");
            BleSendString(msg);
        }
         
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}
