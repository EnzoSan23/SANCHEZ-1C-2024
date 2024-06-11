/*! @mainpage Examen
 *
 * @section genDesc General Description
 *
 * La aplicacion responde a lo solicitado en el enunciado del examen
 * https://docs.google.com/document/d/1x-aZ4DRUakD6gpZ-Z-qbwt8ypvhMQ5MbhMv4pTEjDbk/edit
 * Esto se realiza mediante el uso de diferntes GPIO de la placa ESP32-C6-DevKitC-1.
 * 
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_B1	 	| 	GPIO_20		|
 * | 	PIN_B2	 	| 	GPIO_21		|
 * | 	PIN_B3	 	| 	GPIO_22		|
 * | 	PIN_S1	 	| 	GPIO_23		|
 * | 	PIN_S2		| 	GPIO_1		|
 * | 	PIN_+3V	 	| 	+3V			|
 * | 	PIN_GND	 	| 	GND			|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/06/2024 | Document creation		                         |
 *
 * @author Enzo Sanchez (enzosanchez237@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "analog_io_mcu.h"
#include "gpio_mcu.h"
#include "switch.h"

/*==================[macros and definitions]=================================*/
/** @def TASA_REFRESCO
 *  @brief Constante que representa la tasa de refresco más rápida de los temporizadores
 */
#define TASA_REFRESCO 3000000		//3 segundos de temporizador
/** @def TASA_REFRESCO_LENTA
 *  @brief Constante que representa la tasa de refresco más lenta de los temporizadores
 */
#define TASA_REFRESCO_LENTA 5000000	//5 segundos de temporizador

/** @def gpioConf_t
 *  @brief Estructura que define la configuracion de un GPIO
 */
typedef struct
{
	gpio_t pin; 		/*!< GPIO pin number */
	io_t dir; 			/*!< GPIO direction '0' IN; '1' OUT*/
} gpioConf_t;
/*==================[internal data definition]===============================*/
TaskHandle_t humedad_task_handle = NULL;
TaskHandle_t ph_task_handle = NULL;
TaskHandle_t uart_task_handle = NULL;

/** @def vecBombas
 *  @brief vector que asocia cada bomba a utilizar a un determinado GPIO
 * GPIO_20 -> bombaAgua
 * GPIO_21 -> bombaBasica
 * GPIO_22 -> bombaAcida
 */
gpioConf_t vecBombas [3] = {{GPIO_20, GPIO_OUTPUT}, {GPIO_21, GPIO_OUTPUT}, {GPIO_22, GPIO_OUTPUT}};
gpioConf_t sensorHumedad = {GPIO_23, GPIO_INPUT};

bool encendido = false;
float ph = 0;
float tension = 0;
/*==================[internal functions declaration]=========================*/
void FuncUART(void* param)
{    
    
}

void FuncTimerA(void* param)
{
    xTaskNotifyGive(humedad_task_handle);
}

void FuncTimerB(void* param)
{
    xTaskNotifyGive(ph_task_handle);
}

void FuncTimerC(void* param)
{
    xTaskNotifyGive(uart_task_handle);
}

/** @fn void  encendido()
 *  @brief Modifica el estado de la variable encendido a VERDADERO
 * 	@param 
 *  @return
*/
void controlEncendido()
{
    encendido = true;
}

/** @fn void  encendido()
 *  @brief Modifica el estado de la variable encendido a FALSO
 * 	@param 
 *  @return
*/
void controlApagado()
{
    encendido = false;
}

/** @fn static void  sensarHumedad(void* param)
 *  @brief Chequea el estado del sensor de humedad (imaginario) y decide si es necesario encender la bomba de agua o no
 * 	@param 
 *  @return
*/
static void sensarHumedad(void* param)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (encendido)
		{
			//No se emplea ningun drive externo. El sensor de humedad vive en nuestras mentes
			if (GPIORead(sensorHumedad.pin))
			{
				GPIOOn(vecBombas[0].pin);		
			}
			else
			{
				GPIOOff(vecBombas[0].pin);
			}
		}
	}
}

