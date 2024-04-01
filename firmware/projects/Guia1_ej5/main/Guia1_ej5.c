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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct
{
gpio_t pin; 		/*!< GPIO pin number */
io_t dir; 			/*!< GPIO direction '0' IN; '1' OUT*/
} gpioConf_t;
/*==================[internal functions declaration]=========================*/
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
	// Defino el vector que mapea los bits
	gpioConf_t vecBits [4];
	vecBits[0].pin = GPIO_20;
	vecBits[1].pin = GPIO_21;
	vecBits[2].pin = GPIO_22;
	vecBits[3].pin = GPIO_23;
	vecBits[0].dir = vecBits[1].dir = vecBits[2].dir = vecBits[3].dir = 1;

	uint8_t dBCD = 0b0101;   
	
	//Inicializo los GPIO
	for (int i = 0; i < 4; i++)
	{
		GPIOInit(vecBits[i].pin, vecBits[i].dir);
	}
	 				
	cambioEstadoGPIO(dBCD, vecBits);
}
/*==================[end of file]============================================*/