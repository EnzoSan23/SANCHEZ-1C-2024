/*! @mainpage Template
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
 * | 01/04/2023 | Document creation		                         |
 *
 * @author Enzo Sanchez (enzosanchez237@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/*==================[macros and definitions]=================================*/
#define duracionPulso 0.00005	//50 nanosegundos
/*==================[internal data definition]===============================*/
typedef struct
{
gpio_t pin; 		/*!< GPIO pin number */
io_t dir; 			/*!< GPIO direction '0' IN; '1' OUT*/
} gpioConf_t;
/*==================[internal functions declaration]=========================*/
void  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{	
	int8_t i = 0;
	while (i < digits)
	{
		bcd_number[digits-1-i] = data % 10;
		data = data / 10;
		i++;
	}
}

void cambioEstadoGPIO(uint8_t digitoBCD, gpioConf_t *vecGPIO)
{
	uint8_t mascara;

	for (int i = 0; i < 4; i++)
    {
        mascara = 1;
        mascara = mascara << i;
        if ((digitoBCD & mascara) != 0)
        {
			//printf("Setear\n");
			GPIOOn(vecGPIO[3-i].pin);
        }
        else
            //printf ("No setear\n");
			GPIOOff(vecGPIO[3-i].pin);
    }
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint32_t valor = 258;
	uint8_t digitos = 3;
	uint8_t vecBCD[digitos];

	//Defino el vector que mapea los bits
	gpioConf_t vecBits [4];
	vecBits[0].pin = GPIO_20;
	vecBits[1].pin = GPIO_21;
	vecBits[2].pin = GPIO_22;
	vecBits[3].pin = GPIO_23;
	vecBits[0].dir = vecBits[1].dir = vecBits[2].dir = vecBits[3].dir = 1;

	//Defino el vector que mapea los puertos
	gpioConf_t vecPuertos [3];
	vecPuertos[0].pin = GPIO_19;
	vecPuertos[1].pin = GPIO_18;
	vecPuertos[2].pin = GPIO_9;
	vecPuertos[0].dir = vecPuertos[1].dir = vecPuertos[2].dir = 1;
	
//Inicializo los GPIO
	for (int i = 0; i < 4; i++)
	{
		GPIOInit(vecBits[i].pin, vecBits[i].dir);
	}
	for (int i = 0; i < 3; i++)
	{
		GPIOInit(vecPuertos[i].pin, vecPuertos[i].dir);
	}

	convertToBcdArray(valor, digitos, vecBCD);		//Convierto el valor a numeros BCD
	for (int i = 0; i < digitos; i++)				//Para cada BCD, cargo los bits de entrada del conversor
	{
		cambioEstadoGPIO(vecBCD[i], vecBits);
		GPIOOn(vecPuertos[i].pin);					//Como envio el pulso de 50ns?
		vTaskDelay(duracionPulso / portTICK_PERIOD_MS);
		GPIOOff(vecPuertos[i].pin);
	}

}
/*==================[end of file]============================================*/