/*! @mainpage Primer Proyecto
 *
 * @section genDesc General Description
 *
 * La aplicación permite mostrar en un display LCD "CD4543", 
 * un numero de 3 cifras ingresado previamente.
 * Esto se realiza mediante el uso de diferntes GPIO de la placa ESP32-C6-DevKitC-1.
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

/** @def gpioConf_t
 *  @brief Estructura que define la configuracion de un GPIO
 */
typedef struct
{
	gpio_t pin; 		/*!< GPIO pin number */
	io_t dir; 			/*!< GPIO direction '0' IN; '1' OUT*/
} gpioConf_t;
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/** @fn void  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
 *  @brief Carga un vector con numeros BCD correspondientes a un numero decimal con una determinada cantidad de digitos
 * 	@param [data, digits, bcd_number]
 *  @return
*/
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

/** @fn void cambioEstadoGPIO(uint8_t digitoBCD, gpioConf_t *vecGPIO)
 *  @brief Cambia el estado de un GPIO dependiendo de los digitos de un numero binario BCD
 * 	@param [digitoBCD, vecGPIO]
 *  @return
*/
void cambioEstadoGPIO(uint8_t digitoBCD, gpioConf_t *vecGPIO)
{	
	for (int i = 0; i < 4; i++)
    {
        if (digitoBCD & 1 << i)
        {
			GPIOOn(vecGPIO[i].pin);
        }
        else
			GPIOOff(vecGPIO[i].pin);
    }
}

/** @fn void mostrarDisplay(uint32_t dato, uint8_t dig, gpioConf_t *vecB, gpioConf_t *vecP)
 *  @brief Muestra el dato cargado en el display LCD, integrando las funciones covertToBcdArray y cambioEstadoGPIO, junto con la activacion de los GPIO que seleccionan los puertos del display 
 * 	@param [dato, dig, vecB, vecP]
 *  @return
*/
void mostrarDisplay(uint32_t dato, uint8_t dig, gpioConf_t *vecB, gpioConf_t *vecP)
{

	uint8_t vecBCD[dig];
	convertToBcdArray(dato, dig, vecBCD);

	for (int i = 0; i < dig; i++)
	{
		cambioEstadoGPIO(vecBCD[i], vecB);
		GPIOOn(vecP[i].pin);
		GPIOOff(vecP[i].pin);
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint32_t valor = 138;
	uint8_t digitos = 3;

	//Defino el vector que mapea los bits
	gpioConf_t vecBits [4] = {{GPIO_20, GPIO_OUTPUT}, {GPIO_21, GPIO_OUTPUT}, {GPIO_22, GPIO_OUTPUT}, {GPIO_23, GPIO_OUTPUT}};

	//Defino el vector que mapea los puertos
	gpioConf_t vecPuertos [3] = {{GPIO_19, GPIO_OUTPUT}, {GPIO_18, GPIO_OUTPUT}, {GPIO_9, GPIO_OUTPUT}};
	
	//Inicializo los GPIO
	for (int i = 0; i < 4; i++)
	{
		GPIOInit(vecBits[i].pin, vecBits[i].dir);
	}
	for (int i = 0; i < 3; i++)
	{
		GPIOInit(vecPuertos[i].pin, vecPuertos[i].dir);
	}

	//Muestro el valor enviado por display
	mostrarDisplay(valor, digitos, vecBits, vecPuertos);
}
/*==================[end of file]============================================*/