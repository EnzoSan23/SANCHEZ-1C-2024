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
 * | 20/03/2024 | Document creation		                         |
 *
 * @author Enzo Sanchez (enzosanchez237@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

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
/*==================[external functions definition]==========================*/
void app_main(void){
	uint32_t valor = 2587;
	uint8_t digitos = 4;
	uint8_t vec[digitos];
	
	convertToBcdArray(valor, digitos, vec);

	for (uint8_t i = 0; i < digitos; i++)  //Compruebo que se haya convertido el numero correctamente
	{
		printf("Posicion %d",i);
		printf(". Numero %d\n",vec[i]);
	}
}
/*==================[end of file]============================================*/