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
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
/*==================[macros and definitions]=================================*/
#define TASA_REFRESCO 1000000
#define ECHO GPIO_3
#define TRIGGER GPIO_2

/*==================[internal data definition]===============================*/
TaskHandle_t display_task_handle = NULL;
TaskHandle_t rule_task_handle = NULL;

uint32_t distancia;
uint8_t tecla;
uint8_t teclaUART;
uint8_t base = 10;
bool encendido = false;
bool hold = false;
/*==================[internal functions declaration]=========================*/
void FuncTimerA(void* param)
{
    xTaskNotifyGive(rule_task_handle);
    xTaskNotifyGive(display_task_handle);    /* Envía una notificación a la tarea asociada al display */
}

void FuncUART(void* param)
{    
    UartReadByte(UART_PC, teclaUART); 
    if (teclaUART == 'O')
    {
        controlEncendido();
    }
    else if (teclaUART == 'H')
        {
            controlHold();
        }
}

void controlEncendido()
{
    encendido = !encendido;
}

void controlHold()
{
    hold = !hold;
}

void controlLEDs()
{
    if (distancia < 10)
    {
        LedOff(LED_1);
        LedOff(LED_2);
        LedOff(LED_3);
    }
    else if (10 <= distancia && distancia <= 20)
    {
        LedOn(LED_1);
        LedOff(LED_2);
        LedOff(LED_3);
    }
    else if (20 <= distancia && distancia <= 30)
    {
        LedOn(LED_1);
        LedOn(LED_2);
        LedOff(LED_3);
    }
    else if (30 < distancia)
    {
        LedOn(LED_1);
        LedOn(LED_2);
        LedOn(LED_3);
    }
}

static void medirDistancia(void *pvParameter) // Primer tarea: medir distancia
{
    while (true)
    {
        UartSendString(UART_PC, "medir ");
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (encendido)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
        }
    }
}

static void mostrarDisplay(void *pvParameter) // Segunda tarea: mostrar informacion
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (encendido)
        {
            controlLEDs();
            if (!hold)
            {
                LcdItsE0803Write(distancia);
                UartSendString(UART_PC, (char*)UartItoa(distancia, base));
                UartSendString(UART_PC, " cm\r\n");
            }
        }
        else
        {
            LedsOffAll();
            LcdItsE0803Off();
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    timer_config_t timer_display = {
        .timer = TIMER_A,
        .period = TASA_REFRESCO,            //La tasa de refresco debe estar en microsegundos
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_display);

    LedsInit();
    SwitchesInit();
    SwitchActivInt(SWITCH_1, controlEncendido, NULL);
    SwitchActivInt(SWITCH_2, controlHold, NULL);
    HcSr04Init(ECHO, TRIGGER);
    LcdItsE0803Init();

    serial_config_t puerto_serie = {
        .port = UART_PC,
        .baud_rate = 115200,                //Velocidad de trabajo del puerto serie
        .func_p = FuncUART,
        .param_p = NULL
    };
    UartInit(&puerto_serie); 
  
    xTaskCreate(&medirDistancia, "REGLA", 512, NULL, 5, &rule_task_handle);
    xTaskCreate(&mostrarDisplay, "DISPLAY", 512, NULL, 5, &display_task_handle);
    
    TimerStart(timer_display.timer);
}