/** @fn static void  sensarPH(void* param)
 *  @brief Utiliza eun conversor analogico digital para capturar el valor de PH representado en tension. 
 * Luego lo convierte a valores de ph y decide si es necesario encender alguna de las bombas de solucion acida o solucion basica
 * 	@param 
 *  @return
*/
static void sensarPH(void* param)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (encendido)
		{
			AnalogInputReadSingle(CH1, &tension);
			ph = tension * (14/3);

			if (ph < 6)
			{
				GPIOOn(vecBombas[1].pin);
				GPIOOff(vecBombas[2].pin);
			}
			else if (ph > 6.7)
			{
				GPIOOn(vecBombas[2].pin);
				GPIOOff(vecBombas[1].pin);
			}
			else 
			{
				GPIOOff(vecBombas[1].pin);
				GPIOOff(vecBombas[2].pin);
			}
		}
	}
}

/** @fn static void  notificarEstado(void* param)
 *  @brief Notifica el estado de ph y si se encuentra encendida alguna bomba por medio de la UART
 * 	@param 
 *  @return
*/
static void notificarEstado(void* param)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (encendido)
		{
			UartSendString(UART_PC, "pH: ");
			UartSendString(UART_PC, (char*)UartItoa((ph), 10));
			if (GPIORead(sensorHumedad.pin))
			{
				UartSendString(UART_PC, ", Humedad incorrecta");
    			UartSendString(UART_PC, "\r\n");		
			}
			else
			{
				UartSendString(UART_PC, ", Humedad correcta");
    			UartSendString(UART_PC, "\r\n");
			}

			for (int i = 0; i < 3; i++)
			{
				if (GPIORead(vecBombas[i].pin))
				{
					switch (i)
					{
						case 0:
							UartSendString(UART_PC, "Bomba de agua encendida");
    						UartSendString(UART_PC, "\r\n");
						break; 
						case 1:
							UartSendString(UART_PC, "Bomba basica encendida");
    						UartSendString(UART_PC, "\r\n");
						break; 
						case 2:
							UartSendString(UART_PC, "Bomba acida encendida");
    						UartSendString(UART_PC, "\r\n");
						break; 
						default:
						break;
					}	
				}
			}
		}
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	timer_config_t timer_humedad = {
        .timer = TIMER_A,
        .period = TASA_REFRESCO,            
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_humedad);	

	timer_config_t timer_ph = {
        .timer = TIMER_B,
        .period = TASA_REFRESCO,            
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_ph);

	timer_config_t timer_uart = {
        .timer = TIMER_C,
        .period = TASA_REFRESCO_LENTA,            
        .func_p = FuncTimerC,
        .param_p = NULL
    };
    TimerInit(&timer_uart);	

	serial_config_t puerto_serie = {
        .port = UART_PC,
        .baud_rate = 115200,                
        .func_p = FuncUART,
        .param_p = NULL
    };
    UartInit(&puerto_serie);

	analog_input_config_t conversorAD = {			
	.input = CH1,			
	.mode = ADC_SINGLE,		
	.func_p = NULL,			
	.param_p = NULL,			
	.sample_frec = 0	
    };
    AnalogInputInit(&conversorAD);

	for (int i = 0; i < 3; i++)
	{
		GPIOInit(vecBombas[i].pin, vecBombas[i].dir);
	}
	GPIOInit(sensorHumedad.pin, sensorHumedad.dir);
	SwitchesInit();

    SwitchActivInt(SWITCH_1, controlEncendido, NULL);
    SwitchActivInt(SWITCH_2, controlApagado, NULL);

	xTaskCreate(&sensarHumedad, "HUMEDAD", 512, NULL, 5, &humedad_task_handle);
	xTaskCreate(&sensarPH, "PH", 512, NULL, 5, &ph_task_handle);
	xTaskCreate(&notificarEstado, "ESTADO", 512, NULL, 5, &uart_task_handle);

	TimerStart(timer_humedad.timer);
	TimerStart(timer_ph.timer);
	TimerStart(timer_uart.timer);
}
/*==================[end of file]============================================*/