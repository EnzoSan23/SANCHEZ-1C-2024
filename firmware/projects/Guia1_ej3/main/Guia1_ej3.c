/*! @mainpage Guia1_ej3
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 13/03/2024 | Document creation		                         |
 *
 * @author Enzo Sanchez (enzosanchez237@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/*==================[macros and definitions]=================================*/
#define ON 1
#define OFF 2
#define TOGGLE 3
#define CONFIG_BLINK_PERIOD 1000
/*==================[internal data definition]===============================*/
struct leds 
{
    uint8_t mode;     	//ON, OFF, TOGGLE
	uint8_t n_led;      //indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;   //indica el tiempo de cada ciclo
} my_leds; 

uint8_t i;
/*==================[internal functions declaration]=========================*/
void controlLeds (struct leds *led)
{
	switch (led->mode)
	{
		printf("entre al primer switch");
		case ON:
			switch (led->n_led)
			{
				case 1:
					LedOn(LED_1);
					printf("Led 1 encendido");
				break;
				case 2:
					LedOn(LED_2);
					printf("Led 2 encendido");
				break;
				case 3:
					LedOn(LED_3);
					printf("Led 3 encendido");
				break;
			default:
				break;
		}
		case OFF:
			switch (led->n_led)
			{
				case 1:
					LedOff(LED_1);
					printf("Led 1 apagado");
				break;
				case 2:
					LedOff(LED_2);
					printf("Led 2 apagado");
				break;
				case 3:
					LedOff(LED_3);
					printf("Led 3 apagado");
				break;
			default:
				break;
			}
		case TOGGLE:
			i = 0;
			while (i < led->n_ciclos){
				switch (led->n_led)
				{
					case 1:
						LedToggle(LED_1);
						i++;
					break;
					case 2:
						LedToggle(LED_2);
						i++;
					break;
					case 3:
						LedToggle(LED_3);
						i++;
					break;
				default:
					break;
				}
				/*while (j < led->periodo)
				{
					j++;
					vTaskDelay(j)
				}*/
				vTaskDelay(led->periodo / portTICK_PERIOD_MS);
		}			
	default:
		break;
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	while(1)    {
		LedsInit();			//Todos los perifericos que yo utilice se inicializan en el main una sola vez
		my_leds.mode = ON;
		my_leds.n_led = 1;

		controlLeds(&my_leds);
		vTaskDelay(1000);
	 }
}
/*==================[end of file]============================================*/