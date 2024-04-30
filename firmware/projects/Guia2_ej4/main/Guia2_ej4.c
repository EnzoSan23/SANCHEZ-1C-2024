/*! @mainpage Blinking
 *
 * \section genDesc General Description
 *
 * This example makes LED_1, LED_2 and LED_3 blink at different rates, using FreeRTOS tasks.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 17/04/2023 | Document creation		                         |
 *
 * @author Enzo Sanchez (enzo.sanchez@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define TASA_REFRESCO_RAPIDO 2000
#define TASA_REFRESCO_LENTO 4000
#define BUFFER_SIZE 231

/*==================[internal data definition]===============================*/
TaskHandle_t ad_task_handle = NULL;
TaskHandle_t da_task_handle = NULL;

const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
uint8_t base = 10;
uint16_t lectura;
uint8_t i = 0;
/*==================[internal functions declaration]=========================*/
void FuncTimerA(void* param)
{
    xTaskNotifyGive(ad_task_handle);
}

void FuncTimerB(void* param)
{
    xTaskNotifyGive(da_task_handle);
}

void FuncUART(void* param)
{    
    
}

static void convertir(void* param)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH1, &lectura);
        UartSendString(UART_PC, (char*)UartItoa(lectura, base));
        UartSendString(UART_PC, "\r\n");
    }
}

static void generarECG(void* param)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (i < BUFFER_SIZE)
        {
            AnalogOutputWrite(ecg[i]);
        }
        else
        {
            i = 0;
        }
    }
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
    timer_config_t timer_osciloscopio = {
        .timer = TIMER_A,
        .period = TASA_REFRESCO_RAPIDO,            //La tasa de refresco debe estar en microsegundos
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_osciloscopio);

    timer_config_t timer_ecg = {
        .timer = TIMER_B,
        .period = TASA_REFRESCO_LENTO,            //La tasa de refresco debe estar en microsegundos
        .func_p = FuncTimerB,
        .param_p = NULL
    };
    TimerInit(&timer_ecg);

    serial_config_t puerto_serie = {
        .port = UART_PC,
        .baud_rate = 115200,                //Velocidad de trabajo del puerto serie
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
    AnalogOutputInit();
  
    xTaskCreate(&convertir, "CONVERSORAD", 512, NULL, 5, &ad_task_handle);
    xTaskCreate(&generarECG, "ECG", 512, NULL, 5, &da_task_handle);

    TimerStart(timer_ecg.timer);
    TimerStart(timer_osciloscopio.timer);
}
