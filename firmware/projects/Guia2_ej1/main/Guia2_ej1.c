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
 * | 10/04/2023 | Document creation		                         |
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
/*==================[macros and definitions]=================================*/
#define TASA_REFRESCO_DISPLAY 1000000
#define TASA_REFRESCO_LEDS 1000000
#define ECHO GPIO_3
#define TRIGGER GPIO_2

/*==================[internal data definition]===============================*/
TaskHandle_t display_task_handle = NULL;
// TaskHandle_t leds_task_handle = NULL;

uint32_t distancia;
uint8_t tecla;
bool encendido = false;
bool hold = false;
/*==================[internal functions declaration]=========================*/
void FuncTimerA(void* param)
{
    xTaskNotifyGive(display_task_handle);    /* Envía una notificación a la tarea asociada al display */
}

// void FuncTimerB(void* param)
// {
//     xTaskNotifyGive(leds_task_handle);    /* Envía una notificación a la tarea asociada al control de LEDs */
// }

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
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (encendido)
        {
            distancia = HcSr04ReadDistanceInCentimeters();
            // printf("\nDisntacia: %u", distancia);
        }
        //vTaskDelay(TASA_REFRESCO_LENTO / portTICK_PERIOD_MS);
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
            }
        }
        else
        {
            LedsOffAll();
            LcdItsE0803Off();
        }
        //vTaskDelay(TASA_REFRESCO_LENTO / portTICK_PERIOD_MS);
    }
}

// static void controlTeclas(void *pvParameter) // Tercer tarea: control de teclas
// {
//     while (true)
//     {
//         tecla = SwitchesRead();
//         switch (tecla)
//         {
//         case SWITCH_1:
//             encendido = !encendido;
//             break;
//         case SWITCH_2:
//             hold = !hold;
//             break;
//         }
//         vTaskDelay(10 / portTICK_PERIOD_MS);
//     }
// }
/*==================[external functions definition]==========================*/
void app_main(void)
{
    timer_config_t timer_display = {
        .timer = TIMER_A,
        .period = TASA_REFRESCO_DISPLAY,
        .func_p = FuncTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_display);
    // timer_config_t timer_leds = {
    //     .timer = TIMER_B,
    //     .period = TASA_REFRESCO_LEDS,
    //     .func_p = FuncTimerB,
    //     .param_p = NULL
    // };
    // TimerInit(&timer_leds);

    LedsInit();
    HcSr04Init(ECHO, TRIGGER);
    LcdItsE0803Init();
    SwitchesInit();

    SwitchActivInt(SWITCH_1, controlEncendido, NULL);
    SwitchActivInt(SWITCH_2, controlHold, NULL);
    xTaskCreate(&medirDistancia, "REGLA", 512, NULL, 5, &display_task_handle);
    xTaskCreate(&mostrarDisplay, "DISPLAY", 512, NULL, 5, &display_task_handle);
    //xTaskCreate(&controlTeclas, "TECLA", 512, NULL, 5, NULL); //Cambiar por interrupciones

    TimerStart(timer_display.timer);
    //TimerStart(timer_leds.timer);
}
